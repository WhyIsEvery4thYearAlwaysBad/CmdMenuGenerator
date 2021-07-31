#!/bin/sh
# Compile the program for windows.
# NOTE: Since <thread> is used, MinGW users are going to need to use POSIX MinGW.
COMPILER_X32="i686-w64-mingw32-g++-posix * -o ../cmg-x32 -std=c++2a -static"
COMPILER_X64="x86_64-w64-mingw32-g++-posix * -o ../cmg-x64 -std=c++2a -static"
# Set work directory to script's parent directory.
BINDIR=$(dirname "$(readlink -fn "$0")")
cd "${BINDIR}" || exit 2
# The actual script.
cd ../src > /dev/null || exit
if ${COMPILER_X32} "${@}"
then
	${COMPILER_X64} "${@}"
else
	printf '\033[31mFatal: Errors have occurred in MingW complation!\033[0m'
fi