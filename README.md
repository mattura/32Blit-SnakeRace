# SnakeRace

For 32Blit Snake2 build:
```
mkdir build.stm32
cd build.stm32
cmake .. -D32BLIT_DIR="/path/to/32blit/repo" -DCMAKE_TOOLCHAIN_FILE=/path/to/32blit/repo/32blit.toolchain
make
```

For local Snake2 build:
```
mkdir build
cd build
cmake -D32BLIT_DIR=/path/to/32blit-sdk/ ..
make
```

For PicoSystem build:
```
mkdir build.pico
cd build.pico
cmake -D32BLIT_DIR=/path/to/32blit-sdk -DCMAKE_TOOLCHAIN_FILE=/path/to/32blit-sdk/pico.toolchain -DPICO_BOARD=pimoroni_picosystem
make
```
