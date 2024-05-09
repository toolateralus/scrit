
# Currently available features

## declarations
  `i = 0`
   
  `array = []`
  
  `array = [0,1,2,3]`
  
  `object = {}`
  
  
  
## free functions 
  `func funcName() {}`
## anonymous function
```
// this executes in place.
func() {
  
}()

// assign the func to a var

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
  ```
  for {
  ...  
  }
  ```
  2. expression condition
  runs until the condition == false or is broken with break or return
  ```
  for true {
    
  }
  ```
  3. normal style with declaration, condition, increment
  ```
  for i=0, i<10, i=i+1 {
    println(i)
  }
  ```


## object literals, 


```
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
## imports & modules

### import a module as an object
the module will be available as an object represented by the module name
such as `math`
```
import math

println(math.sqrt(10))

```

### import specific symbols from a module

All specified symbols from the object will be available as first class members of this object

```
import {sqrt, floor, toInt} from math


println(sqrt(10) * floor(0.5) + toInt(1.0))

```

### import all symbols from a module

All symbols from the object will be available as first class members of this object

```
import * from math
println(sqrt(10 * 2) + floor(0.5))
```

Note: `math`, `system`, and `raylib` are the three current builtin modules.
functions like `println`, `readln`, `push` and `pop` are available without import (among a few others.)

# Todo: 

## language features.

#### null propagation, coalescing assignment
- value ?? default, value ??= valueifWasNull and obj?.func() or obj?.Value


#### try, catch , finally & throw keywords
- typical try {} catch (e) {} finally {}
- throw OBJECT/VALUE


#### compound assignment
- += , -= etc for arithmetic
- ++ and --


#### breakpoint debugging & language server


## interpreter features

#### call stack & better debugging info


