#ifndef _GPC_ANALYZER_
#define _GPC_ANALYZER_

#include <algorithm>
#include <analyzer_base.hpp>
#include <array>
#include <nodes.hpp>
#include <string>
#include <symboltable.hpp>
#include <unordered_map>
#include <unordered_set>
#include <utils.hpp>
#include <vector>

namespace masm {
class GPCAnalyzer : public Analyzer {
  std::vector<Node> nodes;
  std::unordered_map<std::string, std::pair<value_t, std::string>> &consts;
  std::unordered_set<std::string> &labels;
  SymbolTable &symtable;

  // Result of analysis
  std::vector<Node> result;

public:
  GPCAnalyzer(
      std::unordered_map<std::string, std::pair<value_t, std::string>> &c,
      std::unordered_set<std::string> &l, SymbolTable &s);

  // bool analyze();
  void set_nodes(std::vector<Node> &&nodes) override;

  std::vector<Node> get_result() override;

  bool first_loop() override;

  bool first_loop_second_phase() override;

  bool second_loop() override;

  bool validate_defined_variables(Node &n);

  bool validate_reserved_variables(Node &n);

  std::pair<bool, std::pair<value_t, std::string>>
  resolve_if_constant(std::string name, std::vector<value_t> expected);

  bool resolve_variable(std::string name, std::vector<data_t> expected);

  bool analyze_stack_based_instructions(Node &n, data_t expected,
                                        std::vector<value_t> vtlist);

  bool analyze_load_store_instructions(Node &n, data_t expected);

  bool analyze_instructions_with_only_const_imm(Node &n,
                                                std::vector<value_t> expected);
};
}; // namespace masm

#endif
