#ifndef _GPC_ANALYZER_
#define _GPC_ANALYZER_

#include <algorithm>
#include <array>
#include <nodes.hpp>
#include <string>
#include <symboltable.hpp>
#include <unordered_map>
#include <unordered_set>
#include <utils.hpp>
#include <vector>

namespace masm {
class GPCAnalyzer {
  std::vector<Node> nodes;
  std::unordered_map<std::string, std::pair<value_t, std::string>> &consts;
  std::unordered_set<std::string> &labels;
  SymbolTable &symtable;

  // Result of analysis
  std::vector<Node> result;

public:
  GPCAnalyzer(
      std::vector<Node> &&n,
      std::unordered_map<std::string, std::pair<value_t, std::string>> &c,
      std::unordered_set<std::string> &l, SymbolTable &s);

  bool analyze();

  std::vector<Node> get_result();

  bool first_loop();

  bool second_loop();

  bool validate_defined_variables(Node &n);

  bool validate_reserved_variables(Node &n);

  std::pair<bool, std::pair<value_t, std::string>>
  resolve_if_constant(std::string name, std::array<value_t, 4> expected);

  bool resolve_variable(std::string name, std::array<data_t, 4> expected);

  bool analyze_stack_based_instructions(Node &n, data_t expected,
                                        std::array<value_t, 4> vtlist);

  bool analyze_load_store_instructions(Node &n, data_t expected);

  bool
  analyze_instructions_with_only_const_imm(Node &n,
                                           std::array<value_t, 4> expected);
};
}; // namespace masm

#endif
