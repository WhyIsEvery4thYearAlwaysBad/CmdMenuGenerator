@echo off
cls
g++ src/binds.cpp src/page.cpp src/launchoptions.cpp src/compiler.cpp src/tokens.cpp src/main.cpp -o cvm_generate.exe -std=c++2a -Wall