@echo off

pushd ..
Vendor\Premake\Windows\premake5.exe --file=KairosEngine-Setup.lua vs2022

popd
PAUSE