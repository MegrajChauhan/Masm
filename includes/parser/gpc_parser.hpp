#ifndef _GPC_PARSER_
#define _GPC_PARSER_

#include <filesystem>
#include <lexer.hpp>
#include <memory>
#include <nodes.hpp>
#include <string>
#include <utils.hpp>
#include <vector>

namespace masm {
class GPCParser {
  std::vector<Node> nodes;
  std::filesystem::path file;

public:
  GPCParser(std::string path);

  bool parse();

  std::vector<Node> getNodes();

  value_t figure_out_type(token_t t);

  bool handle_simple_instructions(node_t type);

  bool handle_include_directory(Lexer &lexer);

  bool handle_const_definition(Lexer &lexer);

  bool handle_variable_defn(Lexer &lexer, Token name);

  bool handle_label(Token name);
  bool handle_dX(Lexer &lexer, Token name);
  bool handle_ds(Lexer &lexer, Token name);
  bool handle_df_dlf(Lexer &lexer, Token name);
  bool handle_resX(Lexer &lexer, Token name);

  bool handle_instructions_with_reg_reg_or_reg_imm(Lexer &lexer, Token inst,
                                                   node_t t);

  bool handle_instructions_with_reg(Lexer &lexer, node_t t);

  bool handle_instructions_with_reg_reg(Lexer &lexer, node_t type);

  bool handle_instructions_with_reg_imm(Lexer &lexer, node_t type);

  bool handle_instructions_with_imm(Lexer &lexer, node_t type);

  bool handle_instructions_with_imm_or_reg(Lexer &lexer, node_t type);
};
}; // namespace masm

#endif
