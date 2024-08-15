#!/bin/bash

# build the interpreter
echo -e "\e[33mBuilding...\e[0m"

if [ "$1" == "remake" ] || [ ! -d "build" ]; then
  mkdir -p build
  cd build
  sudo rm -r *
  cmake ..
else 
  cd build
  make clean
fi

output=$(make -j24 2>&1)
if [ $? -ne 0 ]; then
  echo "Error running make release: $output"
  exit 1
fi
echo -e "\e[32mInterpreter built successfully.\e[0m"
# cd out of build
cd ..
# create the library for -lscrit
echo -e "\e[33mCreating library for -lscrit...\e[0m"
output+=$(sudo cp build/lib/libscrit.a /usr/local/lib 2>&1)
if [ $? -ne 0 ]; then
  echo "Error running ar rcs or sudo mv for libscrit.a: $output"
  exit 1
fi
echo -e "\e[32mLibrary 'libscrit.a' installed at /usr/local/lib.\e[0m"

# create module path 
echo -e "\e[33mCreating module path...\e[0m"
output=$(sudo mkdir -p /usr/local/include/scrit 2>&1)
if [ $? -ne 0 ]; then
  echo "Error running sudo mkdir -p: $output"
  exit 1
fi
echo -e "\e[32mModule path '/usr/local/include/scrit' created.\e[0m"

# move headers into system includes
echo -e "\e[33mMoving headers into system includes...\e[0m"
output=$(sudo cp include/*hpp /usr/local/include/scrit/ 2>&1)
if [ $? -ne 0 ]; then
  echo "Error running sudo cp: $output"
  exit 1
fi
echo -e "\e[32mHeaders moved to /usr/local/include/scrit/.\e[0m"

# install std modules
echo -e "\e[33mInstalling standard modules...\e[0m"
cd modules
output=$(./build.sh 2>&1)
cd ..
if [ $? -ne 0 ]; then
  echo "Error running ./build.sh: $output"
  exit 1
fi
echo -e "\e[32mStandard modules installed.\e[0m"

# copy the scrit binary to /usr/local/bin
echo -e "\e[33mCopying 'scrit' binary to /usr/local/bin...\e[0m"
output=$(sudo cp bin/release/scrit /usr/local/bin/ 2>&1)
if [ $? -ne 0 ]; then
  echo "Error copying 'scrit' binary to /usr/local/bin: $output"
  exit 1
fi
echo -e "\e[32m'scrit' binary copied to /usr/local/bin.\e[0m"

echo -e "\e[36mInstallation complete. You can now use the 'scrit' command.\e[0m"