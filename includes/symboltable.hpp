#ifndef _SYMTABLE_
#define _SYMTABLE_

#include <string>
#include <unordered_map>

namespace masm {
enum data_t {
  BINARY,
  OCTAL,
  HEX,
  BYTE,
  WORD,
  DWORD,
  QWORD,
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
};

class SymbolTable {
  std::unordered_map<std::string, Symbol> table;

public:
  SymbolTable() = default;

  void add_symbol(std::string name, Symbol &&value);

  bool symbol_exists(std::string name);

  std::unordered_map<std::string, Symbol>::iterator
  find_symbol(std::string name);
};
}; // namespace masm

#endif
