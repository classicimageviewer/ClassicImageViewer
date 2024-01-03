#!/bin/sh

qmake .
make -s clean
make -s -j$(nproc)
cp build/civ AppRun/usr/bin
cp install/usr/share/icons/hicolor/256x256/civ.png Apprun
cp -r install/usr/share AppRun/usr
mkdir -p AppImage
chmod -R 0755 *
cd AppImage
wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
chmod +x linuxdeployqt-continuous-x86_64.AppImage
./linuxdeployqt-continuous-x86_64.AppImage ../AppRun/usr/share/applications/classicimageviewer.desktop -appimage -verbose=2 -bundle-non-qt-libs
cd ..

