# Settings

The System Settings application for Cutefish Desktop.

## Dependencies

```shell
sudo pacman -S extra-cmake-modules qt5-base qt5-quickcontrols2 freetype2 fontconfig networkmanager-qt modemmanager-qt kcoreaddons
```

For Ubuntu, to meet the demand of module `icu-i18n`, you need to:
```shell
sudo apt install libicu-dev
```

## Build

```shell
mkdir build
cd build
cmake ..
make
sudo make install
```

## Install

```shell
sudo make install
```
