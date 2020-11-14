These are the source files for building the DMRHost, the program that
interfaces to the MMDVM or DVMega on the one side, and a suitable network on
the other. DMR, and POCSAG on the MMDVM, and DMR on the DVMega.

DMRHost can connect to BrandMeister, DMR+, TGIF, HB Link, or XLX.
It uses the DAPNET Gateway to access DAPNET to receive paging messages.

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

If you have questions, feel free to join our [telegram](https://t.me/dmrhost) group.
