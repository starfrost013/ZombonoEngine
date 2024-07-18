#!/usr/bin/bash
# auto-increments the game's build number using less crazy hacks on linux
# Can't pipe to same file

win64/awk '/VERSION_BUILD/{$3=$3+1""}1;' ../engine/common/version.h > ../engine/common/version2.h
rm ../engine/qcommon/version.h
mv ../engine/qcommon/version2.h version.h