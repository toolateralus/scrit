

# Standard functions accessible with no includes:

## Important!
Any function in scrit can be called like an extension method. That is, if you called
`10.println()` 
the interpreter would actally execute
`println(10)`

This works for any free-function, which includes all standard functions _listed here_, and any _native callable_ like those loaded from a Scrit Module.


## General

### typeof
get a string representing the type of the object passed in.

#### returns 
a string like "int", "float", "string", "array", "object", "callable", "bool"

#### parameters
any object as reference.

#### usage
```go
println(
  typeof(1),
  typeof(1.0),
  typeof({}),
  typeof([]),
  typeof(false),
  typeof(func() {})
)
// output: 
// "int"
// "float"
// "object"
// "array"
// "bool"
// "callable"
```

  Note that builtin functions like println and anything listed here, or from an
  `import` library, will be typeof `"native_callable"`, indicating it was written in C++.

## Terminal / IO

### println
#### parameters (varargs)
print a string or any value (that gets converted to a string) to stdout. a newline is automatically inserted after the value or string.

note: you may pass any number of arguments to this function, and they will be treated like an array of values, printed in order with newlines after each value..

#### usage:
```go
  println(0)
  // output 0
  
  println({ value = 100, string = "hello!"})
  
  // output
  // {
  //   "value": 100
  //   "string": hello! 
  // }
  
  println(0, 1, 2, 3, 4)
  // output:
  // 0
  // 1
  // 2
  // 3
  // 4
```
---
### print
#### parameters (varargs)
works exactly like [println](#println) except it does not insert a newline.


#### usage
```go
print(0,1,2)
// output
// 012
```
---
---
### cls
clear screen. clears the current terminal window.
#### usage
```go
  cls()
```
---
---
### set_cursor

#### parameters (int: x, int: y)

sets the terminal's cursor to the specified (x, y) position.
the coordinate space it uses is (x: char, y: line)
#### usage 
```go
x = 10
y = 10
set_cursor(x,y)
// now, the next write from the terminal will start at 10,10. 
```

---
---
### readln 

this is a blocking call. it will not return until input has been fetched, ie enter was pressed in the terminal.

read a line from the stdin.

#### returns: 
a string containing the read line.



```go
input = readln()
// wait for user to type something. 
// say they typed 'name' and pressed enter then..
println(input)
// would output:
// name
```
---
---
### readch

works a lot like [readln](#readln), except it returns a string containing
one character, as it's typed. no enter should be neccesary, just a keystroke into the terminal

#### returns
a character that was typed into the terminal.

#### usage:

```go
word = ""
for word != "password" {
  c = readch()
}
println("Got password! " + word)
```

---

## Array and String manipulation functions

### len

get the length of a string or array.


#### returns
 the length of a string or an array.
 
#### usage
```go
  my_string = "Hello!"
  
  println(len(my_string))
  // output 
  // 6
  
  my_array = [0, 1, 2, 3, 4, 5]
  println(len(my_array))
  // output
  // 6
```
---
---

### push

push an element on to the back of a string or array. aka, append

#### parameters
  array|string : the reference to the object to push into.
  varargs: any number of elements of any type to push into the reference.
  
  Note that strings must take other strings to concatenate.

#### returns 
  the modified array, however, this modifys the reference passed to the function in place.
  
#### usage

```go
my_array = []
my_string = ""

push(my_array, 0)
push(my_array, 0, 1, 2)
push(my_string, "hello!", " goodbye!")

println(my_array, my_string)

// output
// [0, 0, 1, 2]
// hello! goodbye!

```
---
---


### pop

pops an element from the back of a string or array. the inverse operation of
[push](#push)

#### parameters
  the string or array to mutate.

#### returns
  the element popped off the reference.
  
#### usage

```go
my_array = [0,1,2]

my_value = pop(my_array)

println(my_value)
// output:
// 2
```

---
---

### clear

clears the elements from an array, or clears a string leaving an empty string.

#### parameters
the array or string to clear.

#### usage

```go
my_array = [0,1,2] // the same works for strings.

clear(my_array)

println(my_array)

// output
// []
```

---
---

### expand

expands an array to the specified length, with an optional default value. otherwise, it fills it with undefined.

#### parameters

array: the array to mutate.
int: the new length.
default_value?: optional, a default value to insert in the new pushed elements. otherwise, it's undefined.
 

#### usage

This will not write over existing elements in the array already.

if you have an array 
`array = [0,1,2]` 

and you call
`expand(array, 4, 0)`

you will get 
`[0,1,2,0]`

as the expand only expands the array to the length specified.

You can also pass a callback that will get called each iteration of the expansion, to return specific values, or constructed objects.

```go
  i = 0
  array = expand([], 50, func() {
    return i++
  })
  println(array)
  // output:
  // some output left out for brevity.
  // [0,1,2,3,4,5....50]
  
```

```go
my_array = []

expand(my_array, 5)

println(my_array)
// output
// [0,0,0,0,0]


my_array = []

expand(my_array, 5, {})

println(my_array)
// output
// [{}, {}, {}, {}, {}]
```
---
---

### String only

#### tostr

convert any value to a string value.

#### parameters
any: the value to be converted

#### returns
string: the converted value.


#### usage

```go
// Note that println and other print functions already do this
// internally, this is just an example.

str = tostr({value = 10})
println(str)

// output
// {
//  "value": 10 
// }

str = tostr(0.1)
println(str)
// output
// 0.1

```
 

## Serializer
### serialize
Serializes an object into a string using the specified settings.
#### parameters
- `value`: The object to serialize.

- `options`: A descriptor object telling the serializer how to format the output.

The options object can have any or none of these fields. if the field is not found, it will be assumed default.
  - `indentSize` (optional): The number of spaces to use for indentation. Default is 0.
  - `startingIndent` (optional): The starting indentation level. Default is 0.
  - `referenceHandling` (optional): The reference handling mode. Possible values are "mark", "remove", and "preserve". Default is "mark".
  
#### returns
A string representing the serialized object.
#### usage

```go
options = {
  indentSize = 1,
  startingIndent = 0,
  referenceHandling = "mark"
}

serialized = serialize({value = 10}, options)
```

---
---


## Character recognition functions
### isalnum
returns true if the character was alphabetical, or numeric.
returns false otherwise.
### isdigit
returns true if the character was a number
returns false otherwise.
### ispunct
returns true if the character was ascii punctuation, like `!@#$%^&*()_` among many others.
returns false otherwise.
### isalpha
returns true if the character was alphabetical.
returns false otherwise.

#### usage

```go
c = "!"
println(isalnum(c), isdigit(c), ispunct(c), isalpha(c))
// output:
// false
// false
// true
// false
```
