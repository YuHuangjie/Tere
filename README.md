# Tere

Tere is a c++ library dedicated to image-based rendering with explicit geometry.

# Building Tere

## Third Party Dependencies

No dependencies are required to build Tere. However, building samples requires [glfw](https://github.com/glfw/glfw) and [libjpeg-turbo](https://github.com/libjpeg-turbo/libjpeg-turbo). We recommend installing 3rd-party packages with [vcpkg](https://github.com/Microsoft/vcpkg).

```
vcpkg install glfw3:x64-windows
vcpkg install libjpeg-turbo:x64-windows
```

## How To Build (Visual Studio (2017))

Build only the core library
```
mkdir build
cd build
cmake -A x64 -G"Visual Studio 15 2017" ..
```

or build the core library and a simple renderer
```
mkdir build
cd build
cmake -A x64 -G"Visual Studio 15 2017" -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake -DBUILD_SAMPLES=ON ..
```

To use CUDA, you simply add it onto your command line as ```-DUSE_CUDA=ON```.

## Test
```
cd build/bin/Release
./TereSample.exe ..\..\..\TestData\lion\profile.txt
```