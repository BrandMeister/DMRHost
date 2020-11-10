These are the source files for building the DMRHost, the program that
interfaces to the MMDVM or DVMega on the one side, and a suitable network on
the other. DMR, and POCSAG on the MMDVM, and DMR on the DVMega.

DMRHost can connect to BrandMeister, DMR+, TGIF, HB Link, or XLX.
It uses the DAPNET Gateway to access DAPNET to receive paging messages.

It builds on 32-bit and 64-bit Linux. It can optionally control
various Displays. Currently these are:

- Nextion TFTs (all sizes, both Basic and Enhanced versions)
- OLED 128x64 (SSD1306)
- LCDproc

The Nextion displays can connect to the UART on the Raspberry Pi, or via a USB
to TTL serial converter like the FT-232RL. It may also be connected to the UART
output of the MMDVM modem (Arduino Due, STM32, Teensy)

The OLED display needs an extra library: https://github.com/hallard/ArduiPi_OLED

The LCDproc support enables the use of a multitude of other LCD screens. See
the [supported devices](http://lcdproc.omnipotent.net/hardware.php3) page on
the LCDproc website for more info.

DMRHost uses CMake as its building system:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

Debian / Ubuntu packages can be found at our repo:
```
echo "deb http://repo.test.net.in/dmrhost stable main" > /etc/apt/sources.list.d/dmrhost.list
wget http://repo.test.net.in/dmrhost/public.key -O - | apt-key add -
apt-get update
apt-get install dmrhost
```
