#include "scrit/scritmod.hpp"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <memory>
#include <scrit/native.hpp>
#include <scrit/value.hpp>
#include <string>
#include <thread>
#include <unistd.h>
#include "scrit/ast.hpp"

static Value fexists(std::vector<Value> values) {
  auto filename = values[0];
  if (filename->GetType()!= ValueType::String) {
    return Value_T::UNDEFINED;
  }
  auto fname = static_cast<String_T *>(filename.get());
  return Ctx::CreateBool(std::filesystem::exists(fname->value));
}
static Value fcreate(std::vector<Value> values) {
  auto filename = values[0];
  if (filename->GetType()!= ValueType::String) {
    return Value_T::UNDEFINED;
  }
  auto fname = static_cast<String_T *>(filename.get());
  std::ofstream file(fname->value);
  if (file.is_open()) {
    file.close();
  } else {
    return Ctx::CreateString("Unable to open file");
  }

  return Value_T::UNDEFINED;
}
static Value fwrite(std::vector<Value> values) {
  auto filename = values[0];
  auto content = values[1];

  if (filename->GetType()!= ValueType::String || content->GetType()!= ValueType::String) {
    return Ctx::CreateString("invalid arguments to fwrite");
  }
  auto fname = static_cast<String_T *>(filename.get());
  auto fcontent = static_cast<String_T *>(content.get());

  std::ofstream file(fname->value);
  if (file.is_open()) {
    file << fcontent->value;
    file.close();
  } else {
    return Ctx::CreateString("Unable to open file");
  }

  return Value_T::UNDEFINED;
}
static Value fread(std::vector<Value> values) {
  auto filename = values[0];
  if (filename->GetType()!= ValueType::String) {
    return Value_T::UNDEFINED;
  }
  auto fname = static_cast<String_T *>(filename.get());

  std::ifstream file(fname->value);
  if (file.is_open()) {
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    return Ctx::CreateString(content);
  } else {
    return Ctx::CreateString("Unable to open file");
  }
}
static Value fdelete(std::vector<Value> values) {
  auto filename = values[0];
  if (filename->GetType()!= ValueType::String) {
    return Value_T::UNDEFINED;
  }
  auto fname = static_cast<String_T *>(filename.get());

  if (std::filesystem::remove(fname->value)) {
    return Value_T::UNDEFINED;
  } else {
    return Ctx::CreateString("Unable to delete file");
  }
}
static Value dir_exists(std::vector<Value> values) {
  auto dirname = values[0];
  if (dirname->GetType()!= ValueType::String) {
    return Value_T::UNDEFINED;
  }
  auto dname = static_cast<String_T *>(dirname.get());
  return Ctx::CreateBool(std::filesystem::exists(dname->value));
}
static Value dir_create(std::vector<Value> values) {
  auto dirname = values[0];
  if (dirname->GetType()!= ValueType::String) {
    return Value_T::UNDEFINED;
  }
  auto dname = static_cast<String_T *>(dirname.get());

  if (std::filesystem::create_directory(dname->value)) {
    return Value_T::UNDEFINED;
  } else {
    return Ctx::CreateString("Unable to create directory");
  }
}
static Value cwd(std::vector<Value> values) {
  std::string currentDir = std::filesystem::current_path().string();
  return Ctx::CreateString(currentDir);
}
static Value dir_getfiles(std::vector<Value> values) {
  auto dirname = values[0];
  if (dirname->GetType()!= ValueType::String) {
    return Value_T::UNDEFINED;
  }
  auto dname = static_cast<String_T *>(dirname.get());
  std::vector<Value> files;
  for (const auto &entry : std::filesystem::directory_iterator(dname->value)) {
    if (entry.is_regular_file()) {
      files.push_back(Ctx::CreateString(entry.path().filename().string()));
    }
  }
  return Array_T::New(files);
}
static Value dir_delete(std::vector<Value> values) {
  auto dirname = values[0];
  if (dirname->GetType()!= ValueType::String) {
    return Value_T::UNDEFINED;
  }
  auto dname = static_cast<String_T *>(dirname.get());

  if (std::filesystem::remove_all(dname->value)) {
    return Value_T::UNDEFINED;
  } else {
    return Ctx::CreateString("Unable to delete directory");
  }
}
static Value time(std::vector<Value> values) {
  auto now = std::chrono::high_resolution_clock::now();
  long milliseconds = (double)std::chrono::duration_cast<std::chrono::milliseconds>(
                         now.time_since_epoch())
                         .count();
  return Ctx::CreateFloat(milliseconds / 1000.0);
}
static Value sleep(std::vector<Value> args) {
  if (!args.empty()) {
    float milliseconds;
    int seconds = 1;
    if (Ctx::TryGetInt(args[0], seconds)) {
      std::this_thread::sleep_for(std::chrono::seconds(seconds));
    } else if (Ctx::TryGetFloat(args[0], milliseconds)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(milliseconds)));
    } else {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  return Value_T::UNDEFINED;
}

static Value syscall(std::vector<Value> values) {
  auto cmd = values[0];
  if (cmd->GetType()!= ValueType::String) {
    return Value_T::UNDEFINED;
  }
  auto command = static_cast<String_T *>(cmd.get());
  return Ctx::CreateInt(system(command->value.c_str()));
}
static Value exit(std::vector<Value> values) {
  std::exit(0);
  return Value_T::UNDEFINED;
}

extern "C" ScritModDef *InitScritModule_system() {
  ScritModDef *def = CreateModDef();
  *def->description = "system functions. file io, time, etc.";
  
  AddFunction(def, "fexists", &fexists, ValueType::String, {Argument(ValueType::String, "filename")});
  AddFunction(def, "fexists", &fexists, ValueType::String, {Argument(ValueType::String, "filename")});
  AddFunction(def, "fcreate", &fcreate, ValueType::String, {Argument(ValueType::String, "filename")});
  AddFunction(def, "fwrite", &fwrite, ValueType::String, {Argument(ValueType::String, "filename"), Argument(ValueType::String, "content")});
  AddFunction(def, "fread", &fread, ValueType::String, {Argument(ValueType::String, "filename")});
  AddFunction(def, "fdelete", &fdelete, ValueType::String, {Argument(ValueType::String, "filename")});
  AddFunction(def, "cwd", &cwd, ValueType::String, {});
  AddFunction(def, "dir_exists", &dir_exists, ValueType::String, {Argument(ValueType::String, "dirname")});
  AddFunction(def, "dir_create", &dir_create, ValueType::String, {Argument(ValueType::String, "dirname")});
  AddFunction(def, "dir_delete", &dir_delete, ValueType::String, {Argument(ValueType::String, "dirname")});
  AddFunction(def, "dir_getfiles", &dir_getfiles, ValueType::String, {Argument(ValueType::String, "dirname")});
  AddFunction(def, "time", &time, ValueType::String, {});
  AddFunction(def, "syscall", &syscall, ValueType::String, {Argument(ValueType::String, "cmd")});
  AddFunction(def, "exit", &exit, ValueType::String, {});
  AddFunction(def, "sleep", &sleep, ValueType::Undefined, {{ValueType::Any, "seconds(int)|ms(float)"}});
  
  return def;
}
