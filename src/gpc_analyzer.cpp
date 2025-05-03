#include <gpc_analyzer.hpp>

masm::GPCAnalyzer::GPCAnalyzer(
    std::vector<Node> &&n,
    std::unordered_map<std::string, std::pair<data_t, std::string>> &c,
    std::unordered_set<std::string> &l, SymbolTable &s)
    : nodes(std::move(n)), consts(c), labels(l), symtable(s) {}

bool masm::GPCAnalyzer::analyze() {
  // We perform two loops here:
  // One loop will only check for labels and variable.
  // This first loop will populate the symboltable and the labels
  // list.
  // The second loop will finally analyze the Instructions.
  return true;
}

bool masm::GPCAnalyzer::valid_identifier_in_var_declr(Node &n) {
  switch (n.type) {
  case NODE_DB: {
    NodeDB *b = (NodeDB *)n.node.get();
    if (b->is_iden) {
      std::unordered_map<std::string, std::pair<data_t, std::string>>::iterator
          C = consts.find(b->value);
      if (C == consts.end()) {
        detailed_message(n.file.c_str(), n.line,
                         "Unknown constant used in variable definition '%s'.",
                         b->value.c_str());
        return false;
      }
      std::pair<data_t, std::string> val = C->second;
      if (val.first == STRING || val.first == FLOAT) {
        detailed_message(n.file.c_str(), n.line,
                         "Variable type and constant type doesn't match '%s'.",
                         b->value.c_str());
        return false;
      }
      b->value = val.second;
      b->type = val.first;
    } else {
    }
    break;
  }
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
      labels.insert(label->name);
      break;
    }
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
    case NODE_RESLF: {
    }
    }
  }
  return true;
}
