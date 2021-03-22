@echo off
pushd C:\Users\Owner\Documents\Projects\VoicemenuGenerator\src
g++ -m32 binds.cpp commandmenu.cpp launchoptions.cpp compiler.cpp tokens.cpp main.cpp -o ../cvm_generate_x32.exe -std=c++2a -Wall
if NOT %%ERRORLEVEL%% EQU 0 (
pause
)
g++ -m64 binds.cpp commandmenu.cpp launchoptions.cpp compiler.cpp tokens.cpp main.cpp -o ../cvm_generate_x64.exe -std=c++2a -Wall
if NOT %%ERRORLEVEL%% EQU 0 (
pause
)
popd