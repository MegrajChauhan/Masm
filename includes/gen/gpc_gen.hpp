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
  SymbolTable &symtable;         // All the data
  uint64_t st_address_data;

  std::unordered_map<std::string, uint64_t> &label_addresses;
  std::unordered_map<std::string, uint64_t> &data_addresses;

  // Results
  std::vector<Inst64> instructions;
  std::vector<uint8_t> &data, &string;

public:
  GPCGen(SymbolTable &, std::unordered_map<std::string, uint64_t> &,
         std::unordered_map<std::string, uint64_t> &, std::vector<uint8_t> &,
         std::vector<uint8_t> &, uint64_t);

  void set_final_nodes(std::vector<Node> &&nodes);

  uint64_t get_current_address_point();

  std::vector<Inst64> get_instructions();

  std::vector<uint8_t> get_data();

  bool generate();

  bool first_iteration();

  bool first_iteration_second_phase(uint64_t addr_point);

  bool first_iteration_third_phase(uint64_t addr_point);

  bool second_iteration();

  void add_data(std::string value, value_t type, size_t len);

  void add_reserved_data(std::string len, value_t type, size_t l);

  void simple_instructions(uint8_t opcode);

  uint8_t token_to_regr(token_t regr);

  void instructions_with_single_regr(uint8_t opcode, uint8_t regr);

  void instructions_with_two_regr(uint8_t opcode, uint8_t reg1, uint8_t reg2);

  void instructions_with_one_immediate(uint8_t opcode, Node &n, size_t len,
                                       bool label = false, uint8_t op2 = 0);

  void sin_and_sout_instructions(Node &n);

  void single_operand_which_is_variable(uint8_t opcode, std::string name);

  void single_operand_which_is_immediate(uint8_t opcode, std::string value,
                                         value_t type, size_t len);

  void choose_opcode_according_to_variable(NodeRegrImm *n,
                                           std::vector<uint8_t> opcodes);

  void two_operand_second_is_immediate(uint8_t opcode, std::string value,
                                       value_t type, size_t len, token_t reg1);

  void two_operand_second_is_immediate_in_same_qword(uint8_t opcode,
                                                     token_t regr,
                                                     std::string imm,
                                                     value_t type, size_t len);
};
}; // namespace masm

#endif
