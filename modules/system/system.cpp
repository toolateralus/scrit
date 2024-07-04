#include <scrit/ast.hpp>
#include <scrit/native.hpp>
#include <scrit/scritmod.hpp>
#include <scrit/ctx.hpp>
#include <scrit/native.hpp>
#include <scrit/value.hpp>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>

static Value fexists(std::vector<Value> values) {
  if (values.size() == 0) {
    return Ctx::Undefined();
  }

  auto filename = values[0];
  if (filename->GetPrimitiveType() != PrimitiveType::String) {
    return Value_T::UNDEFINED;
  }
  auto fname = static_cast<String_T *>(filename.get());

  if (fname == nullptr) {
    return Ctx::Undefined();
  }

  return Ctx::CreateBool(std::filesystem::exists(fname->value));
}
static Value fcreate(std::vector<Value> values) {
  if (values.size() == 0) {
    return Ctx::Undefined();
  }
  auto filename = values[0];
  if (filename->GetPrimitiveType() != PrimitiveType::String) {
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

  if (values.size() < 2) {
    return Ctx::Undefined();
  }
  auto filename = values[0];
  auto content = values[1];

  if (filename->GetPrimitiveType() != PrimitiveType::String) {
    return Ctx::CreateString(
        "invalid arguments to fwrite, filename must be a string.");
  }
  auto fname = static_cast<String_T *>(filename.get());
  auto fcontents = content->ToString();

  std::ofstream file(fname->value);
  if (file.is_open()) {
    file << fcontents;
    file.close();
  } else {
    return Ctx::CreateString("Unable to open file");
  }

  return Value_T::UNDEFINED;
}
static Value fread(std::vector<Value> values) {
  if (values.size() == 0) {
    return Ctx::Undefined();
  }
  auto filename = values[0];
  if (filename->GetPrimitiveType() != PrimitiveType::String) {
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
  if (values.size() == 0) {
    return Ctx::Undefined();
  }
  auto filename = values[0];
  if (filename->GetPrimitiveType() != PrimitiveType::String) {
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
  if (values.size() == 0) {
    return Ctx::Undefined();
  }
  auto dirname = values[0];
  if (dirname->GetPrimitiveType() != PrimitiveType::String) {
    return Value_T::UNDEFINED;
  }
  auto dname = static_cast<String_T *>(dirname.get());
  return Ctx::CreateBool(std::filesystem::exists(dname->value));
}
static Value dir_create(std::vector<Value> values) {
  if (values.size() == 0) {
    return Ctx::Undefined();
  }
  auto dirname = values[0];
  if (dirname->GetPrimitiveType() != PrimitiveType::String) {
    return Value_T::UNDEFINED;
  }
  auto dname = static_cast<String_T *>(dirname.get());

  if (std::filesystem::create_directory(dname->value)) {
    return Ctx::CreateString();
  } else {
    return Ctx::CreateString("Unable to create directory");
  }
}
static Value cwd(std::vector<Value>) {
  std::string currentDir = std::filesystem::current_path().string();
  return Ctx::CreateString(currentDir);
}
static Value dir_getfiles(std::vector<Value> values) {
  if (values.size() == 0) {
    return Ctx::Undefined();
  }
  auto dirname = values[0];
  if (dirname->GetPrimitiveType() != PrimitiveType::String) {
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
  if (values.size() == 0) {
    return Ctx::Undefined();
  }
  auto dirname = values[0];
  if (dirname->GetPrimitiveType() != PrimitiveType::String) {
    return Value_T::UNDEFINED;
  }
  auto dname = static_cast<String_T *>(dirname.get());

  if (std::filesystem::remove_all(dname->value)) {
    return Value_T::UNDEFINED;
  } else {
    return Ctx::CreateString("Unable to delete directory");
  }
}

static Value time(std::vector<Value>) {
  auto now = std::chrono::high_resolution_clock::now();
  long milliseconds =
      (double)std::chrono::duration_cast<std::chrono::milliseconds>(
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
      std::this_thread::sleep_for(
          std::chrono::milliseconds(static_cast<int>(milliseconds)));
    } else {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  return Value_T::UNDEFINED;
}

static Value syscall(std::vector<Value> values) {
  if (values.size() == 0) {
    return Ctx::Undefined();
  }
  auto cmd = values[0];
  if (cmd->GetPrimitiveType() != PrimitiveType::String) {
    return Value_T::UNDEFINED;
  }
  auto command = static_cast<String_T *>(cmd.get());
  return Ctx::CreateInt(system(command->value.c_str()));
}
static Value exit(std::vector<Value>) {
  std::exit(0);
  return Value_T::UNDEFINED;
}

static void file(ScritModDef *mod) {
  mod->AddFunction("file_read", CREATE_FUNCTION(fread, "string", {"string"}));
  mod->AddFunction("file_write",
                   CREATE_FUNCTION(fwrite, "undefined", {"string", "string"}));
  mod->AddFunction("file_create",
                   CREATE_FUNCTION(fcreate, "undefined", {"string"}));
  mod->AddFunction("file_exists", CREATE_FUNCTION(fexists, "bool", {"string"}));
  mod->AddFunction("file_delete",
                   CREATE_FUNCTION(fdelete, "undefined", {"string"}));
}

static void directory(ScritModDef *mod) {
  mod->AddFunction("dir_exists",
                   CREATE_FUNCTION(dir_exists, "bool", {"string"}));
  mod->AddFunction("dir_create",
                   CREATE_FUNCTION(dir_create, "undefined", {"string"}));
  mod->AddFunction("dir_delete",
                   CREATE_FUNCTION(dir_delete, "undefined", {"string"}));
  mod->AddFunction("dir_files",
                   CREATE_FUNCTION(dir_getfiles, "array", {"string"}));
  mod->AddFunction("dir_current", CREATE_FUNCTION(cwd, "string", {}));
}

static void general(ScritModDef *def) {
  auto time = CREATE_FUNCTION(::time, "float", {});
  auto syscall = CREATE_FUNCTION(::syscall, "int", {"string"});
  auto exit = CREATE_FUNCTION(::exit, "undefined", {"int"});
  auto sleep = CREATE_FUNCTION(::sleep, "undefined", {"float"});
  def->AddFunction("time", time);
  def->AddFunction("syscall", syscall);
  def->AddFunction("exit", exit);
  def->AddFunction("sleep", sleep);
}

extern "C" ScritModDef *InitScritModule_system() {
  ScritModDef *def = CreateModDef();
  *def->description = "system functions. file io, time, etc.";
  directory(def);
  file(def);
  general(def);
  return def;
}
