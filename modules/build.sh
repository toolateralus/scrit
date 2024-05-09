MODNAME=$1
if [ -z "$MODNAME" ]
then
  echo "No module name provided. Building all modules."
  for dir in */ ; do
    MODNAME=${dir%/}
    echo "Building module $MODNAME"
    if [ -f "${MODNAME}/build.sh" ]; then
      bash "${MODNAME}/build.sh"
    else
      sudo clang++ -std=c++2b -shared -fPIC -o /usr/local/scrit/modules/${MODNAME}.dll ${MODNAME}/${MODNAME}.cpp -lscrit
    fi
  done
else
  if [ -f "${MODNAME}/build.sh" ]; then
    bash "${MODNAME}/build.sh"
  else
    sudo clang++ -std=c++2b -shared -fPIC -o /usr/local/scrit/modules/${MODNAME}.dll ${MODNAME}.cpp -lscrit
  fi
fi