#!/bin/sh
# Config
COMPILER_X32="g++-10 -m32 * -o ../cmg-x32 -std=c++2a"
COMPILER_X64="g++-10 -m64 * -o ../cmg-x64 -std=c++2a"
# Script
## Exec in script dir.
BINDIR=$(dirname "$(readlink -fn "$0")")
cd "${BINDIR}" || exit 2
## Compile stuff
cd ../src > /dev/null || exit
if ${COMPILER_X32} "${@}"
then
	:
else
	read -r "Error(s) have occurred here! Press a key to exit."
	exit
fi
${COMPILER_X64} "${@}"
# Compile Windows binaries too.
COMPILER_X32="i686-w64-mingw32-g++ * -o ../cmg-x32 -std=c++2a -static"
COMPILER_X64="x86_64-w64-mingw32-g++ * -o ../cmg-x64 -std=c++2a -static"
${COMPILER_X32} "${@}"
${COMPILER_X64} "${@}"
# Done!