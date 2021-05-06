pushd C:/Users/Owner/Documents/Projects/CmdMenuGenerator/src
g++ -m32 -I"../utils/icu4c/include" * -o ../cmg-x32 -std=c++2a
if [ ! $? -eq 0 ]
then
	read -n1 -r -p "Error(s) have occurred here! Press a key to continue."
fi
g++ -m64 -I"../utils/icu4c/include" * -o ../cmg-x64 -std=c++2a
popd

