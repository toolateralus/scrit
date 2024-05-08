MODNAME=$1
if [ -z "$MODNAME" ]
then
  echo "No module name provided. Building all modules."
  for dir in */ ; do
    MODNAME=${dir%/}
    echo "Building module $MODNAME"
    sudo clang++ -std=c++2b -shared -fPIC -o /usr/local/scrit/modules/${MODNAME}.dll ${MODNAME}/${MODNAME}.cpp -lscrit
  done
else
  sudo clang++ -std=c++2b -shared -fPIC -o /usr/local/scrit/modules/${MODNAME}.dll ${MODNAME}.cpp -lscrit
fi