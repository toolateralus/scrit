#!/bin/bash

# build the interpreter
echo -e "\e[33mBuilding...\e[0m"
output=$(make release 2>&1)
if [ $? -ne 0 ]; then
  echo "Error running make release: $output"
  exit 1
fi
echo -e "\e[32mInterpreter built successfully.\e[0m"

# create the library for -lscrit
echo -e "\e[33mCreating library for -lscrit...\e[0m"
output=$(ar rcs libscrit.a obj/release/*.o 2>&1)
output+=$(sudo mv libscrit.a /usr/local/lib 2>&1)
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

echo -e "\e[33mAdding 'scrit' alias to bashrc...\e[0m"
if grep -q 'alias scrit=' ~/.bashrc; then
  # If the alias exists, replace it
  sed -i "/alias scrit=/c\alias scrit='$(pwd)/bin/scrit'" ~/.bashrc
  output=$?
else
  # If the alias doesn't exist, append it
  output=$(echo "alias scrit='$(pwd)/bin/release/scrit'" >> ~/.bashrc 2>&1)
fi
if [ $? -ne 0 ]; then
  echo "Error adding 'scrit' alias to bashrc: $output"
  exit 1
fi

echo -e "\e[32mAlias 'scrit' added to bashrc.\e[0m"
echo -e "\e[36mRestart your terminal to use the 'scrit' command.\e[0m"