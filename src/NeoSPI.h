#include <string>
#include <vector>

using namespace std;

class Color {       // Color class to define a color with one variable
    public:
        uint8_t r = 0;      // Red color value
        uint8_t g = 0;      // Green color value
        uint8_t b = 0;      // Blue color value
};

class NeoSPI { // Main classroom
    public: // "User" accesible stuff
        //NeoSPI(string spidev, int _numPixels = 100, int _speed = 800); // Constructor (pretty simple)
        int begin(string spidev, int _numPixels = 100, int _speed = 800);
        void show();
        void fillStripRGB(uint8_t r, uint8_t g, uint8_t b); // Function to fill the strip using an RGB value
        bool setPixelRGB(uint8_t r, uint8_t g, uint8_t b, int pos); // Function to set a single pixel using an RGB value (returns false if the pixel doesn't exist)
        void fillStrip(Color newColor); // Function to fill the strip
        bool setPixel(Color newColor, int pos); // Function to set a single pixel (returns false if the pixel doesn't exist)
    private: // Backend stuff
        void writeBuff();
        bool spiOpen(string dev);
        bool int2Bool(uint8_t input, int digit);
        vector<uint8_t> int2NeoPixel(uint8_t val);
        vector<uint8_t> RGB2BitBang(uint8_t r, uint8_t g, uint8_t b);
        
        int numPixels = 100; // The length of the strip
        int speed = 800; //NeoPixel speed in KHz Newer NeoPixels are 800KHz older ones are 400KHz
        int SPIspeed = speed*8*1024;

        #define SPI_delay 0 // Maybe the waittime between words
        #define SPI_BPW 8 // I have no idea but my theory is: Bytes Per Word
        
        int spi_fd;
        vector<Color> stripState; // A middle-man-buffer where the data gets stored before it gets to the final spi buffer
        vector<uint8_t> rawData; //uint8_t rawData[numPixels*24]; // Stores the raw data that will get send over SPI
};

