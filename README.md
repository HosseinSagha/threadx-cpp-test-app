# LV-PCU
Power Control Unit
# Minimum Requirements
* **[Git](https://gitforwindows.org/)**
* **[CMake](https://cmake.org/download/)**
* **[Ninja build system](https://github.com/ninja-build/ninja/releases)**
    * Add executable/app path to **PATH** environment variable.
* **[GNU Arm Embedded Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)**
    * Remove **blank spaces** from the installation folder e.g. **C:\Arm-GNU-Toolchain-arm-none-eabi\xx.y-relz.

Refer to [How to Setup Firmware Development Platform](https://xwiki-enterprise.corp.wabtec.com/xwiki/wiki/dmtcaswiki/view/05.%20Process%20%26%20Tools/How%20to%20Setup%20Firmware%20Development%20Platform/) page in XWiki.

# Build Command
```
$ build.sh [clean | init | update | master | pull]
```
# Submodules
```
$ build.sh init
$ build.sh master
$ build.sh pull
$ build.sh update
```
# Build
```
$ build.sh clean
$ build.sh
```
