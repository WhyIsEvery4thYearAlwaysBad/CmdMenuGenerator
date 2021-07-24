#!/bin/sh
# Compiles program for linux.
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
	${COMPILER_X64} "${@}"
else
	printf '\033[31mFatal: Errors have occurred in MingW complation!\033[0m'
fi
