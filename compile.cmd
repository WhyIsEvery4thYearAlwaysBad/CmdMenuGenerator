@echo off
cls
g++ src/launchoptions.cpp src/structs.cpp src/tokens.cpp src/lex.cpp src/main.cpp -o cvm_generate.exe -std=c++2a -Wall