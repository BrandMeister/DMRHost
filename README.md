These are the source files for building the DMRHost, the program that
interfaces to the MMDVM or DVMega on the one side, and a suitable network on
the other. DMR, and POCSAG on the MMDVM, and DMR on the DVMega.

DMRHost can connect to BrandMeister, DMR+, TGIF, HB Link, or XLX.
It uses the DAPNET Gateway to access DAPNET to receive paging messages.

It builds on 32-bit and 64-bit Linux. It can optionally control
various Displays. Currently these are:

- Nextion TFTs (all sizes, both Basic and Enhanced versions)
- TFT display sold by Hobbytronics in UK
- OLED 128x64 (SSD1306)

The Nextion displays can connect to the UART on the Raspberry Pi, or via a USB
to TTL serial converter like the FT-232RL. It may also be connected to the UART
output of the MMDVM modem (Arduino Due, STM32, Teensy), or to the UART output
on the UMP.

The Hobbytronics TFT Display, which is a Pi-Hat, connects to the UART on the
Raspbery Pi.

The OLED display needs an extra library see OLED.md

DMRHost uses CMake as its building system:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
