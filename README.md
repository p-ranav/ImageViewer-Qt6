```console
brew install libraw qt@6
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DQt6_DIR=$(brew --prefix qt6)/lib/cmake/Qt6 -DLibRaw_INCLUDE_DIRS=$(brew --prefix libraw)/include -DLibRaw_LIBRARIES=$(brew --prefix libraw)/lib/libraw.dylib ..
make
```

```console
sudo apt install libraw-dev qt6-base-dev
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DLibRaw_INCLUDE_DIRS=$(pkg-config --cflags libraw) -DLibRaw_LIBRARIES=$(pkg-config --libs libraw) ..
make
```