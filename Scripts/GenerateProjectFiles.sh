#!/bin/bash

set -e # stop on error

pushd .. > /dev/null

./Vendor/Premake/Linux/premake5 --file=KairosEngine-Setup.lua gmake

popd > /dev/null
