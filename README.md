# Settings

The System Settings application for Cutefish Desktop.

![screenshot](screenshots/Screenshot_20211025_150819.png)

## Dependencies

Arch/Manjaro Dependencies:
```shell
sudo pacman -S cmake qt6-base qt6-declarative qt6-tools icu libxcrypt freetype2 fontconfig libxcursor
```

Debian/Ubuntu Dependencies:
```shell
sudo apt install cmake debhelper libicu-dev libcrypt-dev libfreetype6-dev libfontconfig1-dev libxcursor-dev qt6-base-dev qt6-declarative-dev qt6-tools-dev qt6-tools-dev-tools pkg-config
```

## Privacy

The Privacy page lists applications that have asked to record the screen
through the portal, read from `~/.config/cutefish/privacy.conf` via the
framework's `ScreenRecordingPermissions`. The switch grants or revokes the
standing permission; an application that is switched off is asked again the
next time it starts a screen cast. Only installed applications are listed: a
record whose desktop entry is gone stays in the file and reappears, with its
previous setting, when the application is installed again.

## Build

```shell
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
make
sudo make install
```

## License

This project has been licensed by GPLv3.
