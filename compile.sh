pushd ./src
if [ "$1" == "--debug" ] || [ "$1" == "-g" ]
then
	g++ -g -m32 * -o ../cmg-x32 -std=c++2a
	if [ ! $? -eq 0 ]
	then
		read -n1 -r -p "Error(s) have occurred here! Press a key to continue."
	fi
	g++ -g -m64 * -o ../cmg-x64 -std=c++2a
else 
	g++ -m32 * -o ../cmg-x32 -std=c++2a
	if [ ! $? -eq 0 ]
	then
		read -n1 -r -p "Error(s) have occurred here! Press a key to continue."
	fi
	g++ -m64 * -o ../cmg-x64 -std=c++2a
fi
popd