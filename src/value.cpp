#include "value.hpp"

shared_ptr<Bool> Value::True = make_shared<::Bool>(true);
shared_ptr<Bool> Value::False = make_shared<::Bool>(false);
shared_ptr<Value> Value::Null = make_shared<::Null>();
shared_ptr<Value> Value::Undefined = make_shared<::Undefined>();