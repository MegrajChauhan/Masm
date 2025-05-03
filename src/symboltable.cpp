#include <symboltable.hpp>

void masm::SymbolTable::add_symbol(std::string name, Symbol &&value)
{
   table[name] = value;
}

bool masm::SymbolTable::symbol_exists(std::string name)
{
  return find_symbol(name) != table.end();
}

std::unordered_map<std::string, masm::Symbol>::iterator masm::SymbolTable::find_symbol(std::string name)
{
   return table.find(name);
}
