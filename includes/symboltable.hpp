#ifndef _SYMTABLE_
#define _SYMTABLE_

#include <string>
#include <unordered_map>
#include <utils.hpp>

namespace masm {
enum value_t {
  VALUE_ERR,
  VALUE_BINARY,
  VALUE_OCTAL,
  VALUE_HEX,
  VALUE_INTEGER,
  VALUE_FLOAT,
  VALUE_STRING,
  VALUE_IDEN
};

enum data_t {
  BYTE,
  WORD,
  DWORD,
  QWORD,
  POINTER,
  FLOAT,
  STRING,
  RESB,
  RESW,
  RESD,
  RESQ
};

struct Symbol {
  data_t type;
  std::string value;
  value_t val_type;
};

class SymbolTable {
  std::unordered_map<std::string, Symbol> table;

public:
  SymbolTable() = default;

  void add_symbol(std::string name, Symbol value);

  bool symbol_exists(std::string name);

  std::unordered_map<std::string, Symbol>::iterator
  find_symbol(std::string name);

  void list_symbols(); // debugging function
};
}; // namespace masm

#endif
