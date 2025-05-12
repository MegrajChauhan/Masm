#include <gpc_analyzer.hpp>

masm::GPCAnalyzer::GPCAnalyzer(
    std::vector<Node> &&n,
    std::unordered_map<std::string, std::pair<value_t, std::string>> &c,
    std::unordered_set<std::string> &l, SymbolTable &s)
    : nodes(std::move(n)), consts(c), labels(l), symtable(s) {}

bool masm::GPCAnalyzer::analyze() {
  // We perform two loops here:
  // One loop will only check for labels and variable.
  // This first loop will populate the symboltable and the labels
  // list.
  // The second loop will finally analyze the Instructions.
  if (!first_loop())
    return false;
  return second_loop();
}

bool masm::GPCAnalyzer::validate_defined_variables(Node &n) {
  switch (n.type) {
  case NODE_DQ:
  case NODE_DD:
  case NODE_DW:
  case NODE_DB: {
    NodeDB *b = (NodeDB *)n.node.get();
    if (b->type == VALUE_IDEN) {
      std::unordered_map<std::string, std::pair<value_t, std::string>>::iterator
          C = consts.find(b->value);
      if (C == consts.end()) {
        detailed_message(n.file.c_str(), n.line,
                         "Unknown constant used in variable definition '%s'.",
                         b->value.c_str());
        return false;
      }
      std::pair<value_t, std::string> val = C->second;
      if (val.first == VALUE_STRING || val.first == VALUE_FLOAT) {
        detailed_message(n.file.c_str(), n.line,
                         "Variable type and constant type doesn't match '%s'.",
                         b->value.c_str());
        return false;
      }
      b->value = val.second;
      b->type = val.first;
    }
    break;
  }
  case NODE_DS: {
    NodeDS *b = (NodeDS *)n.node.get();
    if (b->type == VALUE_IDEN) {
      std::unordered_map<std::string, std::pair<value_t, std::string>>::iterator
          C = consts.find(b->value);
      if (C == consts.end()) {
        detailed_message(n.file.c_str(), n.line,
                         "Unknown constant used in variable definition '%s'.",
                         b->value.c_str());
        return false;
      }
      std::pair<value_t, std::string> val = C->second;
      if (val.first != VALUE_STRING) {
        detailed_message(n.file.c_str(), n.line,
                         "Variable type and constant type doesn't match '%s'.",
                         b->value.c_str());
        return false;
      }
      b->value = val.second;
      b->type = val.first;
    }
    break;
  }
  case NODE_DLF:
  case NODE_DF: {
    NodeDF *b = (NodeDF *)n.node.get();
    if (b->type == VALUE_IDEN) {
      std::unordered_map<std::string, std::pair<value_t, std::string>>::iterator
          C = consts.find(b->value);
      if (C == consts.end()) {
        detailed_message(n.file.c_str(), n.line,
                         "Unknown constant used in variable definition '%s'.",
                         b->value.c_str());
        return false;
      }
      std::pair<value_t, std::string> val = C->second;
      if (val.first != VALUE_FLOAT) {
        detailed_message(n.file.c_str(), n.line,
                         "Variable type and constant type doesn't match '%s'.",
                         b->value.c_str());
        return false;
      }
      b->value = val.second;
      b->type = val.first;
    }
    break;
  }
  default:
    break;
  }
  return true;
}

bool masm::GPCAnalyzer::validate_reserved_variables(Node &n) {
  switch (n.type) {
  case NODE_RESF:
  case NODE_RESLF:
  case NODE_RESQ:
  case NODE_RESD:
  case NODE_RESW:
  case NODE_RESB: {
    NodeRESB *b = (NodeRESB *)n.node.get();
    if (b->type == VALUE_IDEN) {
      std::unordered_map<std::string, std::pair<value_t, std::string>>::iterator
          C = consts.find(b->value);
      if (C == consts.end()) {
        detailed_message(n.file.c_str(), n.line,
                         "Unknown constant used in variable definition '%s'.",
                         b->value.c_str());
        return false;
      }
      std::pair<value_t, std::string> val = C->second;
      if (val.first != VALUE_INTEGER) {
        detailed_message(n.file.c_str(), n.line,
                         "Not a valid length for resX '%s'.", b->value.c_str());
        return false;
      }
      if (val.second.starts_with('-')) {
        detailed_message(n.file.c_str(), n.line,
                         "Cannot have negative length for resX '%s'.",
                         b->value.c_str());
        return false;
      }
      if (val.second == "0") {
        detailed_message(n.file.c_str(), n.line,
                         "Cannot have zero length for resX '%s'.",
                         b->value.c_str());
        return false;
      }
      b->value = val.second;
      b->type = val.first;
    } else {
      if (b->value.starts_with('-')) {
        detailed_message(n.file.c_str(), n.line,
                         "Cannot have negative length for resX '%s'.",
                         b->value.c_str());
        return false;
      }
      if (b->value == "0") {
        detailed_message(n.file.c_str(), n.line,
                         "Cannot have zero length for resX '%s'.",
                         b->value.c_str());
        return false;
      }
    }
    break;
  }
  default:
    break;
  }
  return true;
}

std::vector<masm::Node> masm::GPCAnalyzer::get_result() {
  return std::move(result);
}

bool masm::GPCAnalyzer::first_loop() {
  for (Node &n : nodes) {
    switch (n.type) {
    case NODE_LABEL: {
      NodeLabel *label = (NodeLabel *)n.node.get();
      if (labels.find(label->name) != labels.end()) {
        detailed_message(n.file.c_str(), n.line,
                         "Re-declaration of label '%s'.", label->name.c_str());
        return false;
      }
      if (symtable.symbol_exists(label->name)) {
        detailed_message(n.file.c_str(), n.line,
                         "Re-declaration of variable as label '%s'.",
                         label->name.c_str());
        return false;
      }
      labels.insert(label->name);
      break;
    }
    case NODE_DB:
    case NODE_DW:
    case NODE_DD:
    case NODE_DQ:
    case NODE_DS:
    case NODE_DF:
    case NODE_DLF: {
      NodeDB *dX = (NodeDB *)n.node.get();
      if (labels.find(dX->name) != labels.end()) {
        detailed_message(n.file.c_str(), n.line,
                         "Re-declaration of label as variable '%s'.",
                         dX->name.c_str());
        return false;
      }
      if (symtable.symbol_exists(dX->name)) {
        detailed_message(n.file.c_str(), n.line,
                         "Variable '%s' already exists- redeclaration.",
                         dX->name.c_str());
        return false;
      }
      if (!validate_defined_variables(n))
        return false;
      Symbol sym;
      sym.type = (n.type == NODE_DB)   ? BYTE
                 : (n.type == NODE_DW) ? WORD
                 : (n.type == NODE_DD) ? DWORD
                 : (n.type == NODE_DQ) ? QWORD
                 : (n.type == NODE_DS) ? STRING
                                       : FLOAT;
      sym.val_type = dX->type;
      sym.value = dX->value;
      symtable.add_symbol(dX->name, sym);
      break;
    }
    case NODE_RESB:
    case NODE_RESW:
    case NODE_RESD:
    case NODE_RESQ:
    case NODE_RESF:
    case NODE_RESLF: {
      NodeRESB *resX = (NodeRESB *)n.node.get();
      if (labels.find(resX->name) != labels.end()) {
        detailed_message(n.file.c_str(), n.line,
                         "Re-declaration of label as variable '%s'.",
                         resX->name.c_str());
        return false;
      }
      if (symtable.symbol_exists(resX->name)) {
        detailed_message(n.file.c_str(), n.line,
                         "Variable '%s' already exists- redeclaration.",
                         resX->name.c_str());
        return false;
      }
      if (!validate_reserved_variables(n))
        return false;
      Symbol sym;
      sym.type = (n.type == NODE_RESB)   ? BYTE
                 : (n.type == NODE_RESW) ? WORD
                 : (n.type == NODE_RESD) ? DWORD
                 : (n.type == NODE_RESQ) ? QWORD
                                         : FLOAT;
      sym.val_type = resX->type;
      sym.value = resX->value;
      symtable.add_symbol(resX->name, sym);
      break;
    }
    default:
      break;
    }
  }
  return true;
}

bool masm::GPCAnalyzer::second_loop() {
  /**
   * First loop dealt with symtable population and constant resolution.
   * second loop will analyze the Instructions
   * */
  for (Node &n : nodes) {
    switch (n.type) {
    case CONST_DEF:
      break;
    case NODE_CMP_IMM:
    case NODE_IMOD_IMM:
    case NODE_IDIV_IMM:
    case NODE_IMUL_IMM:
    case NODE_ISUB_IMM:
    case NODE_IADD_IMM:
    case NODE_MOD_IMM:
    case NODE_DIV_IMM:
    case NODE_MUL_IMM:
    case NODE_SUB_IMM:
    case NODE_ADD_IMM: {
      NodeRegrImm *ri = (NodeRegrImm *)n.node.get();
      if (ri->type == VALUE_IDEN) {
        std::pair<bool, std::pair<masm::value_t, std::string>> c =
            resolve_if_constant(ri->immediate, {VALUE_INTEGER, VALUE_BINARY,
                                                VALUE_HEX, VALUE_OCTAL});

        if (c.first) {
          // Indeed a constant
          n.len = 2;
          ri->immediate = c.second.second;
          ri->type = c.second.first;
        } else {
          // Not a constant but could be a variable
          if (resolve_variable(ri->immediate, {BYTE, WORD, DWORD, QWORD})) {
            ri->is_var = true;
            n.len = 1;
          } else {
            detailed_message(n.file.c_str(), n.line,
                             "Unknwon IMMEDIATE value type: Not a constant and "
                             "not a variable.",
                             NULL);
            return false;
          }
        }
      }
      break;
    }
    case NODE_FDIV32_IMM:
    case NODE_FMUL32_IMM:
    case NODE_FSUB32_IMM:
    case NODE_FADD32_IMM:
    case NODE_FDIV_IMM:
    case NODE_FMUL_IMM:
    case NODE_FSUB_IMM:
    case NODE_FADD_IMM: {
      NodeRegrImm *ri = (NodeRegrImm *)n.node.get();
      if (ri->type == VALUE_IDEN) {
        if (resolve_variable(ri->immediate, {FLOAT})) {
          ri->is_var = true;
          n.len = 1;
        } else {
          detailed_message(
              n.file.c_str(), n.line,
              "Expected VARIABLE as operand but got something else.", NULL);
          return false;
        }
      } else {
        detailed_message(n.file.c_str(), n.line,
                         "Expected VARIABLE as operand but got something else.",
                         NULL);
        return false;
      }
      break;
    }
    case NODE_MOVSXB_IMM:
    case NODE_MOVSXW_IMM:
    case NODE_MOVSXD_IMM:
    case NODE_MOVF:
    case NODE_MOVF32:
    case NODE_MOVNZ:
    case NODE_MOVZ:
    case NODE_MOVNE:
    case NODE_MOVE:
    case NODE_MOVNC:
    case NODE_MOVC:
    case NODE_MOVNO:
    case NODE_MOVO:
    case NODE_MOVNN:
    case NODE_MOVN:
    case NODE_MOVNG:
    case NODE_MOVG:
    case NODE_MOVNS:
    case NODE_MOVS:
    case NODE_MOVGE:
    case NODE_MOVSE:
    case NODE_MOV: {
      NodeRegrImm *ri = (NodeRegrImm *)n.node.get();
      if (ri->type == VALUE_IDEN) {
        bool f = n.type == NODE_MOVF || n.type == NODE_MOVF32;
        std::pair<bool, std::pair<masm::value_t, std::string>> c =
            resolve_if_constant(
                ri->immediate,
                f ? std::vector<value_t>{VALUE_FLOAT}
                  : std::vector<value_t>(
                        {VALUE_INTEGER, VALUE_BINARY, VALUE_HEX, VALUE_OCTAL}));

        if (c.first) {
          n.len = 2;
          ri->immediate = c.second.second;
          ri->type = c.second.first;
        } else {
          detailed_message(n.file.c_str(), n.line,
                           "Unknwon IMMEDIATE value type: Not a constant when "
                           "expected a constant.",
                           NULL);
          return false;
        }
      }
      break;
    }
    case NODE_WHDLR:
    case NODE_CALL_IMM:
    case NODE_JNZ:
    case NODE_JZ:
    case NODE_JNE:
    case NODE_JE:
    case NODE_JNC:
    case NODE_JC:
    case NODE_JNO:
    case NODE_JO:
    case NODE_JNN:
    case NODE_JN:
    case NODE_JNG:
    case NODE_JG:
    case NODE_JNS:
    case NODE_JS:
    case NODE_JGE:
    case NODE_JSE:
    case NODE_JMP_IMM: {
      NodeImm *i = (NodeImm *)n.node.get();
      if (labels.find(i->imm) == labels.end()) {
        detailed_message(n.file.c_str(), n.line,
                         "Unknwon LABEL to jump to: Not a valid label.", NULL);
        return false;
      }
      break;
    }
    case NODE_LOOP: {
      NodeRegrImm *i = (NodeRegrImm *)n.node.get();
      if (labels.find(i->immediate) == labels.end()) {
        detailed_message(n.file.c_str(), n.line,
                         "Unknwon LABEL to jump to: Not a valid label.", NULL);
        return false;
      }
      break;
    }
    case NODE_INT: {
      NodeRegrImm *ri = (NodeRegrImm *)n.node.get();
      if (ri->type == VALUE_IDEN) {
        std::pair<bool, std::pair<masm::value_t, std::string>> c =
            resolve_if_constant(ri->immediate, {VALUE_INTEGER, VALUE_BINARY,
                                                VALUE_HEX, VALUE_OCTAL});

        if (c.first) {
          ri->immediate = c.second.second;
          ri->type = c.second.first;
        } else {
          detailed_message(n.file.c_str(), n.line,
                           "Unknwon IMMEDIATE value type: Not a constant.",
                           NULL);
          return false;
        }
      }
      break;
    }
    case NODE_PUSHB:
      if (!analyze_stack_based_instructions(
              n, BYTE, {VALUE_INTEGER, VALUE_BINARY, VALUE_HEX, VALUE_HEX}))
        return false;
      break;
    case NODE_PUSHW:
      if (!analyze_stack_based_instructions(
              n, WORD, {VALUE_INTEGER, VALUE_BINARY, VALUE_HEX, VALUE_HEX}))
        return false;
      break;
    case NODE_PUSHD:
      if (!analyze_stack_based_instructions(
              n, DWORD, {VALUE_INTEGER, VALUE_BINARY, VALUE_HEX, VALUE_HEX}))
        return false;
      break;
    case NODE_PUSHQ:
      if (!analyze_stack_based_instructions(
              n, QWORD, {VALUE_INTEGER, VALUE_BINARY, VALUE_HEX, VALUE_HEX}))
        return false;
      break;
    case NODE_POPB_IMM:
      if (!analyze_stack_based_instructions(n, BYTE, {}))
        return false;
      break;
    case NODE_POPW_IMM:
      if (!analyze_stack_based_instructions(n, WORD, {}))
        return false;
      break;
    case NODE_POPD_IMM:
      if (!analyze_stack_based_instructions(n, DWORD, {}))
        return false;
      break;
    case NODE_POPQ_IMM:
      if (!analyze_stack_based_instructions(n, QWORD, {}))
        return false;
      break;
    case NODE_LOADSB:
    case NODE_LOADSW:
    case NODE_LOADSD:
    case NODE_LOADSQ:
    case NODE_STORESB:
    case NODE_STORESW:
    case NODE_STORESD:
    case NODE_STORESQ:
      if (!analyze_instructions_with_only_const_imm(
              n, {VALUE_INTEGER, VALUE_BINARY, VALUE_HEX, VALUE_OCTAL}))
        return false;
      break;
    case NODE_AND_IMM:
    case NODE_OR_IMM:
    case NODE_XOR_IMM:
    case NODE_SHL_IMM:
    case NODE_SHR_IMM:
      if (!analyze_instructions_with_only_const_imm(
              n, {VALUE_INTEGER, VALUE_BINARY, VALUE_HEX, VALUE_OCTAL}))
        n.len = 2;
      return false;
      break;
    case NODE_SOUT_IMM:
    case NODE_SIN_IMM: {
      NodeImm *i = (NodeImm *)n.node.get();
      if (!symtable.symbol_exists(i->imm)) {
        detailed_message(n.file.c_str(), n.line,
                         "This variable doesn't exists '%s'.", i->imm.c_str());
        return false;
      }
      break;
    }
    case NODE_LOADB_IMM:
      if (!analyze_load_store_instructions(n, BYTE))
        return false;
      break;
    case NODE_LOADW_IMM:
      if (!analyze_load_store_instructions(n, WORD))
        return false;
      break;
    case NODE_LOADD_IMM:
      if (!analyze_load_store_instructions(n, DWORD))
        return false;
      break;
    case NODE_LOADQ_IMM:
      if (!analyze_load_store_instructions(n, QWORD))
        return false;
      break;
    case NODE_STOREB_IMM:
      if (!analyze_load_store_instructions(n, BYTE))
        return false;
      break;
    case NODE_STOREW_IMM:
      if (!analyze_load_store_instructions(n, WORD))
        return false;
      break;
    case NODE_STORED_IMM:
      if (!analyze_load_store_instructions(n, DWORD))
        return false;
      break;
    case NODE_STOREQ_IMM:
      if (!analyze_load_store_instructions(n, QWORD))
        return false;
      break;
    case NODE_CMPXCHG_IMM: {
      NodeCMPXCHGImm *i = (NodeCMPXCHGImm *)n.node.get();
      if (!resolve_variable(i->imm, {BYTE})) {
        detailed_message(n.file.c_str(), n.line,
                         "Invalid CMPXCHG Instruction format.", NULL);
        return false;
      }
      break;
    }
    default:
      break;
    }
    if (n.type != CONST_DEF)
      result.push_back(std::move(n));
  }
  return true;
}

std::pair<bool, std::pair<masm::value_t, std::string>>
masm::GPCAnalyzer::resolve_if_constant(std::string name,
                                       std::vector<value_t> expected) {
  std::unordered_map<std::string, std::pair<value_t, std::string>>::iterator
      res = consts.find(name);
  if (res == consts.end())
    return std::make_pair(false, std::make_pair(VALUE_ERR, ""));

  if (std::find(expected.begin(), expected.end(), res->second.first) ==
      expected.end())
    return std::make_pair(false, std::make_pair(VALUE_ERR, ""));
  return std::make_pair(true, res->second);
}

bool masm::GPCAnalyzer::resolve_variable(std::string name,
                                         std::array<data_t, 4> expected) {
  if (!symtable.symbol_exists(name))
    return false;
  auto res = symtable.find_symbol(name);
  if (std::find(expected.begin(), expected.end(), res->second.type) ==
      expected.end())
    return false;
  return true;
}

bool masm::GPCAnalyzer::analyze_stack_based_instructions(
    Node &n, data_t expected, std::vector<value_t> vtlist) {
  NodeImm *imm = (NodeImm *)n.node.get();
  if (imm->type == VALUE_IDEN) {
    if (resolve_variable(imm->imm, {expected})) {
      imm->is_var = true;
      return true;
    } else {
      std::pair<bool, std::pair<masm::value_t, std::string>> res =
          resolve_if_constant(imm->imm, vtlist);
      if (!res.first) {
        detailed_message(n.file.c_str(), n.line,
                         "Invalid Operand For Instruction", NULL);
        return false;
      } else {
        imm->imm = res.second.second;
        imm->type = res.second.first;
        n.len = 2;
      }
    }
  }
  switch (n.type) {
  case NODE_POPB_IMM:
  case NODE_POPW_IMM:
  case NODE_POPD_IMM:
  case NODE_POPQ_IMM:
    if (!imm->is_var) {
      detailed_message(n.file.c_str(), n.line,
                       "Invalid Operand For Instruction", NULL);
      return false;
    }
  default:
    break;
  }
  return true;
}

bool masm::GPCAnalyzer::analyze_load_store_instructions(Node &n,
                                                        data_t expected) {
  NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
  if (imm->type == VALUE_IDEN) {
    if (resolve_variable(imm->immediate, {expected})) {
      imm->is_var = true;
      return true;
    } else {
      detailed_message(n.file.c_str(), n.line,
                       "Invalid Operand For Instruction", NULL);
      return false;
    }
  } else {
    detailed_message(
        n.file.c_str(), n.line,
        "Instruction doesn't accept anything other than a variable.", NULL);
    return false;
  }
  return true;
}

bool masm::GPCAnalyzer::analyze_instructions_with_only_const_imm(
    Node &n, std::vector<value_t> expected) {
  NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
  if (imm->type == VALUE_IDEN) {
    std::pair<bool, std::pair<masm::value_t, std::string>> res =
        resolve_if_constant(imm->immediate, expected);
    if (!res.first) {
      detailed_message(n.file.c_str(), n.line,
                       "Invalid Operand For Instruction", NULL);
      return false;
    } else {
      imm->immediate = res.second.second;
      imm->type = res.second.first;
    }
  }
  return true;
}
