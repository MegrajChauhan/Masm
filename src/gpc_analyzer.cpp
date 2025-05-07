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
  return true;
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
    case NODE_LABEL:
    case NODE_DB:
    case NODE_DW:
    case NODE_DD:
    case NODE_DQ:
    case NODE_DS:
    case NODE_DF:
    case NODE_DLF:
    case NODE_RESB:
    case NODE_RESW:
    case NODE_RESD:
    case NODE_RESQ:
    case NODE_RESF:
    case NODE_RESLF:
    case NODE_NOP: {
      result.push_back(std::move(n));
      break;
    }
    }
  }
  return true;
}
