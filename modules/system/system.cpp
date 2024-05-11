#include "scrit/scritmod.hpp"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <memory>
#include <scrit/value.hpp>
#include <string>
#include <unistd.h>

static Value fexists(std::vector<Value> values) {
  auto filename = values[0];
  if (filename->GetType()!= ValueType::String) {
    return Value_T::Undefined;
  }
  auto fname = static_cast<String_T *>(filename.get());
  return Ctx::CreateBool(std::filesystem::exists(fname->value));
}
static Value fcreate(std::vector<Value> values) {
  auto filename = values[0];
  if (filename->GetType()!= ValueType::String) {
    return Value_T::Undefined;
  }
  auto fname = static_cast<String_T *>(filename.get());
  std::ofstream file(fname->value);
  if (file.is_open()) {
    file.close();
  } else {
    return Ctx::CreateString("Unable to open file");
  }

  return Value_T::Undefined;
}
static Value fwrite(std::vector<Value> values) {
  auto filename = values[0];
  auto content = values[1];

  if (filename->GetType()!= ValueType::String || content->GetType()!= ValueType::String) {
    return Ctx::CreateString("invalid arguments");
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

  return Value_T::Undefined;
}
static Value fread(std::vector<Value> values) {
  auto filename = values[0];
  if (filename->GetType()!= ValueType::String) {
    return Value_T::Undefined;
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
    return Value_T::Undefined;
  }
  auto fname = static_cast<String_T *>(filename.get());

  if (std::filesystem::remove(fname->value)) {
    return Value_T::Undefined;
  } else {
    return Ctx::CreateString("Unable to delete file");
  }
}
static Value dir_exists(std::vector<Value> values) {
  auto dirname = values[0];
  if (dirname->GetType()!= ValueType::String) {
    return Value_T::Undefined;
  }
  auto dname = static_cast<String_T *>(dirname.get());
  return Ctx::CreateBool(std::filesystem::exists(dname->value));
}
static Value dir_create(std::vector<Value> values) {
  auto dirname = values[0];
  if (dirname->GetType()!= ValueType::String) {
    return Value_T::Undefined;
  }
  auto dname = static_cast<String_T *>(dirname.get());

  if (std::filesystem::create_directory(dname->value)) {
    return Value_T::Undefined;
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
    return Value_T::Undefined;
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
    return Value_T::Undefined;
  }
  auto dname = static_cast<String_T *>(dirname.get());

  if (std::filesystem::remove_all(dname->value)) {
    return Value_T::Undefined;
  } else {
    return Ctx::CreateString("Unable to delete directory");
  }
}
static Value time(std::vector<Value> values) {
  auto now = std::chrono::high_resolution_clock::now();
  auto duration = now.time_since_epoch();
  auto milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  return Ctx::CreateFloat(milliseconds / 1000.0);
}
static Value sleep(std::vector<Value> args) {
  if (!args.empty()) {
    float fseconds;
    int seconds = 1;
    if (Ctx::TryGetInt(args[0], seconds)) {
      sleep(seconds);
    } else if (Ctx::TryGetFloat(args[0], fseconds)) {
      usleep(fseconds * 1'000'000);
    } else {
      sleep(1);
    }
  }
  return Value_T::Undefined;
}

static Value syscall(std::vector<Value> values) {
  auto cmd = values[0];
  if (cmd->GetType()!= ValueType::String) {
    return Value_T::Undefined;
  }
  auto command = static_cast<String_T *>(cmd.get());
  return Ctx::CreateInt(system(command->value.c_str()));
}
static Value exit(std::vector<Value> values) {
  std::exit(0);
  return Value_T::Undefined;
}
extern "C" ScritModDef *InitScritModule_system() {
  ScritModDef *def = CreateModDef();
  *def->description = "system functions. file io, time, etc.";
  
  AddFunction(def, "fexists", &fexists, ValueType::String, CreateArgumentSignature({Argument(ValueType::String, "filename")}));
  AddFunction(def, "fexists", &fexists, ValueType::String, CreateArgumentSignature({Argument(ValueType::String, "filename")}));
  AddFunction(def, "fcreate", &fcreate, ValueType::String, CreateArgumentSignature({Argument(ValueType::String, "filename")}));
  AddFunction(def, "fwrite", &fwrite, ValueType::String, CreateArgumentSignature({Argument(ValueType::String, "filename"), Argument(ValueType::String, "content")}));
  AddFunction(def, "fread", &fread, ValueType::String, CreateArgumentSignature({Argument(ValueType::String, "filename")}));
  AddFunction(def, "fdelete", &fdelete, ValueType::String, CreateArgumentSignature({Argument(ValueType::String, "filename")}));
  AddFunction(def, "cwd", &cwd, ValueType::String, CreateArgumentSignature({}));
  AddFunction(def, "dir_exists", &dir_exists, ValueType::String, CreateArgumentSignature({Argument(ValueType::String, "dirname")}));
  AddFunction(def, "dir_create", &dir_create, ValueType::String, CreateArgumentSignature({Argument(ValueType::String, "dirname")}));
  AddFunction(def, "dir_delete", &dir_delete, ValueType::String, CreateArgumentSignature({Argument(ValueType::String, "dirname")}));
  AddFunction(def, "dir_getfiles", &dir_getfiles, ValueType::String, CreateArgumentSignature({Argument(ValueType::String, "dirname")}));
  AddFunction(def, "time", &time, ValueType::String, CreateArgumentSignature({}));
  AddFunction(def, "syscall", &syscall, ValueType::String, CreateArgumentSignature({Argument(ValueType::String, "cmd")}));
  AddFunction(def, "exit", &exit, ValueType::String, CreateArgumentSignature({}));

  return def;
}
