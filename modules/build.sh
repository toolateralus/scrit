#!/bin/bash
set -e

# TODO: let's make a more automated way to do this.

cd math
./build.sh || { echo "Error building math mod"; exit 1; }

cd ../system
./build.sh || { echo "Error building system mod"; exit 1; }

cd ../raylib
./build.sh || { echo "Error building raylib mod"; exit 1; }

cd ../sstream
./build.sh || { echo "Error building sstream mod"; exit 1; }

cd ../string
./build.sh || { echo "Error building string mod"; exit 1; }

cd ../array
./build.sh || { echo "Error building array mod"; exit 1; }