#!/bin/bash
set -e

sudo mkdir -p /usr/local/scrit/modules
# TODO: let's make a more automated way to do this.

cd system
./build.sh || { echo "Error building system mod"; exit 1; }

cd ../string
./build.sh || { echo "Error building string mod"; exit 1; }

cd ../array
./build.sh || { echo "Error building array mod"; exit 1; }

# cd ../raylib
# make -j24

