#!/bin/bash

curl -SL https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage -o appimagetool

mkdir -p AppDir/usr/
rsync -av --exclude='*.o' --prune-empty-dirs linux/releasex64/ AppDir/usr/bin
cp linux/appimage/AppRun AppDir/
cp /usr/bin/yad AppDir/usr/bin
chmod +x AppDir/AppRun
chmod +x AppDir/usr/bin
ln -sr AppDir/usr/bin/quake2 AppDir/AppRun

cp linux/appimage/Zombono_512x512.png AppDir/Zombono.png
cp linux/appimage/Zombono.desktop AppDir/Zombono.desktop
mkdir -p AppDir/usr/share/applications && cp ./AppDir/Zombono.desktop ./AppDir/usr/share/applications
mkdir -p AppDir/usr/share/icons && cp ./AppDir/Zombono.png ./AppDir/usr/share/icons
mkdir -p AppDir/usr/share/icons/hicolor/512x512/apps && cp ./AppDir/Zombono.png ./AppDir/usr/share/icons/hicolor/512x512/apps
cp linux/appimage/icon.png AppDir/usr/share/icons/

mkdir -p assets/demo
curl -sSfL https://github.com/kondrak/Zombono2/releases/download/1.5.9/Zombono2-1.5.9_win64.zip -o assets/Zombono2-1.5.9_win64.zip
cd assets
unzip -qq *.zip **/zombonogame/players/* **/zombonogame/pak0.pak -d .
cp -r **/zombonogame/* demo
cd -
cp -r assets/demo AppDir/usr/bin/

chmod a+x appimagetool
./appimagetool AppDir/ Zombono.AppImage
