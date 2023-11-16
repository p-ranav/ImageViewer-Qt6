```console
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DQt6_DIR=$(brew --prefix qt6)/lib/cmake/Qt6 cmake -DLibRaw_INCLUDE_DIRS=$(brew --prefix libraw)/include -DLibRaw_LIBRARIES=$(brew --prefix libraw)/lib/libraw.dylib ..
make
```