
# Currently available features

Please, if you'd like to contribute to this documentation making it more robust, feel free to make pull requests with any amount of change.
Thanks.


## declarations

Variables without mutability modifiers are const by default.

in the below examples, to make any of these mutable,
we would just use

`let mut ...`

or to explicitly declare const

`let const ...`

### type inferred declarations:


`let i = 0` : `type= int`

`let b = false` : `type= bool`

`let f = 0.0` : `type= float`
  
`let arr = []` : `type= array`

`let arr = [0,1,2,3]` : `type= array<int>`

`let obj = {}` : `type= object`
  
### explicitly typed declarations:
  
`let i: int = 0`

`let f: float = 0.0`

`let b: bool = false`

`let a: array = []`

`let a: array<int> = [0,1,2,3]`

`let obj: object = {}`

### re-definition of variables

```rust
let x = 0
let x = false
```

This is valid syntax. 
re-defining a variable will destroy any existing variables
that exist with that name in this scope, and provide the new definition.

variable shadowing works, so doing the below will create a new local
not overwrite the existing variable in the parent scope.
```rust
let something = 10
func f() {
  let something = 20
  println(something)
}
```



## arrays

### Pushing values 
This is one example of the many functions provided by `std::array`
Check the module's documentation for more info.

```rust

// include the array methods from the std::array
// module. 
using std::array

let mut arr = []

// invalid, out of range.
arr[0] = 10

// after this, arr[0] == 10.
arr.push(10)

```

### Indexing & Subscript

```rust 

let mut arr = [1]

// prints 1
println(arr[0])

arr[0] = 10

// prints 10
println(arr[0])

// place a function pointer in the array
arr = [func(f : any) { println(arg)}]

arr[0]("Hello")

// prints hello
```


### objects

regular object.
```rust

let object = {
  member = 10 // if this were to be mutated, it would need
  mut member = 10
  
  // for methods, we need to explicitly take self as a parameter.
  // If not, the interpeter will pass it to it, and it will throw an error complaining about too many arguments being passed to a function.
  func method(self: object) {
    println(self.member)
  }
  
  // to make mutations to self, simply declare a mut param.
  func method1(mut self: object) {
    self.member = 100
  }
}

// only works if its declared in the object as mut.
object.member = 200

// call the methods.
object.method()

object.method1()

```

map-style (key-value pairs with [] access)
```ts
let map = {}

value = 10

// this fails, because we have a strict type system.
// we cannot declare variables in this fashion.
// this is to be worked on, to make dictionaries cheap and easy.
map["myKey"] = value


// prints null or throws an error complaining about not finding the variable.
println(map["myKey"])

// prints 10
```



## structs

structs are currently the only way to define a custom type.
They do not have oop features like inheritance, but they do have member methods.

They work just like [objects](#objects), except they are strongly typed, and not treated as a generic object.

```rust

struct MyStruct {
  func print_members(self: MyStruct) {
    // this return tuples, key being a string, and v being 
    // whatever type that field is.
    for k,v : self {
      println(k + ": " + v.tostr())
    }
  }
}

```


## compound assignment

- += , -= etc for arithmetic
- ++ and --

these are all valid statements.
```rust
let mut i = 0
i += 10
i -= 10
i++
++i
--i
i++
```
  
#
  
## free functions 

### function typing & inferred 'null' return type.

function declarations have many modes

`func someFunc() {}`

has a type of `func() -> null`

`func someFunc() -> int { 0 }`

has a type of `func() -> int`

  
### parameterized function

#### for const (immutable) parameters..
``` go
func funcName(param : paramType, param1: paramType) {
  println(param, param1)
}
```
#### for mutable parameters..
```go 
func funcName(mut param: paramType, ...) {...}
```

#### default values; 
``` go
func funcName(param: object = {}, param1: array = [], param2: someType = someType()) {
  println(param, param1, param2)
} 

// then if we call

funcName()
// it will print the default values, otherwise if we called

funcName(0,1,2) 
// it would print 0, 1 and 2 because we provided those arguments.


funcName(null, null, 2)
// would still print 
// undefined, undefined, 2
// because values were provided.

```


## type constructors

all types can be default constructed with this syntax

`let i : int` 

or in an object 
```go
{ 
  i : int,
  v : int,
}
```

however, explicit constructors can be used.

`let i = int() // defaults to 0`

`let instance = MyStruct()`

and for structs, explicit constructors will instantiate members in the order they were declared. so, for this struct:

also, note that const fields can be set with constructors, but they cannot be set again after instantiation.
```rust
struct Some {
  const n: int, 
  const v = false,
}

let instance = Some(10, true)

println(instance)

// prints 
// { "n": 10, "v": true }

```


## anonymous function
``` go
// this executes in place.
func() {
  
}()

// assign the func to a var, a fn pointer.

f = func() {
  
}
```

## if else 
  ```
  if i == 0 {
     println("b") 
  } else {
    println() 
  }
  ```
  
## for loops 
  1. no condition, runs until explicitly broken with 'break' or a return
  ``` go
  for {
  ...  
  }
  ```
  2. expression condition
  runs until the condition == false or is broken with break or return
  ```go
  for true {
    
  }
  ```
  3. normal style with declaration, condition, increment
  ```go
  for i=0, i<10, i=i+1 {
    println(i)
  }
  ```


## object literals, 


``` go
o = { 
  field1 = 0 
  field2 = 10
  func method() {
    // this executes in the context of the owner object.
    // no 'this' keyword is required.
    return  field1 + field2
  }
}
```

## Null coalescing

if you want to assign an object that may be null  only when it is null you may use `??=`

``` js
myObject = { someThing = 20 }

myObject ??= { somethingElse = 10}

println(myObject) 

// this will print
// { someThing = 20 }
// because myObject was not null when ??= was used


myObject = null


myObject ??=  {
  
}

println(myObject)

// this will print
// {}
// because myObject was null
// when ??= was used.

```

for within expressions, you may use the same logic, but with `??`

``` js

myValue = null ?? 10

println(myValue)

// this will print 10 because the lhs was null


myValue = 10

myOtherValue = myValue ?? 200

println(myValue)

// this will print 10 because the lhs was not null when ?? was used.

```

## Pattern matching

scrit has a rust-like pattern matching system using the `match` keyword.
the syntax is nearly identical to `rust`.



It can be used as an expression as such:
```rust
variable = match some_expression {
  0 => 1
  1 => 0
  2 => {
    some_value = 3.5 * factor_of("some_condition")
    return some_value
  }
}
```

It can be used as a statement as such:
```rust
match get_state_indicator() {
  0 => println("Error! Something happened.")
  1 => mutate_based_on_key(system_manager, Actions.Rotate)
  2 => {
    system.exit()
  }
}
```

default values are denoted by the `default` keyword


```rust
match "this could be any string.. that's a lot of possible values" {
  default => panic("that wasn't supposed to happen...")
  "some specific string" => {
    // do something.
  }
}
```


note that a default could be defined several times, and the latest default will be taken.
On the other hand, if an expression case is defined several times, the first defined occurence of that value will always be taken.


```rust

match 0 {
  0 => println("this will happen")
  0 => println("this will not happen")
}


match 1 {
  0 => println("this will not happen")
  0 => println("this will not happen")
  default => println("this will also not happen")
  default => println("because this will happen")
}
```

This is because there is only one default, whereas we don't know the value of the expressions until interpretation.
So, each time a default is defined, it's overwriting the old one, however, the parser has no idea several expressions that
evaluate to the same value have been defined. beware!


## Property fields
declaring a variable like
`f => ..some expression..`
will result in a `property` which is calculated each time it is used. properties are read only

this syntax produces the same effect
``` go
f => {
  ... some lambda body...
}
```

example:

```go
vec2 = {
  x = 0,
  y = 0,
  mag => sqrt(x*x + y*y)
}

vec2.x = 10
vec2.y = 10

println(vec2.mag)
// prints 14.142136

```


## Range based for loops 

Range based for loops can be used to iterate over a collection of elements.

A collection could be
- an array
- an object or struct instance.
  
the syntax goes as follows:

```rust
for LOCAL_VARIABLE : MY_COLLECTION {...}

// OR

for tuple, deconstructed, here : MY_OBJECT_OR_TUPLE_ARRAY { ... }
```

For more readable examples

```rust


// Tuples deconstructed from an array
let n = [(0,0), (1,1)]
for k,v : n {
  println(k)
  println(v)
}
// tuple deconstruct from an object's key value pairs
let obj =  {
  some: int,
  other: int,
}

for k,v: obj {
  ...
}

// just iterating over a normal array
let arr =  [0,1,2,3,4]
for i : arr {
  ...
}
```


## Tuples

simple tuples are supported with `(v, v1, ...)` syntax.
currently, the only way to access a tuple element is to destructure it into fields, with
`v, v1, v2 = (0, 1, 2)` syntax. note the rhs of the destructure does not need to be a literal,
it can be any tuple identifier or return value.


## Lambdas

we have a `=> expression` and `=> { block returning some value}` syntax for 'lambda's.

So, as we saw in [Pattern matching](#pattern-matching), this is fairly clean and very `rust-like`. However, for assigning a variable, currently (subject to change), this is what we are looking at.


```js

// note that if we omitted the => here,
// we would just be creating an object.
// variable = { v = ..., x = ..., etc.}

variable => {
  v = do_something(state_obj, intensity)
  x = something_else(state_obj, v, intensity)
  return complete_action(v, x, intensity)
}

// OR...

// there is no real purpose to do this yet.
// later, we will have C# style property fields which get evaluated like a getter each time it's accessed.
variable => 1


```

## Omitting the return keyword for the last statement in a block

Just like rust, you can return a value by placing a single operand at the end of a block, instead of using return.

So, for example, we can change this function

```go
func get_something(state: MyStateType) -> int{
  if state.entities >= 100 {
    return 2.0
  }
  return 0.0
}

```
To look more like this


```go
func get_something(state: MyStateType) -> int {
  if state.entities >= 100 {
    2.0
  }
  0.0
}
```
It does the same thing, but the return is implicit.

## Modules

This documentation is totally obsolete.
The module system is undergoing a full rework and is to be determined
just exactly how it will work.
For defining modules, see the [modules.md](modules.md) file.
### using a module as an object
the module will be available as an object represented by the module name
such as `math`
``` ts
using math

println(math.sqrt(10))

```

### using specific symbols from a module

All specified symbols from the object will be available as first class members of this object

``` ts
using {sqrt, floor, toInt} from math



println(sqrt(10) * floor(0.5) + toInt(1.0))

```

### using all symbols from a module

All symbols from the object will be available as first class members of this object
 
``` ts
using * from math
println(sqrt(10 * 2) + floor(0.5))
```

Note: `math`, `system`, and `raylib` are the three current builtin modules.
functions like `println`, `readln`, `push` and `pop` are available without using (among a few others.)





