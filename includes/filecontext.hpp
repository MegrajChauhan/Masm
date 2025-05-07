#ifndef _FILE_CONTEXT_
#define _FILE_CONTEXT_

#include <consts.hpp>
#include <filesystem>
#include <gpc_analyzer.hpp>
#include <gpc_parser.hpp>
#include <iostream>
#include <lexer.hpp>
#include <memory>
#include <string>
#include <symboltable.hpp>
#include <unordered_map>
#include <unordered_set>
#include <utils.hpp>
#include <vector>

namespace masm {
class FileContext {
  std::unordered_map<std::string, std::pair<value_t, std::string>> CONSTANTS;
  std::unordered_set<std::string> LABELS;
  SymbolTable symtable;

  std::vector<std::filesystem::path> &include_paths;

  std::unordered_set<std::filesystem::path> imports;

  file_t type;
  std::vector<Node> nodes;

public:
  FileContext(std::vector<std::filesystem::path> &i_paths);

  /*File related functions*/
  bool is_file_a_directory(std::filesystem::path path);

  bool file_exists(std::filesystem::path path);

  bool deduce_file_type(std::filesystem::path path);

  std::pair<bool, size_t> find_the_parent_directory(std::filesystem::path path);

  bool file_already_imported(std::filesystem::path path);

  std::unordered_map<std::string, std::pair<value_t, std::string>>
  get_CONSTANTS();

  std::unordered_set<std::string> get_LABELS();

  std::unordered_set<std::filesystem::path> get_imports();

  SymbolTable get_symtable();

  std::vector<Node> get_nodes();

  void
  set_CONSTANTS(std::unordered_map<std::string, std::pair<value_t, std::string>>
                    &&constants);

  void set_LABELS(std::unordered_set<std::string> &&labels);

  void set_imports(std::unordered_set<std::filesystem::path> &&f);

  void set_symtable(SymbolTable &&table);

  void set_nodes(std::vector<Node> &&n);

  /*Processing functions*/
  bool file_process(std::string input_file);

  bool file_includes_another_file(Node &node);

  bool constant_definition(Node &node);

  // GPC
  bool file_process_GPC(std::filesystem::path working_path);
};
}; // namespace masm

#endif
