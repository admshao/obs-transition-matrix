# obs-transition-matrix
Customize Any -> One or One -> One scene transitions

# Compiling obs-transition-matrix
## Prerequisites
You'll need [Qt 5.10.x](https://download.qt.io/official_releases/qt/5.10/),
[CMake](https://cmake.org/download/), and a working [development environment for
OBS Studio](https://obsproject.com/wiki/install-instructions) installed on your
computer.

## Windows
In cmake-gui, you'll have to set the following variables :
- **QTDIR** (path) : location of the Qt environment suited for your compiler and architecture
- **LIBOBS_INCLUDE_DIR** (path) : location of the libobs subfolder in the source code of OBS Studio
- **LIBOBS_LIB** (filepath) : location of the obs.lib file
- **OBS_FRONTEND_LIB** (filepath) : location of the obs-frontend-api.lib file

Then generate the Visual Studio solution and click to build

## Linux
- **QTDIR** Should be automatically detected if installed in the system
- **LIBOBS_INCLUDE_DIR** (path) : path to the libobs sub-folder in obs-studio's source code
- **LIBOBS_LIB** (filepath) : location of the libobs.so file
- **OBS_FRONTEND_LIB** (filepath) : location of the libobs-frontend-api.so file
- **CUSTOM** (bool) : define this var to not use default unix structure (Like
 OBS param UNIX_STRUCTURE)
- **CMAKE_INSTALL_PREFIX** (path) : program instalation output

Default OBS install
```
git clone https://github.com/admshao/obs-transition-matrix.git
cd obs-transition-matrix
mkdir build && cd build
cmake -DLIBOBS_INCLUDE_DIR="path" -DLIBOBS_LIB="filepath" -DOBS_FRONTEND_LIB="filepath" -DCMAKE_INSTALL_PREFIX=/usr ..
make -j4
sudo make install
```
Custom OBS install
```
git clone https://github.com/admshao/obs-transition-matrix.git
cd obs-transition-matrix
mkdir build && cd build
cmake -DLIBOBS_INCLUDE_DIR="path" -DLIBOBS_LIB="filepath" -DOBS_FRONTEND_LIB="filepath" -DCMAKE_INSTALL_PREFIX="path to your custom compilation" -DCUSTOM=1 ..
make -j4
make install
```

## OS X
**Not supported** as i don't have the hardware to develop/test