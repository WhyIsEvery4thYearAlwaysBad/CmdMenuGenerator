COMPILER_X32="g++-10 -m32 * -o ../cmg-x32 -std=c++2a"
COMPILER_X64="g++-10 -m64 * -o ../cmg-x64 -std=c++2a"
pushd ./src
${COMPILER_X32} ${@}
if [ ! $? -eq 0 ]
then
	read -n1 -r -p "Error(s) have occurred here! Press a key to exit."
	exit
fi
${COMPILER_X64} ${@}
# Compile Windows binaries too.
COMPILER_X32="i686-w64-mingw32-g++ * -o ../cmg-x32 -std=c++2a"
COMPILER_X64="x86_64-w64-mingw32-g++ * -o ../cmg-x64 -std=c++2a"
${COMPILER_X32} ${@}
${COMPILER_X64} ${@}
# Done!
popd
