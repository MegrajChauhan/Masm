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

masm::data_t masm::GPCParser::figure_out_type(token_t t) {
  data_t type;

  switch (t) {
  case TOKEN_INTEGER:
    type = QWORD;
    break;
  case TOKEN_BINARY:
    type = BINARY;
    break;
  case TOKEN_HEX:
    type = HEX;
    break;
  case TOKEN_OCTAL:
    type = OCTAL;
    break;
  case TOKEN_FLOAT:
    type = FLOAT;
    break;
  case TOKEN_STRING:
    type = STRING;
    break;
  default:
    type = data_t::RESB;
  }
  return type;
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

  data_t type = figure_out_type(const_value.type);
  if (type == RESB) {
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
  printf("NAME: %s\nNXT: %d\n", name.value.c_str(), colon.type);
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
  data_t t = figure_out_type(value.type);
  bool iden = false;
  if (t == RESB || t == STRING || t == FLOAT) {
    if (value.type == TOKEN_IDENTIFIER)
      iden = true;
    else {
      detailed_message(file.c_str(), value.line,
                       "Expected integer after variable type.", NULL);
      return false;
    }
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
  n->is_iden = iden;
  n->type = t;
  nodes.push_back(std::move(node));
  return true;
}

bool masm::GPCParser::handle_ds(Lexer &lexer, Token name) {
  lexer.next_token();
  Token value = lexer.next_token();
  data_t t = figure_out_type(value.type);
  bool iden = false;
  if (t != STRING) {
    if (value.type == TOKEN_IDENTIFIER)
      iden = true;
    else {
      detailed_message(file.c_str(), value.line,
                       "Expected string after string type.", NULL);
      return false;
    }
  }
  Node node;
  node.file = file;
  node.line = name.line;
  node.type = NODE_DS;
  node.node = std::make_unique<NodeDS>();
  NodeDS *n = (NodeDS *)node.node.get();
  n->name = name.value;
  n->value = value.value;
  n->is_iden = iden;
  n->type = t;
  nodes.push_back(std::move(node));
  return true;
}

bool masm::GPCParser::handle_df_dlf(Lexer &lexer, Token name) {
  Token type = lexer.next_token();
  Token value = lexer.next_token();
  data_t t = figure_out_type(value.type);
  bool iden = false;
  if (t != FLOAT) {
    if (value.type == TOKEN_IDENTIFIER)
      iden = true;
    else {
      detailed_message(file.c_str(), value.line,
                       "Expected Floating point value after float type.", NULL);
      return false;
    }
  }
  Node node;
  node.file = file;
  node.line = name.line;
  node.type = (type.type == TOKEN_DF) ? NODE_DF : NODE_DLF;
  node.node = std::make_unique<NodeDF>();
  NodeDF *n = (NodeDF *)node.node.get();
  n->name = name.value;
  n->value = value.value;
  n->is_iden = iden;
  n->type = t;
  nodes.push_back(std::move(node));
  return true;
}

bool masm::GPCParser::handle_resX(Lexer &lexer, Token name) {
  Token type = lexer.next_token();
  Token value = lexer.next_token();
  data_t t = figure_out_type(value.type);
  bool iden = false;
  if (t == FLOAT || t == RESB || t == STRING) {
    if (value.type == TOKEN_IDENTIFIER)
      iden = true;
    else {
      detailed_message(file.c_str(), value.line,
                       "Invalid reserve length provided.", NULL);
      return false;
    }
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
  n->is_iden = iden;
  n->type = t;
  nodes.push_back(std::move(node));
  return true;
}
