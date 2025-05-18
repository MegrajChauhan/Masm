#ifndef _MASM_CONTEXT_
#define _MASM_CONTEXT_

#include <filecontext.hpp>

// This is also responsible for parsing the input CMD arguments
namespace masm {
class MasmContext {
  uint64_t d_address = 0;

  std::unordered_map<std::string, std::pair<value_t, std::string>> CONSTANTS;
  std::unordered_set<std::string> LABELS;
  SymbolTable symtable;
  std::unordered_map<std::string, uint64_t> label_addresses;
  std::unordered_map<std::string, uint64_t> data_addresses;

  std::vector<std::filesystem::path> include_paths;

  std::vector<FileContext> contexts;
  std::vector<std::string> input_files;

  std::string output_file = "./M.mbin";

  int argc;
  char **argv;

public:
  MasmContext(int, char **);
};
}; // namespace masm

#endif
