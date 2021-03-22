pushd src
g++ -m32 binds.cpp commandmenu.cpp launchoptions.cpp compiler.cpp tokens.cpp main.cpp -o ../cvm_generate_x32.exe -std=c++2a -Wall
if [ ! $? -eq 0 ]
then
	read -n1 -r -p "Error(s) have occurred here! Press a key to continue."
fi
g++ -m64 binds.cpp commandmenu.cpp launchoptions.cpp compiler.cpp tokens.cpp main.cpp -o ../cvm_generate_x64.exe -std=c++2a -Wall
if [ ! $? -eq 0 ]
then
	read -n1 -r -p "Error(s) have occurred here! Press a key to continue."
fi
popd