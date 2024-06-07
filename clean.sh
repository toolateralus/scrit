echo running make clean '(removes binary and obj files)'
make clean

echo removing all modules
sudo rm -rf /usr/local/scrit/modules/*

echo removing static library 'libscrit.a'
sudo rm -rf  /usr/local/lib/libscrit.a

echo removing headers from /usr/local/include/scrit
sudo rm -rf /usr/local/include/scrit/*
