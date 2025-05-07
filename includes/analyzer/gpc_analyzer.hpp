#ifndef _GPC_ANALYZER_
#define _GPC_ANALYZER_

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
};
}; // namespace masm

#endif
