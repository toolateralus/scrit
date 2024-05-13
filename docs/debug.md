## Breakpoint debugger in cmd line.

Breakpointing can be done with
`scrit inputFile.scrit br:lineNumber`  

such as `scrit myFile.scrit br:100`

when this breakpoint gets hit, 
`c` or `continue` will go till the next breakpoint
`over` or `ov` steps over this breakpoint, often to the next line of code.
`in` or `i` steps into a function or block-statement if possible.
`out` or `o` steps out of a function that was previously stepped into
`inspect` or `s` prints the variables and functions in the current scope.