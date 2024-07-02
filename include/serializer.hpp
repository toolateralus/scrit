#pragma once
#include "error.hpp"
#include "lexer.hpp"
#include "native.hpp"
#include "parser.hpp"
#include "type.hpp"
#include "value.hpp"
#include <set>
#include <stdexcept>
#include <unordered_set>
#include <map>

enum struct ReferenceHandling {
  Remove,
  Mark,
  Preserve
};


struct Reader {
  vector<Token> tokens;
  Parser parser;
  
  Reader() = delete;
  Reader(const string &input) {
    auto lexer = Lexer();
    auto tokens = lexer.Lex(input);
    std::reverse(tokens.begin(), tokens.end());
    parser = Parser(tokens);
  }
  
  Value Read() {
    auto next = parser.Peek();
    
    switch(next.type) {
      case TType::LCurly:
        return ReadObject();
      case TType::SubscriptLeft:
        return ReadArray();
      default:
        if (next.family == TFamily::Literal) {
          return parser.ParseExpression()->Evaluate();
        }
        throw std::runtime_error("Deserialization error: invalid token:\n\t'" + TTypeToString(next.type) + "'");
    }
    
  }
  
  Value ReadArray() {
    parser.Expect(TType::SubscriptLeft);
    vector<Value> values;
    while (!tokens.empty()) {
      if (parser.Peek().type == TType::LCurly) {
        values.push_back(ReadObject());
        continue;
      }
      values.push_back(parser.ParseExpression()->Evaluate());
    }
    return Ctx::CreateArray(values);
  }
  
  Value ReadObject() {
    auto object = Ctx::CreateObject();
    parser.Expect(TType::LCurly);
    while (!parser.tokens.empty()) {
      if (parser.Peek().type == TType::RCurly) {
        break;
      }
      auto key = parser.Expect(TType::String);            
      parser.Expect(TType::Colon);
      auto value = parser.ParseExpression();
      object->SetMember(key.value, value->Evaluate());
      if (parser.Peek().type == TType::Comma) {
        parser.Eat();
      }
    }
    parser.Expect(TType::RCurly);
    return object;
  }
  
};


static REGISTER_FUNCTION(deserialize, "any", {"string"}) {
  Reader reader(args[0]->ToString());  
  return reader.Read();
}

struct Writer {
  
  struct Indenter {
    Writer *writer;
    Indenter(Writer *writer);
    ~Indenter();
  };
  struct Settings {
    static Settings Default() {
      return {};
    }
    int StartingIndentLevel = 0;
    int IndentSize = 0;
    ReferenceHandling ref_handling = ReferenceHandling::Mark;
  };  
  
  Writer() = delete;
  Writer(Settings settings = Settings::Default()) : settings(settings) {
    if (settings.IndentSize > 0) {
      newline = "\n";
    }
  }
  
  string newline = "";
  string indent = "";
  int indentLevel = 0;
  Settings settings {};
  std::unordered_set<const Value_T *> foundObjs{};
  std::map<const Value_T *, int> references{};
  std::stringstream stream;
  void BuildMap(const Value_T *);
  void Map(const Value_T *array);
  bool HandleRefs(const string &element_delimter,
                 Value_T *&value, const string &key = "");
                 
  void WriteArray(const Array_T *val);
  void WriteObject(const Object_T *val);
  
  void Write(const Value_T *array);
  static string ToString(const Value_T * value, Settings settings);
};