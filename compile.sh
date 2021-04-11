pushd C:/Users/Owner/Documents/Projects/CmdMenuGenerator/src
g++ -g -m32 * -o ../cmg-x32.exe -std=c++2a -Wall
if [ ! $? -eq 0 ]
then
	read -n1 -r -p "Error(s) have occurred here! Press a key to continue."
fi
g++ -g -m64 * -o ../cmg-x64.exe -std=c++2a -Wall
if [ ! $? -eq 0 ]
then
	read -n1 -r -p "Error(s) have occurred here! Press a key to continue."
fi
popd