#pragma once
#include "native.hpp"
#include "value.hpp"
#include <unordered_set>
#include <map>

enum struct ReferenceHandling {
  Remove,
  Mark,
  Preserve
};



struct Writer {
  
  struct Indenter {
    Writer *writer;
    Indenter(Writer *writer);
    ~Indenter();
  };
  struct Settings {
    int StartingIndentLevel = 0;
    int IndentSize = 0;
    ReferenceHandling ref_handling = ReferenceHandling::Mark;
  };  
  
  string indent = "";
  int indentLevel = 0;
  Settings settings {};
  std::unordered_set<const Value_T *> foundObjs{};
  std::map<const Value_T *, int> references{};
  std::stringstream stream;
  void BuildMap(const Value_T *);
  void Map(const Value_T *array);
  void HandleRefs(const string &element_delimter,
                 Value_T *&value, const string &key = "");
                 
  void WriteArray(const Array_T *val);
  void WriteObject(const Object_T *val);
  
  void Write(const Value_T *array);
  static string ToString(const Value_T * value, Settings settings);
};