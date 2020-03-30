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

To build this plugin on macOS the following software needs to be installed on the system:

* Xcode and Xcode command line tools
* Homebrew (https://brew.sh)

Then follow these steps to build the plugin - **for each step return to your root working directory**.

### Install necessary dependencies
```
brew install cmake mbedtls
brew install https://gist.githubusercontent.com/DDRBoxman/b3956fab6073335a4bf151db0dcbd4ad/raw/ed1342a8a86793ea8c10d8b4d712a654da121ace/qt.rb`
brew pin qt
```

### Build OBS and necessary obs-libraries

```
wget --quiet --retry-connrefused --waitretry=1 https://obs-nightly.s3.amazonaws.com/osx-deps-2018-08-09.tar.gz
tar -xf ./osx-deps-2018-08-09.tar.gz -C /tmp

git clone https://github.com/obsproject/obs-studio
cd obs-studio
git checkout 24.0.6
mkdir build && cd build
cmake .. \
    -DBUILD_CAPTIONS=true \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.11 \
    -DDISABLE_PLUGINS=true \
    -DENABLE_SCRIPTING=0 \
    -DDepsPath=/tmp/obsdeps \
    -DCMAKE_PREFIX_PATH=/usr/local/opt/qt/lib/cmake \
&& make -j4
```

### Build obs-transition-matrix plugin

```
git clone https://github.com/admshao/obs-transition-matrix.git
cd obs-transition-matrix
mkdir build && cd build
cmake .. \
    -DQTDIR=/usr/local/opt/qt \
    -DLIBOBS_INCLUDE_DIR="../../obs-studio/libobs" \
    -DLIBOBS_LIB="../../obs-studio/build/libobs/libobs.dylib" \
    -DOBS_FRONTEND_LIB="$(pwd)/../../obs-studio/build/UI/obs-frontend-api/libobs-frontend-api.dylib" \
    -DCMAKE_INSTALL_PREFIX="/usr" \
&& make -j4
```

### Prepare and install plugin

```
install_name_tool \
        -add_rpath @executable_path/../Frameworks/QtWidgets.framework/Versions/5/ \
        -add_rpath @executable_path/../Frameworks/QtGui.framework/Versions/5/ \
        -add_rpath @executable_path/../Frameworks/QtCore.framework/Versions/5/ \
        -change /usr/local/opt/qt/lib/QtWidgets.framework/Versions/5/QtWidgets @rpath/QtWidgets \
        -change /usr/local/opt/qt/lib/QtGui.framework/Versions/5/QtGui @rpath/QtGui \
        -change /usr/local/opt/qt/lib/QtCore.framework/Versions/5/QtCore @rpath/QtCore \
        obs-transition-matrix.so

sudo mkdir -p /Library/Application\ Support/obs-studio/plugins/obs-transition-matrix/bin
sudo cp -r ../data /Library/Application\ Support/obs-studio/plugins/obs-transition-matrix/
sudo cp obs-transition-matrix.so /Library/Application\ Support/obs-studio/plugins/obs-transition-matrix/bin
```