pushd src
g++ -m64 binds.cpp page.cpp launchoptions.cpp compiler.cpp tokens.cpp main.cpp -o ../cvm_generate_x64.exe -std=c++2a -Wall
g++ -m32 binds.cpp page.cpp launchoptions.cpp compiler.cpp tokens.cpp main.cpp -o ../cvm_generate_x32.exe -std=c++2a -Wall
popd