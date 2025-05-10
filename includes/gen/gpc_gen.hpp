#ifndef _GPC_GEN_
#define _GPC_GEN_

#include <gen_base.hpp>
#include <gpc_gen_base.hpp>
#include <nodes.hpp>
#include <symboltable.hpp>
#include <unordered_map>
#include <vector>

namespace masm {
typedef Inst64 Data64;
class GPCGen {
  std::vector<Node> final_nodes; // All we will convert to instructions
  SymbolTable symtable;          // All the data
  uint64_t st_address_data;

  std::unordered_map<std::string, uint64_t> label_addresses;
  std::unordered_map<std::string, uint64_t> data_addresses;

  // Results
  std::vector<Inst64> instructions;
  std::vector<uint8_t> data;

public:
  GPCGen(std::vector<Node> &, SymbolTable &, uint64_t);

  uint64_t get_current_address_point();

  std::vector<Inst64> get_instructions();

  std::vector<uint8_t> get_data();

  bool generate();

  bool first_iteration();

  bool second_iteration();

  void add_data(std::string value, value_t type, size_t len);

  void add_reserved_data(std::string len, value_t type, size_t l);

  void simple_instructions(uint8_t opcode);

  uint8_t token_to_regr(token_t regr);

  void instructions_with_single_regr(uint8_t opcode, uint8_t regr);

  void instructions_with_two_regr(uint8_t opcode, uint8_t reg1, uint8_t reg2);
};
}; // namespace masm

#endif
