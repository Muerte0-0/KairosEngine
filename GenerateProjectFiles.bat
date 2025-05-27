@echo off

Vendor\Premake\Windows\premake5.exe --file=Build-KairosEngine.lua vs2022
popd
PAUSE