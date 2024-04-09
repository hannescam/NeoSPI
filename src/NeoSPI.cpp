#include "NeoSPI.h"
#include <vector>
#include <cstring>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

bool NeoSPI::spiOpen(string dev)
{
  if((spi_fd = open(dev.c_str(), O_RDWR)) < 0) // Trying to open spi device
  {
    printf("error opening %s\n",dev); // Crying when it doesn't work
    return false;
  }
  return true; // Or just be happy :)
}

void NeoSPI::writeBuff() // It just works but I have no idea how... 🤷
{
  struct spi_ioc_transfer spi; // A lot of complicated SPI stuff
  memset (&spi, 0, sizeof(spi)); // Reserving memmory for SPI
  spi.tx_buf = reinterpret_cast<unsigned long>(rawData.data());    //(unsigned long)rawData; // Converts the array to a single long value
  spi.len = numPixels*24; // Calculates the SPI message length
  spi.delay_usecs = SPI_delay;
  spi.speed_hz = SPIspeed; // Sets the precise SPI speed
  spi.bits_per_word = SPI_BPW;
  ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi); // Gives all the infos to the linux kernel for processing
}

bool NeoSPI::int2Bool(uint8_t input, int digit)
{
   switch(digit) { // Split the uint8_t into individual bits in reverse direction
    case 7:
      return input & 0x01;
      break;
    case 6:
      return input & 0x02;
      break;
    case 5:
      return input & 0x04;
      break;
    case 4:
      return input & 0x08;
      break;
    case 3:
      return input & 0x10;
      break;
    case 2:
      return input & 0x20;
      break;
    case 1:
      return input & 0x40;
      break;
    case 0:
      return input & 0x80;
      break;
  }
  return false;
}

vector<uint8_t> NeoSPI::int2NeoPixel(uint8_t val)
{
  int cnt = 0; // Initialize a counter with 7
  vector<uint8_t> color(8); // Initialize the array with 8 values for the 8 bits
  color = std::vector<uint8_t>(8, 0xC0); // Fill the LOW bytes in with the SPI BitBang bytes
  while (cnt < 8) // Count through evrey bit of a byte
  {
    if (int2Bool(val, cnt)) color.at(cnt) = 0xF8; // Fill the HIGH bytes in with the SPI BitBang bytes
    cnt++;
  }
  return color;
}

vector<uint8_t> NeoSPI::RGB2BitBang(uint8_t r, uint8_t g, uint8_t b)
{
  vector<uint8_t> r_array = int2NeoPixel(r); // Get the SPI bytes for the red channel
  vector<uint8_t> g_array = int2NeoPixel(g); // Get the SPI bytes for the green channel
  vector<uint8_t> b_array = int2NeoPixel(b); // Get the SPI bytes for the blue channel

  int cnt = 0;
  vector<uint8_t> fullColor(24);
  while(cnt < 24)
  {
    if (cnt < 8)
    {
      fullColor.at(cnt) = r_array.at(cnt); // Append the red SPI bytes to the fullColor array
    }
    else if (cnt < 16)
    {
      fullColor.at(cnt) = g_array.at(cnt-8); // Append the green SPI bytes to the fullColor array
    }
    else if (cnt < 24)
    {
      fullColor.at(cnt) = b_array.at(cnt-16); // Append the blue SPI bytes to the fullColor array
    }
    cnt++;
  }
  return fullColor;
}

// Fill the strip with a specific color
void NeoSPI::fillStripRGB(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0)
{
  Color newColor; // Putting the new color into the Color class
  newColor.r = r;
  newColor.g = g;
  newColor.b = b;
  stripState = std::vector<Color>(numPixels, newColor); // Saving the changes to the middle-man-buffer
}

// Set the color of a specific pixel
bool NeoSPI::setPixelRGB(uint8_t r, uint8_t g, uint8_t b, int pos)
{
  if (pos >= numPixels) return false; // Return false if the LED doesn't exist
  Color newColor;
  newColor.r = r;
  newColor.g = g;
  newColor.b = b;
  stripState.at(pos) = newColor; // Saving the changes to the middle-man-buffer
  return true; // If the length isn't larger than the amount of LEDs on the strip return true
}

void NeoSPI::fillStrip(Color newColor)
{
  stripState = std::vector<Color>(numPixels, newColor); // Saving the changes to the middle-man-buffer
}

// Set the color of a specific pixel
bool NeoSPI::setPixel(Color newColor, int pos)
{
  if (pos >= numPixels) return false; // Return false if the LED doesn't exist
  stripState.at(pos) = newColor; // Saving the changes to the middle-man-buffer
  return true; // If the length isn't larger than the amount of LEDs on the strip return true
}

void NeoSPI::show()
{
  Color activeElement; // Create new Color value
  vector<uint8_t> col(24); // And a new raw SPI data value
  int cntTotal = 0; // Counter initializations
  int cntCol = 0;
  int cnt = 0;
  while(cnt < numPixels) // Looping through all of the LEDs
  {
    activeElement = stripState.at(cnt); // Retrieve color from middle-man-buffer
    col = RGB2BitBang(activeElement.r, activeElement.g, activeElement.b); // Convert that RGB value to the raw SPI bytes
    cntCol = 0; // Reset the color counter (complicated)
    while (cntCol < sizeof(col) / sizeof(col.at(0))) { // Count through all 24 bytes of the raw SPI color value for the current LED
        rawData.at(cntTotal) = col.at(cntCol); // Place that SPI value in the final array that (almost) represents the state of the physical LEDs
        cntCol++; // Counter action
        cntTotal++;
    }
    cnt++;
  }
  writeBuff(); // Write the contents of the final raw SPI array to the LEDs
  usleep(100); // Waiting for the signals to be sent (probably too low for the 400KHz NeoPixels)
}

/*NeoSPI::NeoSPI(string spidev, int _numPixels, int _speed)
{
  
}*/

int NeoSPI::begin(string spidev, int _numPixels, int _speed)
{
  numPixels = _numPixels; // Transfer the temporary values to the "public" permanent ones
  speed = _speed;
  SPIspeed = speed*1024*8; // Calculate the SPI speed in bytes/s
  stripState.resize(numPixels); // Apply the size value to the arrays
  rawData.resize(numPixels*24);
  bool sucess = spiOpen(spidev); // Open the SPI device
  usleep(100000); // Sleep for 100ms to ensure that the spidev is ready
  fillStripRGB(0,0,0); // Clear the strip so there are no LEDs remaining on
  show(); // Commit the changes
  return sucess;
}
