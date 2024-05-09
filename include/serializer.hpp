#include "native.hpp"
#include "parser.hpp"
#include "value.hpp"
#include <unordered_set>
#include <vector>

enum struct ReferenceHandling {
  Remove,
  Mark,
  Preserve
};

struct WriterSettings {
  int StartingIndentLevel = 0;
  int IndentSize = 0;
  ReferenceHandling ReferenceHandling = ReferenceHandling::Mark;
};

struct Writer {
  int indentLevel = 0;
  WriterSettings settings {};
  std::unordered_set<const Value_T *> foundObjs{};
  std::map<const Value_T *, int> references{};
  std::stringstream stream;
  void BuildMap(const Value_T *);
  void Map(const Value_T *array);
  void Write(const Value_T *array);
  static string ToString(const Value_T * value, WriterSettings settings);
};