
<img width="874" alt="example" src="https://github.com/p-ranav/ImageViewer-Qt6/assets/8450091/7d1a47f1-fa2c-4372-bd78-f50158ad6c0d">

# Features

- Load common image types such as `.jpg`, `.png`, `.tiff`, and raw types like `.nef`.
- Zoom and pan with trackpad/mouse.
- Copy image to clipboard.
- Copy image path.
- Copy image to location.
- Delete image.
- Next image, previous image, first image, last image.
- Sort by name, size, or date modified.
- Start a slideshow, change slideshow period.

# Building from Source

## MacOS

```console
brew install libraw qt@6
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DQt6_DIR=$(brew --prefix qt6)/lib/cmake/Qt6 -DLibRaw_INCLUDE_DIRS=$(brew --prefix libraw)/include -DLibRaw_LIBRARIES=$(brew --prefix libraw)/lib/libraw.dylib ..
make
```

## Ubuntu

```console
sudo apt install libraw-dev qt6-base-dev
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DLibRaw_INCLUDE_DIRS=$(pkg-config --cflags libraw) -DLibRaw_LIBRARIES=$(pkg-config --libs libraw) ..
make
```

For older versions of Ubuntu,

```console
sudo add-apt-repository ppa:okirby/qt6-backports
sudo apt update
sudo apt install qt6-base-dev
```
