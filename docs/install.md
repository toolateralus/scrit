# Installation

Installation is very simple. Just run `./install.sh`.

This will: 

- Compile the lexer, parser & interpreter
- Create a statically linked library called libscrit.a in `/usr/lib/local`
- Move the `/include` folder's contents into `/usr/local/include/scrit/`
- Create an alias for the program called `scrit` which can be invoked from the command line to run files , as such
  `scrit inputFile.scrit`
  For debugging, see [Debugging](debug.md)