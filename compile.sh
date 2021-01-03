pushd src
g++ binds.cpp page.cpp launchoptions.cpp compiler.cpp tokens.cpp main.cpp -o ../cvm_generate.exe -std=c++2a -Wall
popd