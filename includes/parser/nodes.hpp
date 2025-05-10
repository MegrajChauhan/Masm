#ifndef _NODES_
#define _NODES_

#include <filesystem>
#include <lexer_base.hpp>
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
  NODE_HALT,
  NODE_ADD_IMM,
  NODE_ADD_REGR,
  NODE_SUB_IMM,
  NODE_SUB_REGR,
  NODE_MUL_IMM,
  NODE_MUL_REGR,
  NODE_DIV_IMM,
  NODE_DIV_REGR,
  NODE_MOD_IMM,
  NODE_MOD_REGR,
  NODE_IADD_IMM,
  NODE_IADD_REGR,
  NODE_ISUB_IMM,
  NODE_ISUB_REGR,
  NODE_IMUL_IMM,
  NODE_IMUL_REGR,
  NODE_IDIV_IMM,
  NODE_IDIV_REGR,
  NODE_IMOD_IMM,
  NODE_IMOD_REGR,
  NODE_FADD_IMM,
  NODE_FADD_REGR,
  NODE_FSUB_IMM,
  NODE_FSUB_REGR,
  NODE_FMUL_IMM,
  NODE_FMUL_REGR,
  NODE_FDIV_IMM,
  NODE_FDIV_REGR,
  NODE_FADD32_IMM,
  NODE_FADD32_REGR,
  NODE_FSUB32_IMM,
  NODE_FSUB32_REGR,
  NODE_FMUL32_IMM,
  NODE_FMUL32_REGR,
  NODE_FDIV32_IMM,
  NODE_FDIV32_REGR,
  NODE_INC,
  NODE_DEC,
  NODE_AND_IMM,
  NODE_AND_REGR,
  NODE_OR_IMM,
  NODE_OR_REGR,
  NODE_XOR_IMM,
  NODE_XOR_REGR,
  NODE_SHL_IMM,
  NODE_SHL_REGR,
  NODE_SHR_IMM,
  NODE_SHR_REGR,
  NODE_NOT,
  NODE_CMP_IMM,
  NODE_CMP_REGR,
  NODE_RET,
  NODE_RETNZ,
  NODE_RETZ,
  NODE_RETNE,
  NODE_RETE,
  NODE_RETNC,
  NODE_RETC,
  NODE_RETNO,
  NODE_RETO,
  NODE_RETNN,
  NODE_RETN,
  NODE_RETNG,
  NODE_RETG,
  NODE_RETNS,
  NODE_RETS,
  NODE_RETGE,
  NODE_RETSE,
  NODE_PUSHA,
  NODE_POPA,
  NODE_OUTR,
  NODE_UOUTR,
  NODE_CFLAGS,
  NODE_RESET,
  NODE_MOV,
  NODE_MOVB,
  NODE_MOVW,
  NODE_MOVD,
  NODE_MOVQ,
  NODE_MOVF,
  NODE_MOVF32,
  NODE_MOVSXB_IMM,
  NODE_MOVSXB_REG,
  NODE_MOVSXW_IMM,
  NODE_MOVSXW_REG,
  NODE_MOVSXD_IMM,
  NODE_MOVSXD_REG,
  NODE_EXCGB,
  NODE_EXCGW,
  NODE_EXCGD,
  NODE_EXCGQ,
  NODE_MOVEB,
  NODE_MOVEW,
  NODE_MOVED,
  NODE_MOVEQ,
  NODE_MOVNZ,
  NODE_MOVZ,
  NODE_MOVNE,
  NODE_MOVE,
  NODE_MOVNC,
  NODE_MOVC,
  NODE_MOVNO,
  NODE_MOVO,
  NODE_MOVNN,
  NODE_MOVN,
  NODE_MOVNG,
  NODE_MOVG,
  NODE_MOVNS,
  NODE_MOVS,
  NODE_MOVGE,
  NODE_MOVSE,
  NODE_JNZ,
  NODE_JZ,
  NODE_JNE,
  NODE_JE,
  NODE_JNC,
  NODE_JC,
  NODE_JNO,
  NODE_JO,
  NODE_JNN,
  NODE_JN,
  NODE_JNG,
  NODE_JG,
  NODE_JNS,
  NODE_JS,
  NODE_JGE,
  NODE_JSE,
  NODE_INT,
  NODE_JMP_IMM,
  NODE_JMP_REG,
  NODE_CALL_IMM,
  NODE_CALL_REG,
  NODE_PUSHB,
  NODE_PUSHW,
  NODE_PUSHD,
  NODE_PUSHQ,
  NODE_PUSH,
  NODE_POPB_IMM,
  NODE_POPB_REG,
  NODE_POPW_IMM,
  NODE_POPW_REG,
  NODE_POPD_IMM,
  NODE_POPD_REG,
  NODE_POPQ_IMM,
  NODE_POPQ_REG,
  NODE_LOOP,
  NODE_LOADSB,
  NODE_LOADSW,
  NODE_LOADSD,
  NODE_LOADSQ,
  NODE_STORESB,
  NODE_STORESW,
  NODE_STORESD,
  NODE_STORESQ,
  NODE_FCMP,
  NODE_FCMP32,
  NODE_CIN,
  NODE_COUT,
  NODE_SIN_IMM,
  NODE_SIN_REG,
  NODE_SOUT_IMM,
  NODE_SOUT_REG,
  NODE_IN,
  NODE_OUT,
  NODE_INF,
  NODE_OUTF,
  NODE_INF32,
  NODE_OUTF32,
  NODE_INW,
  NODE_OUTW,
  NODE_IND,
  NODE_OUTD,
  NODE_INQ,
  NODE_OUTQ,
  NODE_UIN,
  NODE_UOUT,
  NODE_UINW,
  NODE_UOUTW,
  NODE_UIND,
  NODE_UOUTD,
  NODE_UINQ,
  NODE_UOUTQ,
  NODE_LOADB_IMM,
  NODE_LOADB_REG,
  NODE_LOADW_IMM,
  NODE_LOADW_REG,
  NODE_LOADD_IMM,
  NODE_LOADD_REG,
  NODE_LOADQ_IMM,
  NODE_LOADQ_REG,
  NODE_STOREB_IMM,
  NODE_STOREB_REG,
  NODE_STOREW_IMM,
  NODE_STOREW_REG,
  NODE_STORED_IMM,
  NODE_STORED_REG,
  NODE_STOREQ_IMM,
  NODE_STOREQ_REG,
  NODE_ATM_LOADB_IMM,
  NODE_ATM_LOADB_REG,
  NODE_ATM_LOADW_IMM,
  NODE_ATM_LOADW_REG,
  NODE_ATM_LOADD_IMM,
  NODE_ATM_LOADD_REG,
  NODE_ATM_LOADQ_IMM,
  NODE_ATM_LOADQ_REG,
  NODE_ATM_STOREB_IMM,
  NODE_ATM_STOREB_REG,
  NODE_ATM_STOREW_IMM,
  NODE_ATM_STOREW_REG,
  NODE_ATM_STORED_IMM,
  NODE_ATM_STORED_REG,
  NODE_ATM_STOREQ_IMM,
  NODE_ATM_STOREQ_REG,
  NODE_WHDLR,
  NODE_LEA,
  NODE_CMPXCHG_IMM,
  NODE_CMPXCHG_REG
};

struct NodeBase {};

struct NodeIncDir : public NodeBase {
  std::string path_included;
};

struct NodeConstDef : public NodeBase {
  std::string const_name;
  std::string const_value;
  value_t type;
};

struct NodeDB : public NodeBase {
  std::string name;
  std::string value;
  value_t type;
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

// GPC Instruction Nodes
struct NodeRegrImm : public NodeBase {
  token_t regr;
  std::string immediate;
  value_t type;
  bool is_var = false;
};

struct NodeRegReg : public NodeBase {
  token_t r1, r2;
};

struct NodeReg : public NodeBase {
  token_t reg;
};

struct NodeImm : public NodeBase {
  std::string imm;
  value_t type;
  bool is_var = false;
};

struct NodeLea : public NodeBase {
  token_t r1, r2, r3, r4;
};

struct NodeCMPXCHGImm : public NodeBase {
  token_t r1, r2;
  std::string imm;
  value_t type;
};

struct NodeCMPXCHGReg : public NodeBase {
  token_t r1, r2, r3;
};

struct Node {
  node_t type;
  size_t line;
  std::filesystem::path file;
  std::unique_ptr<NodeBase> node;
};
}; // namespace masm

#endif
