#include <memory>
#include <scrit/native.hpp>
#include <scrit/scritmod.hpp>
#include <scrit/type.hpp>
#include <scrit/ctx.hpp>

#pragma clang diagnostic ignored "-Wunused-parameter"

#define function(name) Value name(std::vector<Value> args)

function(isalnum) {
  auto arg = args[0];
  string value;
  if (Ctx::TryGetString(arg, value)) {
    if (value.empty()) {
      return Value_T::False;
    }
    return Ctx::CreateBool(std::isalnum(value[0]));
  }
  return Ctx::Null();
}
function(isdigit) {
  auto arg = args[0];
  string value;
  if (Ctx::TryGetString(arg, value)) {
    if (value.empty()) {
      return Value_T::False;
    }
    return Ctx::CreateBool(std::isdigit(value[0]));
  }
  return Ctx::Null();
}
function(ispunct) {
  auto arg = args[0];
  string value;
  if (Ctx::TryGetString(arg, value)) {
    if (value.empty()) {
      return Value_T::False;
    }
    return Ctx::CreateBool(std::ispunct(value[0]));
  }
  return Ctx::Null();
}
function(isalpha) {
  auto arg = args[0];
  string value;
  if (Ctx::TryGetString(arg, value)) {
    if (value.empty()) {
      return Value_T::False;
    }
    return Ctx::CreateBool(std::isalpha(value[0]));
  }
  return Ctx::Null();
}
function(split) {
  if (args.size() < 2 ||
      args[0]->GetPrimitiveType() != Values::PrimitiveType::String ||
      args[1]->GetPrimitiveType() != Values::PrimitiveType::String) {
    return Ctx::Null();
  }

  string s;
  if (!Ctx::TryGetString(args[0], s)) {
    return Ctx::Null();
  }

  string delim;
  if (!Ctx::TryGetString(args[1], delim)) {
    return Ctx::Null();
  }

  if (!s.contains(delim)) {
    return Ctx::CreateArray();
  }

  char delimiter = delim.at(0);
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(s);

  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }

  return Ctx::FromStringVector(tokens);
}
function(substring) {
#define undefined Ctx::Null()
  if (args.size() < 3 ||
      args[0]->GetPrimitiveType() != Values::PrimitiveType::String) {
    return undefined;
  }

  auto str = args[0]->Cast<String_T>();
  std::pair<int64_t, int64_t> indices;
  
  if (!Ctx::TryGetInt(args[1], indices.first)) {
    return undefined;
  }
  if (!Ctx::TryGetInt(args[2], indices.second)) {
    return undefined;
  }
  return Ctx::CreateString(str->value.substr(indices.first, indices.second));
}
function(indexOf) {
#define undefined Ctx::Null()
  if (args.size() < 2 ||
      args[0]->GetPrimitiveType() != Values::PrimitiveType::String) {
    return undefined;
  }
  auto str = args[0]->Cast<String_T>();
  auto srch_c = args[1]->Cast<String_T>();
  size_t found = str->value.find(srch_c->value);
  if (found != std::string::npos) {
    return Ctx::CreateInt(found);
  }
  return Ctx::CreateInt(-1);
}
function(push) {
  if (args.empty()) {
    return Ctx::Null();
  }
  if (args[0]->GetPrimitiveType() == PrimitiveType::String) {
    auto arg = static_cast<String_T *>(args[0].get());
    for (size_t i = 1; i < args.size(); i++) {
      arg->value += args[i]->ToString();
    }
  }
  return Ctx::Null();
}
function(front) {
  if (args.size() == 0 ||
      args[0]->GetPrimitiveType() != Values::PrimitiveType::String) {
    return Ctx::Null();
  }
  string str;
  if (Ctx::TryGetString(args[0], str) && str.length() != 0) {
    return Ctx::CreateString(string(1, str.front()));
  }
  return Ctx::Null();
}
function(back) {
  if (args.size() == 0 ||
      args[0]->GetPrimitiveType() != Values::PrimitiveType::String) {
    return Ctx::Null();
  }
  string str;
  if (Ctx::TryGetString(args[0], str) && str.length() != 0) {
    return Ctx::CreateString(string(1, str.back()));
  }
  return Ctx::Null();
}
function(pop) {
  if (args.empty() || args[0]->GetPrimitiveType() != PrimitiveType::String) {
    return Ctx::Null();
  }

  auto str_value = static_cast<String_T *>(args[0].get());
  if (str_value->value.empty()) {
    return Ctx::Null();
  }

  string character = std::string(1, str_value->value.back());
  str_value->value.pop_back();
  return Ctx::CreateString(character);
}
function(len) {
  if (args.empty() ||
      args[0]->GetPrimitiveType() != Values::PrimitiveType::String) {
    return Ctx::Null();
  }

  string result;
  if (Ctx::TryGetString(args[0], result)) {
    return Int_T::New(result.length());
  }

  return Ctx::Null();
}

function(insert) {
  if (args.size() < 3 ||
      args[0]->GetPrimitiveType() != Values::PrimitiveType::String ||
      args[1]->GetPrimitiveType() != Values::PrimitiveType::Int) {
    return undefined;
  }
  auto str = std::static_pointer_cast<String_T>(args[0]);
  auto &value = str->value;
  
  int64_t index;
  if (!Ctx::TryGetInt(args[1], index)) {
    return undefined;
  }

  string insertion;
  if (!Ctx::TryGetString(args[2], insertion)) {
    return undefined;
  }

  if (index < 0 || (size_t)index > value.size()) {
    return undefined;
  }

  value.insert(index, insertion);

  return Ctx::CreateString(value);
}

function(contains) {
  if (args.size() < 2) {
    return Ctx::Null();
  }

  string result;
  if (!Ctx::TryGetString(args[0], result)) {
    throw std::runtime_error("contains may only be used on strings");
  }

  string comparison;
  if (!Ctx::TryGetString(args[1], comparison)) {
    throw std::runtime_error("contains may only be used on strings");
  }

  if (result.find(comparison) != std::string::npos) {
    return Ctx::CreateBool(true);
  }

  return Ctx::CreateBool(false);
}

function(replace) {
  if (args.size() < 3 ||
      args[0]->GetPrimitiveType() != Values::PrimitiveType::String ||
      args[1]->GetPrimitiveType() != Values::PrimitiveType::String ||
      args[2]->GetPrimitiveType() != Values::PrimitiveType::String) {
    return Ctx::Null();
  }

  string str;
  if (!Ctx::TryGetString(args[0], str)) {
    return Ctx::Null();
  }

  string pattern;
  if (!Ctx::TryGetString(args[1], pattern)) {
    return Ctx::Null();
  }

  string replacement;
  if (!Ctx::TryGetString(args[2], replacement)) {
    return Ctx::Null();
  }

  size_t pos = 0;
  while ((pos = str.find(pattern, pos)) != std::string::npos) {
    str.replace(pos, pattern.length(), replacement);
    pos += replacement.length();
  }

  return Ctx::CreateString(str);
}

function(remove) {
  if (args.empty()) {
    return Ctx::Null();
  }

  if (args[0]->GetPrimitiveType() == Values::PrimitiveType::String) {
    string str;
    if (!Ctx::TryGetString(args[0], str)) {
      return Ctx::Null();
    }

    if (args[1]->GetPrimitiveType() == Values::PrimitiveType::String) {
      string pattern;
      if (!Ctx::TryGetString(args[1], pattern)) {
        return Ctx::Null();
      }

      size_t pos = 0;
      while ((pos = str.find(pattern, pos)) != std::string::npos) {
        str.replace(pos, pattern.length(), "");
        pos += pattern.length();
      }
    }
    return Ctx::CreateString(str);
  }
  return Ctx::Null();
}

function(without) {
  if (args.empty() ||
      args[0]->GetPrimitiveType() != Values::PrimitiveType::String ||
      args[1]->GetPrimitiveType() != Values::PrimitiveType::String) {
    return Ctx::Null();
  }

  string target;
  if (!Ctx::TryGetString(args[0], target)) {
    return Ctx::Null();
  }

  string pattern;
  if (!Ctx::TryGetString(args[1], pattern)) {
    return Ctx::Null();
  }

  size_t pos = 0;
  while ((pos = target.find(pattern, pos)) != std::string::npos) {
    target.replace(pos, pattern.length(), "");
    pos += pattern.length();
  }

  return Ctx::CreateString(target);
}

extern "C" ScritModDef *InitScritModule_std_SR_string() {
  ScritModDef *def = CreateModDef();
  *def->description = "your description here";
  auto type = make_shared<StringType>();
  
  type->Set("isalpha", CREATE_CALLABLE(isalpha, "bool", {"string"}));
  type->Set("ispunct", CREATE_CALLABLE(ispunct, "bool", {"string"}));
  type->Set("isdigit", CREATE_CALLABLE(isdigit, "bool", {"string"}));
  type->Set("isalnum", CREATE_CALLABLE(isalnum, "bool", {"string"}));
  type->Set("split", CREATE_CALLABLE(split, "array", {"string", "string"}));
  type->Set("substring",
            CREATE_CALLABLE(substring, "string", {"string", "int", "int"}));
  type->Set("indexOf", CREATE_CALLABLE(indexOf, "int", {"string", "string"}));
  type->Set("front", CREATE_CALLABLE(front, "string", {"string"}));
  type->Set("back", CREATE_CALLABLE(back, "string", {"string"}));
  type->Set("pop", CREATE_CALLABLE(pop, "string", {"string"}));
  type->Set("push", CREATE_CALLABLE(push, "null", {"string", "string"}));
  type->Set("len", CREATE_CALLABLE(len, "int", {"string"}));
  type->Set("insert",
            CREATE_CALLABLE(insert, "string", {"string", "int", "string"}));
  type->Set("contains",
            CREATE_CALLABLE(contains, "bool", {"string", "string"}));
  type->Set("replace",
            CREATE_CALLABLE(replace, "string", {"string", "string", "string"}));
  type->Set("remove", CREATE_CALLABLE(remove, "string", {"string", "string"}));
  type->Set("without",
            CREATE_CALLABLE(without, "string", {"string", "string"}));
  def->AddType("string", type);
  
  def->AddFunction("isalpha", CREATE_FUNCTION(isalpha, "bool", {"string"}));
  def->AddFunction("ispunct", CREATE_FUNCTION(ispunct, "bool", {"string"}));
  def->AddFunction("isdigit", CREATE_FUNCTION(isdigit, "bool", {"string"}));
  def->AddFunction("isalnum", CREATE_FUNCTION(isalnum, "bool", {"string"}));
  def->AddFunction("split",
                   CREATE_FUNCTION(split, "array", {"string", "string"}));
  def->AddFunction("substring", CREATE_FUNCTION(substring, "string",
                                                {"string", "int", "int"}));
  def->AddFunction("indexOf",
                   CREATE_FUNCTION(indexOf, "int", {"string", "string"}));
  def->AddFunction("front", CREATE_FUNCTION(front, "string", {"string"}));
  def->AddFunction("back", CREATE_FUNCTION(back, "string", {"string"}));
  def->AddFunction("pop", CREATE_FUNCTION(pop, "string", {"string"}));
  def->AddFunction("push",
                   CREATE_FUNCTION(push, "null", {"string", "string"}));
  def->AddFunction("len", CREATE_FUNCTION(len, "int", {"string"}));
  def->AddFunction(
      "insert", CREATE_FUNCTION(insert, "string", {"string", "int", "string"}));
  def->AddFunction("contains",
                   CREATE_FUNCTION(contains, "bool", {"string", "string"}));
  def->AddFunction("replace", CREATE_FUNCTION(replace, "string",
                                              {"string", "string", "string"}));
  def->AddFunction("remove",
                   CREATE_FUNCTION(remove, "string", {"string", "string"}));
  def->AddFunction("without",
                   CREATE_FUNCTION(without, "string", {"string", "string"}));

  def->SetNamespace("std::string");
  return def;
}