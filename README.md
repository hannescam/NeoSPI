# NeoSPI
A simplistic C++ SPI NeoPixel driver that works on any linux device with SPI

- build:
```
make
```
- run:
```
./examples/NeoSPI_Standarttest /dev/spidev0.0
```
- clean:
```
make clean
```
- If you want to drive more than 170 LEDs it is necesary to increase the SPI buffer size using ```spidev.bufsiz=65536``` in the kernel command line
- SPI C/C++ code template: https://forums.raspberrypi.com/viewtopic.php?t=304828#p1824960
