#!/bin/bash

pushd .. > /dev/null

Vendor/Premake/Linux/premake5 --file=KairosEngine-Setup.lua gmake2

popd > /dev/null

read -p "Press Enter to continue..."