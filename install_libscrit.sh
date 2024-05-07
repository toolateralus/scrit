#!/bin/bash

make clean ; make

ar rcs libscrit.a obj/*.o

sudo mv libscrit.a /usr/local/lib
sudo mkdir -p /usr/local/include/scrit
sudo cp include/*hpp /usr/local/include/scrit/