#include <iostream>
#include <symboltable.hpp>

void masm::SymbolTable::add_symbol(std::string name, Symbol value) {
  table[name] = value;
}

bool masm::SymbolTable::symbol_exists(std::string name) {
  return find_symbol(name) != table.end();
}

std::unordered_map<std::string, masm::Symbol>::iterator
masm::SymbolTable::find_symbol(std::string name) {
  return table.find(name);
}

void masm::SymbolTable::list_symbols() {
  for (auto sym : table) {
    std::cout << "NAME: " << sym.first << '\n';
    std::cout << "VALUE: " << sym.second.value << '\n';
    std::cout << "TYPE: " << sym.second.type << "\n\n";
  }
}
