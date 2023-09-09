# ClassicImageViewer

ClassicImageViewer is a simple image viewer for Linux with some editing features.

![ClassicImageViewer](./misc/screenshot.png?raw=true "ClassicImageViewer") 

# Install dependecies

## Debian/Ubuntu
If you want to build on Qt5:
```
sudo apt install g++ make qtbase5-dev qttools5-dev-tools
```
As an alternative, if you want to build on Qt6:
```
sudo apt install g++ make qt6-base-dev qt6-l10n-tools
```

## Fedora
```
sudo dnf install g++ make qt6-qtbase-devel qt6-qttools-devel
```

# Recommended packages

## Debian/Ubuntu
```
sudo apt install libvips-dev libgraphicsmagick++1-dev libimage-exiftool-perl
```

## Fedora
```
sudo dnf install vips-devel GraphicsMagick-c++-devel perl-Image-ExifTool
```

# Get the source
If you have git installed:
```
git clone https://github.com/classicimageviewer/ClassicImageViewer.git
```
Otherwise download as ZIP and extract the content.

# Build from source
Enter the directory of the repository.
```
qmake .
make -j$(nproc)
make install
```
This will build and install for the current user, into
```
~/.local
```
You might need to log-out to make it available in the Main/Applications menu.

