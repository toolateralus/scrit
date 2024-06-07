#!/bin/bash
set -e

# TODO: let's make a more automated way to do this.

cd math
./build.sh || { echo "Error building math"; exit 1; }

cd ../system
./build.sh || { echo "Error building system"; exit 1; }

cd ../raylib
./build.sh || { echo "Error building raylib"; exit 1; }