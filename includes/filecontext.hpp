#ifndef _FILE_CONTEXT_
#define _FILE_CONTEXT_

#include <analyzer_base.hpp>
#include <consts.hpp>
#include <filesystem>
#include <gen_base.hpp>
#include <gpc_analyzer.hpp>
#include <gpc_gen.hpp>
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
  std::unordered_map<std::string, std::pair<value_t, std::string>> &CONSTANTS;
  std::unordered_set<std::string> &LABELS;
  SymbolTable &symtable;
  std::unordered_map<std::string, uint64_t> &label_addresses;
  std::unordered_map<std::string, uint64_t> &data_addresses;
  std::vector<std::filesystem::path> &include_paths;
  std::vector<uint8_t> &data, &string;

  std::unordered_set<std::filesystem::path> imports;

  file_t type;
  std::vector<Node> nodes, tmp;

  std::filesystem::path wp;

  uint64_t d_addr;

  std::unique_ptr<Analyzer> analyzer;
  std::unique_ptr<Gen> gen;

public:
  FileContext(
      std::vector<std::filesystem::path> &i_paths,
      std::unordered_map<std::string, std::pair<value_t, std::string>> &C,
      std::unordered_set<std::string> &L, SymbolTable &sym,
      std::unordered_map<std::string, uint64_t> &laddr,
      std::unordered_map<std::string, uint64_t> &daddr, std::vector<uint8_t> &D,
      std::vector<uint8_t> &S, uint64_t d_addr);

  /*File related functions*/
  bool is_file_a_directory(std::filesystem::path path);

  bool file_exists(std::filesystem::path path);

  bool deduce_file_type(std::filesystem::path path);

  std::pair<bool, size_t> find_the_parent_directory(std::filesystem::path path);

  bool file_already_imported(std::filesystem::path path);

  std::unordered_set<std::filesystem::path> get_imports();

  std::vector<Node> get_nodes();

  std::vector<Inst64> get_instructions();

  std::vector<uint8_t> get_data();

  uint64_t get_d_addr();

  file_t get_file_type();

  void set_imports(std::unordered_set<std::filesystem::path> &&f);

  void set_nodes(std::vector<Node> &&n);

  /*Processing functions*/
  bool file_prepare(std::string input_file);

  bool child_file_type_valid(file_t parent);

  bool should_process_file();

  bool parse_file();

  bool pre_analysis();

  bool analyze_file_first_step();

  bool analyze_file_first_step_second_phase();

  bool analyze_file_second_step();

  bool gen_file_first_step(uint64_t addr);

  bool gen_file_first_step_second_phase(uint64_t addr);

  bool gen_file_first_step_third_phase(uint64_t addr);

  bool gen_file_second_step();

  Inst64 get_ENTRY_INSTRUCTION(size_t addr);

  bool file_includes_another_file(Node &node);

  bool constant_definition(Node &node);
};
}; // namespace masm

#endif
