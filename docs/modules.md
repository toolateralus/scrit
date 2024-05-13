

### defining a module in C++

- first, have the interpreter built & installed. run `./install.sh` from the root of the source.

- navigate to the /tools directory in the source : `cd tools`
- note: if you want to create this module elsewhere, just copy the module tool file into whatever dir you want your module to exist in and run it there.

- run the `module_tool.scrit` file with one argument: the name of your module. for myTestModule: `scrit module_tool.scrit myTestModule` 

you should now have
 
``` 
myTestModule
  ├── MyTestModule.cpp
  └── build.sh
```

### Declaring functions in C to be embedded in Scrit

#### Some info about the C++ types that we use to represent values in Scrit

there is a typedef alias for `shared_ptr<Value_T>` called Value.
this is the base how all values are represented in Scrit.

`Bool_T`, `Float_T`, `String_T`, `Int_T`, `Object_T`, `Array_T`, `Undefined_T`, and `Null_T` all inherit from `Value_T`.

these are also typedef aliases, and the actual types are all postfixed by a `_T`. this just allows less obnoxious code in the back end, as typing
`shared_ptr` can get cumbersome and clog up the code.

so, a full list of types (using their typedefs) is:

- `Bool`
- `String`
- `Int`
- `Float`
- `Array`
- `Object`
- `Null`
- `Undefined`

#### How native functions are represented in C++

A `NativeFunction` is the type that encapsulates an embedded function.
It contains a name identifier, return type `enum ValueType` and a
`vector<pair<ValueType>, string>` which represents the parameters
to your function. the `return type` and `parameters` are optionally defined.


#### Function signature of your defined functions
The actual function pointer signature looks as follows:

`Value myFunction(std::vector<Value> args) {...}`

All functions must return a value. Void functions return `Value_T::UNDEFINED`


#### Using the arguments vector & writing a simple `floor` function
To fetch arguments, first we range check for the expected parameter length,
then we can use the helpers in `Ctx::` static class, to extract usable values.

so for example, if we want a `floor()` function that takes a float and returns an int:

```cpp
Value floor(std::vector<Value> args) {
  if (args.empty()) {
    // it is somewhat conventional to return either
    // undefined for improper arguments, but in most cases 
    // its preferable to return a string that can be handled as
    // 'err' or something.
    return Value_T::UNDEFINED;
  }
  
  float value;
  // note, if this function returns false,
  // the ref / out value remains uninitialized, or as it was.
  // `value` is only mutated if this returns true.
  if (!Ctx::TryGetFloat(args[0], value)) {
    return Value_T::UNDEFINED;
  }
  
  // use Create*typename* for most value initializations.
  return Ctx::CreateInt((int)value);
}

```
  
#### Including functions and variables in your `ScritModDef`
Then, when we're done creating our function(s), We can modify the `InitScritModule_myTestModule` function to actually include them.

``` cpp
extern "C" ScritModDef *InitScritModule_myTestModule() {
  ScritModDef *def = CreateModDef();
  *def->description = "A simple test module";
  AddFunction(def,
    "floor",  // identifier this function will be called with
    &floor, // a pointer to our function.
    ValueType::Int,  // return type
    {{ValueType::Float "value_toFloor"}} // parameter signatures
  );
  
  // If you don't want to include function information for the LSP to use for auto complete etc
  
  // this is a minimal way to do so.
  AddFunctionNoInfo(def, "floor", &floor);
  
  return def;
}

```

In a similar fashion, we can define any global variables in our module.

```cpp

extern "C" ScritModDef *InitScritModule_myTestModule() {
  ScritModDef *def = CreateModDef();
  *def->description = "A simple test module";
  
  auto object = Ctx::CreateObject(); // create a new object.
  
  object->scope->variables["myIntegerVariable"] = Ctx::CreateInt(100); // add a member to the object called myIntegerVariable
  
  AddVariable(def, "myObject", object);
  AddVariable(def, "myInteger", Ctx::CreateInt(20000));
  
  return def;
}

```

Now, if you want to use your module, just run `./build.sh` and it will install it to `/usr/local/scrit/modules/myModuleName`

to use the module in the language, just use import syntax with your given module name.


``` ts 
// import all the objects and functions as first-class members, so theyre avaiable in this scope with no prefix.
import * from myTestModule

x = floor(10.2)

// import the module as an object
import myTestModule

x = myTestModule.floor(10.2)

// import specific members from the module as first class members
import {floor, myObject, myInteger} from myTestModule

x = floor(1.02 * myInteger * myObject.myIntegerVariable)

```




