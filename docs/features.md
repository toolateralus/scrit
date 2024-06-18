
# Currently available features

Please, if you'd like to contribute to this documentation making it more robust, feel free to make pull requests with any amount of change.
Thanks.


## declarations
  `i = 0`
   
  `array = []`
  
  `array = [0,1,2,3]`
  
  `object = {}`
  

## arrays


### Pushing values 
```ts

array = []

// invalid, out of range.
array[0] = 10

// expand array (since its empty)
push(array, 10)

```

### Popping values
```ts

array = [0,1,2]


x = pop(array)

// x == 2

```

### Indexing & Subscript
```ts

array = [1]

array[0] = 10

println(array[0])

array = [func(arg) { println(arg)}]

array[0]("Hello")

// prints hello
```


### objects

regular object.
```ts

object = {
  member = 10
  func method() {
    // both of these are valid.
    // methods are executed from the object's context.
    println(this.member)
    println(member)
  }
}

object.member = 200

object.method()

```

map-style (kvps with [] access)
```ts
map = {}

value = 10

map["myKey"] = value

println(map["myKey"])

// prints 10
```



## compound assignment
- += , -= etc for arithmetic
- ++ and --
  
## free functions 

parameterless
  
`func funcName() {}`
  
  
parameterized function
``` go
func funcName(param, param1) {
  println(param, param1)
}
```


default values; 
``` go
func funcName(param = {}, param1 = [], param2 = SomeConstructorFunction()) {
  println(param, param1, param2)
} 

// then if we call

funcName()
// it will print the default values, otherwise if we called

funcName(0,1,2) 
// it would print 0, 1 and 2 because we provided those arguments.


funcName(undefined, undefined, 2)
// would still print 
// undefined, undefined, 2
// because values were provided.

```

  
## anonymous function
``` go
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

if you want to assign an object that may be null or undefined only when it is null or undefined you may use `??=`

``` js
myObject = { someThing = 20 }

myObject ??= { somethingElse = 10}

println(myObject) 

// this will print
// { someThing = 20 }
// because myObject was not null or undefined when ??= was used


myObject = undefined


myObject ??=  {
  
}

println(myObject)

// this will print
// {}
// because myObject was null or undefined (undefined in this case)
// when ??= was used.

```

for within expressions, you may use the same logic, but with `??`

``` js

myValue = undefined ?? 10

println(myValue)

// this will print 10 because the lhs was undefined


myValue = 10

myOtherValue = myValue ?? 200

println(myValue)

// this will print 10 because the lhs was not null or undefined when ?? was used.

```


## Modules

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



# Todo: 

## language features.

#### null propagation, coalescing assignment
- value ?? default, value ??= valueifWasNull and obj?.func() or obj?.Value


#### try, catch , finally & throw keywords
- typical try {} catch (e) {} finally {}
- throw OBJECT/VALUE

#### breakpoint debugging & language server


#### operator overloading for objects

similar to python, if theres a function named `add` or `divide` etc, 
we invoke that in an object instead of just not doing anything

```
func Vector2() {
  return {
    x = 0.0,
    y = 0.0,
    
    func add(other) {
      if (typeof(other.x) == "float" && typeof(other.y) == "float") {
        return {
          x = other.x + this.x,
          y = other.y + this.y
        }
      }
    }
  }
}


```

#### event functions

a C# style event to simply event handlers. 

would simply be a list of functions that get invoked when the () is used on an event type.

``` go
event myEvent

func subscriber() {
  println("My event was invoked")
}

myEvent += subscriber

myEvent()

```

#### coroutines

go style coroutines. this would require some significant changes tothe interpreter to allow multi threading,
or a special subset of features limited to async. like mutex / locked variable access when coming from a coroutien,
a special scope maybe?
``` 
start func() {
  // coroutine body
}()
```

#### switch statements
C# style switch statements, can compare any value
``` C#
switch ("") {
  case "":
  break
  case "1":
  ...etc
}

```





## interpreter features

#### call stack & better debugging info


