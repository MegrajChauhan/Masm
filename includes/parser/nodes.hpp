#ifndef _NODES_
#define _NODES_

#include <filesystem>
#include <memory>
#include <symboltable.hpp>

namespace masm {
enum node_t {
  INCLUDE_DIR,
  CONST_DEF,
  // GPC
  NODE_DB,
  NODE_DW,
  NODE_DD,
  NODE_DQ,
  NODE_DS,
  NODE_DF,
  NODE_DLF,
  NODE_RESB,
  NODE_RESW,
  NODE_RESD,
  NODE_RESQ,
  NODE_RESF,
  NODE_RESLF,
  NODE_LABEL,
  NODE_NOP,
};

struct NodeBase {};

struct NodeIncDir : public NodeBase {
  std::string path_included;
};

struct NodeConstDef : public NodeBase {
  std::string const_name;
  std::string const_value;
  data_t type;
};

struct NodeDB : public NodeBase {
  std::string name;
  std::string value;
  data_t type;
  bool is_iden;
};

struct NodeDW : public NodeDB {};
struct NodeDD : public NodeDB {};
struct NodeDQ : public NodeDB {};
struct NodeDS : public NodeDB {};
struct NodeDF : public NodeDB {};

struct NodeRESB : public NodeDB {};

struct NodeRESW : public NodeRESB {};
struct NodeRESD : public NodeRESB {};
struct NodeRESQ : public NodeRESB {};
struct NodeRESS : public NodeRESB {};
struct NodeRESF : public NodeRESB {};

struct NodeLabel : public NodeBase {
  std::string name;
};

struct Node {
  node_t type;
  size_t line;
  std::filesystem::path file;
  std::unique_ptr<NodeBase> node;
};
}; // namespace masm

#endif
