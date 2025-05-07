#include <gpc_parser.hpp>

masm::GPCParser::GPCParser(std::string path) { file = path; }

bool masm::GPCParser::parse() {
  bool lex_res = true;
  Lexer lexer(file.string(), &lex_res);

  if (!lex_res)
    return false;

  Token curr = lexer.next_token();
  while (curr.type != TOKEN_EOF) {
    switch (curr.type) {
    case TOKEN_ERROR:
      return false;
    case TOKEN_INCLUDE:
      if (!handle_include_directory(lexer))
        return false;
      break;
    case TOKEN_DEFINE:
      if (!handle_const_definition(lexer))
        return false;
      break;
    case TOKEN_IDENTIFIER:
      if (!handle_variable_defn(lexer, curr))
        return false;
      break;
    case TOKEN_NOP:
      handle_simple_instructions(NODE_NOP);
      break;
    case TOKEN_HALT:
      handle_simple_instructions(NODE_HALT);
      break;
    case TOKEN_RET:
      handle_simple_instructions(NODE_RET);
      break;
    case TOKEN_RETNZ:
      handle_simple_instructions(NODE_RETNZ);
      break;
    case TOKEN_RETZ:
      handle_simple_instructions(NODE_RETZ);
      break;
    case TOKEN_RETNE:
      handle_simple_instructions(NODE_RETNE);
      break;
    case TOKEN_RETE:
      handle_simple_instructions(NODE_RETE);
      break;
    case TOKEN_RETNC:
      handle_simple_instructions(NODE_RETNC);
      break;
    case TOKEN_RETC:
      handle_simple_instructions(NODE_RETC);
      break;
    case TOKEN_RETNO:
      handle_simple_instructions(NODE_RETNO);
      break;
    case TOKEN_RETO:
      handle_simple_instructions(NODE_RETO);
      break;
    case TOKEN_RETN:
      handle_simple_instructions(NODE_RETN);
      break;
    case TOKEN_RETNG:
      handle_simple_instructions(NODE_RETNG);
      break;
    case TOKEN_RETG:
      handle_simple_instructions(NODE_RETG);
      break;
    case TOKEN_RETNS:
      handle_simple_instructions(NODE_RETNS);
      break;
    case TOKEN_RETS:
      handle_simple_instructions(NODE_RETS);
      break;
    case TOKEN_RETGE:
      handle_simple_instructions(NODE_RETGE);
      break;
    case TOKEN_RETSE:
      handle_simple_instructions(NODE_RETSE);
      break;
    case TOKEN_PUSHA:
      handle_simple_instructions(NODE_PUSHA);
      break;
    case TOKEN_POPA:
      handle_simple_instructions(NODE_POPA);
      break;
    case TOKEN_OUTR:
      handle_simple_instructions(NODE_OUTR);
      break;
    case TOKEN_UOUTR:
      handle_simple_instructions(NODE_UOUTR);
      break;
    case TOKEN_CFLAGS:
      handle_simple_instructions(NODE_CFLAGS);
      break;
    case TOKEN_RESET:
      handle_simple_instructions(NODE_RESET);
      break;
    case TOKEN_ADD:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_ADD_IMM))
        return false;
      break;
    case TOKEN_SUB:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_SUB_IMM))
        return false;
      break;
    case TOKEN_MUL:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_MUL_IMM))
        return false;
      break;
    case TOKEN_DIV:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_DIV_IMM))
        return false;
      break;
    case TOKEN_MOD:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_MOD_IMM))
        return false;
      break;
    case TOKEN_IADD:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_IADD_IMM))
        return false;
      break;
    case TOKEN_ISUB:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_ISUB_IMM))
        return false;
      break;
    case TOKEN_IMUL:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_IMUL_IMM))
        return false;
      break;
    case TOKEN_IDIV:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_IDIV_IMM))
        return false;
      break;
    case TOKEN_IMOD:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_IMOD_IMM))
        return false;
      break;
    case TOKEN_FADD:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_FADD_IMM))
        return false;
      break;
    case TOKEN_FSUB:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_FSUB_IMM))
        return false;
      break;
    case TOKEN_FMUL:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_FMUL_IMM))
        return false;
      break;
    case TOKEN_FDIV:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_FDIV_IMM))
        return false;
      break;
    case TOKEN_FADD32:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_FADD32_IMM))
        return false;
      break;
    case TOKEN_FSUB32:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_FSUB32_IMM))
        return false;
      break;
    case TOKEN_FMUL32:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_FMUL32_IMM))
        return false;
      break;
    case TOKEN_FDIV32:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_FDIV32_IMM))
        return false;
      break;
    case TOKEN_AND:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_AND_IMM))
        return false;
      break;
    case TOKEN_OR:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_OR_IMM))
        return false;
      break;
    case TOKEN_XOR:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_XOR_IMM))
        return false;
      break;
    case TOKEN_SHL:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_SHL_IMM))
        return false;
      break;
    case TOKEN_SHR:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_SHR_IMM))
        return false;
      break;
    case TOKEN_CMP:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_CMP_IMM))
        return false;
      break;
    case TOKEN_INC:
      if (!handle_instructions_with_reg(lexer, NODE_INC))
        return false;
      break;
    case TOKEN_DEC:
      if (!handle_instructions_with_reg(lexer, NODE_DEC))
        return false;
      break;
    case TOKEN_NOT:
      if (!handle_instructions_with_reg(lexer, NODE_NOT))
        return false;
      break;
    case TOKEN_MOV:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOV))
        return false;
      break;
    case TOKEN_MOVB:
      if (!handle_instructions_with_reg_reg(lexer, NODE_MOVB))
        return false;
      break;
    case TOKEN_MOVW:
      if (!handle_instructions_with_reg_reg(lexer, NODE_MOVW))
        return false;
      break;
    case TOKEN_MOVD:
      if (!handle_instructions_with_reg_reg(lexer, NODE_MOVD))
        return false;
      break;
    case TOKEN_MOVQ:
      if (!handle_instructions_with_reg_reg(lexer, NODE_MOVQ))
        return false;
      break;
    case TOKEN_MOVF:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVF))
        return false;
      break;
    case TOKEN_MOVF32:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVF32))
        return false;
      break;
    case TOKEN_MOVSXB:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_MOVSXB_IMM))
        return false;
      break;
    case TOKEN_MOVSXW:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_MOVSXW_IMM))
        return false;
      break;
    case TOKEN_MOVSXD:
      if (!handle_instructions_with_reg_reg_or_reg_imm(lexer, curr,
                                                       NODE_MOVSXD_IMM))
        return false;
      break;
    case TOKEN_EXCGB:
      if (!handle_instructions_with_reg_reg(lexer, NODE_EXCGB))
        return false;
      break;
    case TOKEN_EXCGW:
      if (!handle_instructions_with_reg_reg(lexer, NODE_EXCGW))
        return false;
      break;
    case TOKEN_EXCGD:
      if (!handle_instructions_with_reg_reg(lexer, NODE_EXCGD))
        return false;
      break;
    case TOKEN_EXCGQ:
      if (!handle_instructions_with_reg_reg(lexer, NODE_EXCGQ))
        return false;
      break;
    case TOKEN_MOVEB:
      if (!handle_instructions_with_reg_reg(lexer, NODE_MOVEB))
        return false;
      break;
    case TOKEN_MOVEW:
      if (!handle_instructions_with_reg_reg(lexer, NODE_MOVEW))
        return false;
      break;
    case TOKEN_MOVED:
      if (!handle_instructions_with_reg_reg(lexer, NODE_MOVED))
        return false;
      break;
    case TOKEN_MOVEQ:
      if (!handle_instructions_with_reg_reg(lexer, NODE_MOVEQ))
        return false;
      break;
    case TOKEN_MOVNZ:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVNZ))
        return false;
      break;
    case TOKEN_MOVZ:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVZ))
        return false;
      break;
    case TOKEN_MOVNE:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVNE))
        return false;
      break;
    case TOKEN_MOVE:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVE))
        return false;
      break;
    case TOKEN_MOVNC:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVNC))
        return false;
      break;
    case TOKEN_MOVC:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVC))
        return false;
      break;
    case TOKEN_MOVNO:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVNO))
        return false;
      break;
    case TOKEN_MOVO:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVO))
        return false;
      break;
    case TOKEN_MOVNN:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVNN))
        return false;
      break;
    case TOKEN_MOVN:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVN))
        return false;
      break;
    case TOKEN_MOVNG:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVNG))
        return false;
      break;
    case TOKEN_MOVG:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVG))
        return false;
      break;
    case TOKEN_MOVNS:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVNS))
        return false;
      break;
    case TOKEN_MOVS:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVS))
        return false;
      break;
    case TOKEN_MOVGE:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVGE))
        return false;
      break;
    case TOKEN_MOVSE:
      if (!handle_instructions_with_reg_imm(lexer, NODE_MOVSE))
        return false;
      break;
    case TOKEN_JNZ:
      if (!handle_instructions_with_imm(lexer, NODE_JNZ))
        return false;
      break;
    case TOKEN_JZ:
      if (!handle_instructions_with_imm(lexer, NODE_JZ))
        return false;
      break;
    case TOKEN_JNE:
      if (!handle_instructions_with_imm(lexer, NODE_JNE))
        return false;
      break;
    case TOKEN_JE:
      if (!handle_instructions_with_imm(lexer, NODE_JE))
        return false;
      break;
    case TOKEN_JNC:
      if (!handle_instructions_with_imm(lexer, NODE_JNC))
        return false;
      break;
    case TOKEN_JC:
      if (!handle_instructions_with_imm(lexer, NODE_JC))
        return false;
      break;
    case TOKEN_JNO:
      if (!handle_instructions_with_imm(lexer, NODE_JNO))
        return false;
      break;
    case TOKEN_JO:
      if (!handle_instructions_with_imm(lexer, NODE_JO))
        return false;
      break;
    case TOKEN_JNN:
      if (!handle_instructions_with_imm(lexer, NODE_JNN))
        return false;
      break;
    case TOKEN_JN:
      if (!handle_instructions_with_imm(lexer, NODE_JN))
        return false;
      break;
    case TOKEN_JNG:
      if (!handle_instructions_with_imm(lexer, NODE_JNG))
        return false;
      break;
    case TOKEN_JG:
      if (!handle_instructions_with_imm(lexer, NODE_JG))
        return false;
      break;
    case TOKEN_JNS:
      if (!handle_instructions_with_imm(lexer, NODE_JNS))
        return false;
      break;
    case TOKEN_JS:
      if (!handle_instructions_with_imm(lexer, NODE_JS))
        return false;
      break;
    case TOKEN_JGE:
      if (!handle_instructions_with_imm(lexer, NODE_JGE))
        return false;
      break;
    case TOKEN_JSE:
      if (!handle_instructions_with_imm(lexer, NODE_JSE))
        return false;
      break;
    case TOKEN_INT:
      if (!handle_instructions_with_imm(lexer, NODE_INT))
        return false;
      break;
    case TOKEN_JMP:
      if (!handle_instructions_with_imm_or_reg(lexer, NODE_JMP_IMM))
        return false;
      break;
    case TOKEN_CALL:
      if (!handle_instructions_with_imm_or_reg(lexer, NODE_CALL_IMM))
        return false;
      break;
    default:
      detailed_message(file.c_str(), curr.line,
                       "Cannot build a node from this.", NULL);
      return false;
    }
    curr = lexer.next_token();
  }

  return true;
}

std::vector<masm::Node> masm::GPCParser::getNodes() { return std::move(nodes); }

masm::value_t masm::GPCParser::figure_out_type(token_t t) {
  value_t type;

  switch (t) {
  case TOKEN_INTEGER:
    type = VALUE_INTEGER;
    break;
  case TOKEN_BINARY:
    type = VALUE_BINARY;
    break;
  case TOKEN_HEX:
    type = VALUE_HEX;
    break;
  case TOKEN_OCTAL:
    type = VALUE_OCTAL;
    break;
  case TOKEN_FLOAT:
    type = VALUE_FLOAT;
    break;
  case TOKEN_STRING:
    type = VALUE_STRING;
    break;
  case TOKEN_IDENTIFIER:
    type = VALUE_IDEN;
    break;
  default:
    type = VALUE_ERR;
  }
  return type;
}

bool masm::GPCParser::handle_simple_instructions(masm::node_t type) {
  Node n;
  n.type = type;
  nodes.push_back(std::move(n));
  return true;
}

bool masm::GPCParser::handle_include_directory(masm::Lexer &lexer) {
  // The last token was the "include" keyword
  Token path = lexer.next_token();
  if (path.type != TOKEN_STRING) {
    detailed_message(file.c_str(), path.line,
                     "Expected a include string here got something else.",
                     NULL);
    return false;
  }
  Node node;
  node.file = file;
  node.type = INCLUDE_DIR;
  node.node = std::make_unique<NodeIncDir>();
  ((NodeIncDir *)node.node.get())->path_included = path.value;
  nodes.push_back(std::move(node));
  return true;
}

bool masm::GPCParser::handle_const_definition(Lexer &lexer) {
  Token const_name = lexer.next_token(), const_value;

  if (const_name.type != TOKEN_IDENTIFIER) {
    detailed_message(
        file.c_str(), const_name.line,
        "Expected a constant name after define but got something else.", NULL);
    return false;
  }

  const_value = lexer.next_token();

  value_t type = figure_out_type(const_value.type);
  if (type == VALUE_ERR) {
    detailed_message(file.c_str(), const_name.line,
                     "Invalid value for constant definition.", NULL);
    return false;
  }

  Node node;
  node.file = file;
  node.type = CONST_DEF;
  node.node = std::make_unique<NodeConstDef>();
  NodeConstDef *n = (NodeConstDef *)node.node.get();
  n->const_name = const_name.value;
  n->const_value = const_value.value;
  n->type = type;
  nodes.push_back(std::move(node));
  return true;
}

bool masm::GPCParser::handle_variable_defn(Lexer &lexer, Token name) {
  Token colon = lexer.next_token(), type = lexer.peek_token();
  if (colon.type != TOKEN_COLON) {
    detailed_message(file.c_str(), colon.line,
                     "Expected ':' after identifier name..", NULL);
    return false;
  }

  switch (type.type) {
  case TOKEN_DB:
  case TOKEN_DW:
  case TOKEN_DD:
  case TOKEN_DQ:
    return handle_dX(lexer, name);
  case TOKEN_DS:
    return handle_ds(lexer, name);
  case TOKEN_DF:
  case TOKEN_DLF:
    return handle_df_dlf(lexer, name);
  case TOKEN_RESB:
  case TOKEN_RESW:
  case TOKEN_RESD:
  case TOKEN_RESQ:
  case TOKEN_RESF:
  case TOKEN_RESLF:
    return handle_resX(lexer, name);
  default:
    return handle_label(name);
  }
  return false;
}

bool masm::GPCParser::handle_label(Token name) {
  Node node;
  node.file = file;
  node.type = NODE_LABEL;
  node.node = std::make_unique<NodeLabel>();
  node.line = name.line;
  ((NodeLabel *)node.node.get())->name = name.value;
  nodes.push_back(std::move(node));
  return true;
}

bool masm::GPCParser::handle_dX(Lexer &lexer, Token name) {
  Token type = lexer.next_token();
  Token value = lexer.next_token();
  value_t t = figure_out_type(value.type);
  if (t == VALUE_ERR || t == VALUE_STRING || t == VALUE_FLOAT) {
    detailed_message(file.c_str(), value.line,
                     "Expected integer after variable type.", NULL);
    return false;
  }
  Node node;
  node.file = file;
  node.line = name.line;
  node.type = (type.type == TOKEN_DB)   ? NODE_DB
              : (type.type == TOKEN_DW) ? NODE_DW
              : (type.type == TOKEN_DD) ? NODE_DD
                                        : NODE_DQ;
  node.node = std::make_unique<NodeDB>();
  NodeDB *n = (NodeDB *)node.node.get();
  n->name = name.value;
  n->value = value.value;
  n->type = t;
  nodes.push_back(std::move(node));
  return true;
}

bool masm::GPCParser::handle_ds(Lexer &lexer, Token name) {
  lexer.next_token();
  Token value = lexer.next_token();
  value_t t = figure_out_type(value.type);
  if (t != VALUE_STRING && t != VALUE_IDEN) {
    detailed_message(file.c_str(), value.line,
                     "Expected string after string type.", NULL);
    return false;
  }
  Node node;
  node.file = file;
  node.line = name.line;
  node.type = NODE_DS;
  node.node = std::make_unique<NodeDS>();
  NodeDS *n = (NodeDS *)node.node.get();
  n->name = name.value;
  n->value = value.value;
  n->type = t;
  nodes.push_back(std::move(node));
  return true;
}

bool masm::GPCParser::handle_df_dlf(Lexer &lexer, Token name) {
  Token type = lexer.next_token();
  Token value = lexer.next_token();
  value_t t = figure_out_type(value.type);
  if (t != VALUE_FLOAT && t != VALUE_IDEN) {
    detailed_message(file.c_str(), value.line,
                     "Expected Floating point value after float type.", NULL);
    return false;
  }
  Node node;
  node.file = file;
  node.line = name.line;
  node.type = (type.type == TOKEN_DF) ? NODE_DF : NODE_DLF;
  node.node = std::make_unique<NodeDF>();
  NodeDF *n = (NodeDF *)node.node.get();
  n->name = name.value;
  n->value = value.value;
  n->type = t;
  nodes.push_back(std::move(node));
  return true;
}

bool masm::GPCParser::handle_resX(Lexer &lexer, Token name) {
  Token type = lexer.next_token();
  Token value = lexer.next_token();
  value_t t = figure_out_type(value.type);
  if (t == VALUE_FLOAT || t == VALUE_ERR || t == VALUE_STRING) {
    detailed_message(file.c_str(), value.line,
                     "Invalid reserve length provided.", NULL);
    return false;
  }
  Node node;
  node.file = file;
  node.line = name.line;
  node.type = (type.type == TOKEN_RESB)   ? NODE_RESB
              : (type.type == TOKEN_RESW) ? NODE_RESW
              : (type.type == TOKEN_RESD) ? NODE_RESD
              : (type.type == TOKEN_RESQ) ? NODE_RESQ
              : (type.type == TOKEN_RESF) ? NODE_RESF
                                          : NODE_RESLF;
  node.node = std::make_unique<NodeRESB>();
  NodeRESB *n = (NodeRESB *)node.node.get();
  n->name = name.value;
  n->value = value.value;
  n->type = t;
  nodes.push_back(std::move(node));
  return true;
}

bool masm::GPCParser::handle_instructions_with_reg_reg_or_reg_imm(Lexer &lexer,
                                                                  Token inst,
                                                                  node_t t) {
  node_t type = t;
  token_t first, second;
  std::string imm; // for immediate

  bool is_reg = false;

  // The first operand
  Token operand = lexer.next_token();
  if (!(operand.type >= R0 && operand.type <= ACC)) {
    detailed_message(file.c_str(), inst.line,
                     "GPC: Expected a register as first operand here.", NULL);
    return false;
  }

  first = operand.type;
  operand = lexer.next_token();

  value_t val_type = figure_out_type(operand.type);

  if (!(operand.type >= R0 && operand.type <= ACC)) {
    if (val_type == VALUE_FLOAT || val_type == VALUE_STRING ||
        val_type == VALUE_ERR) {
      detailed_message(file.c_str(), inst.line,
                       "GPC: Expected a register, immediate or a constant as "
                       "the second operand here.",
                       NULL);
      return false;
    } else {
      imm = operand.value;
    }
  } else {
    is_reg = true;
    second = operand.type;
    type = (node_t)(type + 1);
  }

  Node node;
  node.type = type;
  node.file = file;
  node.line = inst.line;
  if (is_reg) {
    node.node = std::make_unique<NodeRegReg>();
    NodeRegReg *n = (NodeRegReg *)node.node.get();
    n->r1 = first;
    n->r2 = second;
  } else {
    node.node = std::make_unique<NodeRegrImm>();
    NodeRegrImm *n = (NodeRegrImm *)node.node.get();
    n->immediate = imm;
    n->regr = first;
    n->type = val_type;
  }

  nodes.push_back(std::move(node));

  return true;
}

bool masm::GPCParser::handle_instructions_with_reg(Lexer &lexer, node_t t) {
  Token regr = lexer.next_token();
  if (!(regr.type >= R0 && regr.type <= ACC)) {
    detailed_message(file.c_str(), regr.line,
                     "GPC: Expected a register as operand here.", NULL);
    return false;
  }
  Node node;
  node.file = file;
  node.line = regr.line;
  node.type = t;
  node.node = std::make_unique<NodeReg>();
  NodeReg *n = (NodeReg *)node.node.get();
  n->reg = regr.type;
  nodes.push_back(std::move(node));
  return true;
}

bool masm::GPCParser::handle_instructions_with_reg_imm(Lexer &lexer,
                                                       node_t type) {
  Token reg1 = lexer.next_token();
  if (!(reg1.type >= R0 && reg1.type <= ACC)) {
    detailed_message(file.c_str(), reg1.line,
                     "GPC: Expected a register as first operand here.", NULL);
    return false;
  }

  Token oper2 = lexer.next_token();
  value_t res = figure_out_type(oper2.type);
  if (res == VALUE_ERR || res == VALUE_STRING) {
    detailed_message(file.c_str(), reg1.line,
                     "GPC: Expected an immediate as second operand here.",
                     NULL);
    return false;
  }
  Node node;
  node.file = file;
  node.line = reg1.line;
  node.type = type;
  node.node = std::make_unique<NodeRegrImm>();
  NodeRegrImm *n = (NodeRegrImm *)node.node.get();
  n->regr = reg1.type;
  n->immediate = oper2.value;
  n->type = res;
  nodes.push_back(std::move(node));
  return true;
}

bool masm::GPCParser::handle_instructions_with_imm(Lexer &lexer, node_t type) {
  Token imm = lexer.next_token();
  value_t val_type = figure_out_type(imm.type);
  if (val_type == VALUE_ERR || val_type == VALUE_STRING) {
    detailed_message(file.c_str(), imm.line,
                     "GPC: Expected an immediate as operand here.", NULL);
    return false;
  }
  Node node;
  node.file = file;
  node.line = imm.line;
  node.type = type;
  node.node = std::make_unique<NodeImm>();
  NodeImm *n = (NodeImm *)node.node.get();
  n->imm = imm.value;
  n->type = val_type;
  nodes.push_back(std::move(node));
  return true;
}

bool masm::GPCParser::handle_instructions_with_imm_or_reg(Lexer &lexer,
                                                          node_t type) {
  bool is_reg = false;
  Token oper = lexer.next_token();
  value_t val_type = figure_out_type(oper.type);
  if (oper.type >= R0 && oper.type <= ACC) {
    is_reg = true;
    type = (node_t)(type + 1);
  } else if (val_type == VALUE_ERR || val_type == VALUE_STRING) {
    detailed_message(file.c_str(), oper.line,
                     "GPC: Expected an immediate or register as operand here.",
                     NULL);
    return false;
  }
  Node node;
  node.file = file;
  node.line = oper.line;
  node.type = type;
  if (is_reg) {
    node.node = std::make_unique<NodeReg>();
    NodeReg *n = (NodeReg *)node.node.get();
    n->reg = oper.type;
  } else {
    node.node = std::make_unique<NodeImm>();
    NodeImm *n = (NodeImm *)node.node.get();
    n->imm = oper.value;
    n->type = val_type;
  }
  nodes.push_back(std::move(node));
  return true;
}
