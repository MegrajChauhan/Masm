#include <gpc_gen.hpp>

masm::GPCGen::GPCGen(std::vector<Node> &n, SymbolTable &t, uint64_t a)
    : final_nodes(std::move(n)), symtable(t), st_address_data(a) {}

uint64_t masm::GPCGen::get_current_address_point() { return st_address_data; }

std::vector<masm::Inst64> masm::GPCGen::get_instructions() {
  return std::move(instructions);
}

std::vector<uint8_t> masm::GPCGen::get_data() { return std::move(data); }

bool masm::GPCGen::generate() { return true; }

bool masm::GPCGen::first_iteration() {
  // First iteration will generate addresses for data and labels.
  // Here we will only generate data

  // To achieve the goals of first iteration:
  // 1) First generate the addresses for all the labels.
  // 2) Go through the nodes again but generate addresses for data.
  //
  // The plan is to have a new data type ADDRESS which would be simply
  // pointer in conventional programming language but in this case
  // ADDRESS also holds addresses for procedures.
  //
  // It is to be noted that procedure address and variable address
  // are two completely different things since procedures reside in
  // the instruction memory of the core while the data memory holds
  // the data which is shared among the cores.
  //
  // Thus, to achieve the goals, we have to:
  // 1) First generate the addresses for all labels
  // 2) Go through the nodes again to generate addresses for the data
  //    including the ADDRESS variables.
  // 3) When going through the nodes the third time, initialize the
  //    ADDRESS variables.
  // To minimize the number of iterations, we will use the same third
  // iteration to generate the instructions as well.

  uint64_t i = 8;
  for (Node &n : final_nodes) {
    switch (n.type) {
    case NODE_LABEL: {
      NodeLabel *lbl = (NodeLabel *)n.node.get();
      label_addresses[lbl->name] = i;
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
    case NODE_RESLF:
      break;
    default:
      i += 8;
    }
  }
  for (Node &n : final_nodes) {
    switch (n.type) {
    case NODE_LABEL:
      break;
    case NODE_DB: {
      NodeDB *db = (NodeDB *)n.node.get();
      data_addresses[db->name] = st_address_data;
      add_data(db->value, db->type, 1);
      st_address_data++;
      break;
    }
    case NODE_DW: {
      NodeDW *dw = (NodeDW *)n.node.get();
      data_addresses[dw->name] = st_address_data;
      add_data(dw->value, dw->type, 2);
      st_address_data += 2;
      break;
    }
    case NODE_DD: {
      NodeDD *dd = (NodeDD *)n.node.get();
      data_addresses[dd->name] = st_address_data;
      add_data(dd->value, dd->type, 4);
      st_address_data += 4;
      break;
    }
    case NODE_DQ: {
      NodeDQ *dq = (NodeDQ *)n.node.get();
      data_addresses[dq->name] = st_address_data;
      add_data(dq->value, dq->type, 8);
      st_address_data += 8;
      break;
    }
    case NODE_DF: {
      NodeDF *df = (NodeDF *)n.node.get();
      data_addresses[df->name] = st_address_data;
      add_data(df->value, df->type, 4);
      st_address_data += 4;
      break;
    }
    case NODE_DLF: {
      NodeDF *dlf = (NodeDF *)n.node.get();
      data_addresses[dlf->name] = st_address_data;
      add_data(dlf->value, dlf->type, 8);
      st_address_data += 8;
      break;
    }
    case NODE_DS: {
      NodeDS *ds = (NodeDS *)n.node.get();
      data_addresses[ds->name] = st_address_data;
      add_data(ds->value, ds->type, 0);
      st_address_data += ds->value.length();
      break;
    }
    case NODE_RESB: {
      NodeRESB *res = (NodeRESB *)n.node.get();
      data_addresses[res->name] = st_address_data;
      add_reserved_data(res->value, res->type, 1);
      break;
    }
    case NODE_RESW: {
      NodeRESW *res = (NodeRESW *)n.node.get();
      data_addresses[res->name] = st_address_data;
      add_reserved_data(res->value, res->type, 2);
      break;
    }
    case NODE_RESD: {
      NodeRESD *res = (NodeRESD *)n.node.get();
      data_addresses[res->name] = st_address_data;
      add_reserved_data(res->value, res->type, 4);
      break;
    }
    case NODE_RESQ: {
      NodeRESQ *res = (NodeRESQ *)n.node.get();
      data_addresses[res->name] = st_address_data;
      add_reserved_data(res->value, res->type, 8);
      break;
    }
    case NODE_RESF: {
      NodeRESF *res = (NodeRESF *)n.node.get();
      data_addresses[res->name] = st_address_data;
      add_reserved_data(res->value, res->type, 4);
      break;
    }
    case NODE_RESLF: {
      NodeRESF *res = (NodeRESF *)n.node.get();
      data_addresses[res->name] = st_address_data;
      add_reserved_data(res->value, res->type, 8);
      break;
    }
    }
  }
  return true;
}

bool masm::GPCGen::second_iteration() {
  // Now we can go through the nodes and generate Instructions
  // First instruction is actually a jump instruction to the
  // entry procedure named main which must be defined
  // otherwise there will be errors.
  if (label_addresses.find("main") == label_addresses.end()) {
    simple_message("MISSING entry procedure MAIN.... Cannot generate code.",
                   NULL);
    return false;
  }

  /*PUSH a JUMP instruction to MAIN here*/

  // Due to MY lack of brain power during the design of the
  // assembler, we have to deal with insanity.
  // We could have grouped the instructions tight enough such
  // that a very small amount of logic would have sufficed
  // to cover the entire ISA but now we need a lot more logic
  // There are still groups that can be made but they are small
  // and not dense enough.
  // If I had TRULY separated entire backends for every core
  // from the very beginning then every step could have been
  // simplified.

  for (Node &n : final_nodes) {
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
      break;
    case NODE_NOP:
      simple_instructions(OP_NOP);
      break;
    case NODE_HALT:
      simple_instructions(OP_HALT);
      break;
    case NODE_RET:
      simple_instructions(OP_RET);
      break;
    case NODE_RETNZ:
      simple_instructions(OP_RETNZ);
      break;
    case NODE_RETZ:
      simple_instructions(OP_RETZ);
      break;
    case NODE_RETNE:
      simple_instructions(OP_RETNE);
      break;
    case NODE_RETE:
      simple_instructions(OP_RETE);
      break;
    case NODE_RETNC:
      simple_instructions(OP_RETNC);
      break;
    case NODE_RETC:
      simple_instructions(OP_RETC);
      break;
    case NODE_RETNO:
      simple_instructions(OP_RETNO);
      break;
    case NODE_RETO:
      simple_instructions(OP_RETO);
      break;
    case NODE_RETNN:
      simple_instructions(OP_RETNN);
      break;
    case NODE_RETN:
      simple_instructions(OP_RETN);
      break;
    case NODE_RETNG:
      simple_instructions(OP_RETNG);
      break;
    case NODE_RETG:
      simple_instructions(OP_RETG);
      break;
    case NODE_RETNS:
      simple_instructions(OP_RETNS);
      break;
    case NODE_RETS:
      simple_instructions(OP_RETS);
      break;
    case NODE_RETGE:
      simple_instructions(OP_RETGE);
      break;
    case NODE_RETSE:
      simple_instructions(OP_RETSE);
      break;
    case NODE_PUSHA:
      simple_instructions(OP_PUSHA);
      break;
    case NODE_POPA:
      simple_instructions(OP_POPA);
      break;
    case NODE_OUTR:
      simple_instructions(OP_OUTR);
      break;
    case NODE_UOUTR:
      simple_instructions(OP_UOUTR);
      break;
    case NODE_CFLAGS:
      simple_instructions(OP_CFLAGS);
      break;
    case NODE_RESET:
      simple_instructions(OP_RESET);
      break;
    case NODE_INC: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_INC, token_to_regr(r->reg));
      break;
    }
    case NODE_DEC: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_DEC, token_to_regr(r->reg));
      break;
    }
    case NODE_CALL_REG: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_CALL_REG, token_to_regr(r->reg));
      break;
    }
    case NODE_JMP_REG: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_JMP_REGR, token_to_regr(r->reg));
      break;
    }
    case NODE_PUSH: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_PUSH_REG, token_to_regr(r->reg));
      break;
    }
    case NODE_POPB_REG: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_POP8, token_to_regr(r->reg));
      break;
    }
    case NODE_POPW_REG: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_POP16, token_to_regr(r->reg));
      break;
    }
    case NODE_POPD_REG: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_POP32, token_to_regr(r->reg));
      break;
    }
    case NODE_POPQ_REG: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_POP64, token_to_regr(r->reg));
      break;
    }
    case NODE_NOT: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_NOT, token_to_regr(r->reg));
      break;
    }
    case NODE_CIN: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_CIN, token_to_regr(r->reg));
      break;
    }
    case NODE_COUT: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_COUT, token_to_regr(r->reg));
      break;
    }
    case NODE_IN: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_IN, token_to_regr(r->reg));
      break;
    }
    case NODE_OUT: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_OUT, token_to_regr(r->reg));
      break;
    }
    case NODE_INW: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_INW, token_to_regr(r->reg));
      break;
    }
    case NODE_OUTW: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_OUTW, token_to_regr(r->reg));
      break;
    }
    case NODE_IND: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_IND, token_to_regr(r->reg));
      break;
    }
    case NODE_OUTD: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_OUTD, token_to_regr(r->reg));
      break;
    }
    case NODE_INQ: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_OUTQ, token_to_regr(r->reg));
      break;
    }
    case NODE_UIN: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_UIN, token_to_regr(r->reg));
      break;
    }
    case NODE_UOUT: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_UOUT, token_to_regr(r->reg));
      break;
    }
    case NODE_UINW: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_UINW, token_to_regr(r->reg));
      break;
    }
    case NODE_UOUTW: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_UOUTW, token_to_regr(r->reg));
      break;
    }
    case NODE_UIND: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_UIND, token_to_regr(r->reg));
      break;
    }
    case NODE_UOUTD: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_UOUTD, token_to_regr(r->reg));
      break;
    }
    case NODE_UINQ: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_UINQ, token_to_regr(r->reg));
      break;
    }
    case NODE_UOUTQ: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_UOUTQ, token_to_regr(r->reg));
      break;
    }
    case NODE_INF: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_INF, token_to_regr(r->reg));
      break;
    }
    case NODE_INF32: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_INF32, token_to_regr(r->reg));
      break;
    }
    case NODE_OUTF: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_OUTF, token_to_regr(r->reg));
      break;
    }
    case NODE_OUTF32: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_OUTF32, token_to_regr(r->reg));
      break;
    }
    case NODE_SOUT_REG: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_INC, token_to_regr(r->reg));
      break;
    }
    case NODE_SIN_REG: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_INC, token_to_regr(r->reg));
      break;
    }
    case NODE_ADD_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_ADD_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_SUB_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_SUB_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_MUL_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_MUL_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_DIV_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_DIV_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_MOD_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_MOD_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_IADD_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_IADD_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_ISUB_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_ISUB_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_IMUL_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_IMUL_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_IDIV_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_IDIV_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_IMOD_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_IMOD_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_FADD_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_FADD, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_FSUB_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_FSUB, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_FMUL_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_FMUL, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_FDIV_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_FDIV, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_FADD32_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_FADD32, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_FSUB32_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_FSUB32, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_FMUL32_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_FMUL32, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_FDIV32_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_FDIV32, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_MOVB: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_MOVE_REG8, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_MOVW: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_MOVE_REG16, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_MOVD: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_MOVE_REG32, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_MOVQ: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_MOVE_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_MOVSXB_REG: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_MOVESX_REG8, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_MOVSXW_REG: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_MOVESX_REG16, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_MOVSXD_REG: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_MOVESX_REG32, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_EXCGB: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_EXCG8, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_EXCGW: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_EXCG16, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_EXCGD: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_EXCG32, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_EXCGQ: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_EXCG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_MOVEB: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_MOV8, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_MOVEW: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_MOV16, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_MOVED: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_MOV32, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_MOVEQ: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_MOVE_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    }
  }

  return true;
}

void masm::GPCGen::add_data(std::string value, value_t type, size_t len) {
  Data64 val;
  switch (type) {
  case VALUE_HEX: {
    val.whole_word = std::strtoull(value.c_str(), NULL, 16);
    break;
  }
  case VALUE_OCTAL: {
    val.whole_word = std::strtoull(value.c_str(), NULL, 8);
    break;
  }
  case VALUE_BINARY: {
    val.whole_word = std::strtoull(value.c_str(), NULL, 2);
    break;
  }
  case VALUE_INTEGER: {
    val.whole_word = std::strtoull(value.c_str(), NULL, 10);
    break;
  }
  case VALUE_FLOAT: {
    if (len == 8) {
      union {
        double d;
        uint64_t i;
      } fl;
      fl.d = std::strtod(value.c_str(), NULL);
      val.whole_word = fl.i;
    } else {
      union {
        float d;
        uint32_t i;
      } fl;
      fl.d = std::strtof(value.c_str(), NULL);
      val.whole_word = fl.i;
    }
    break;
  }
  case VALUE_STRING: {
    for (size_t i = 0; i < value.length(); i++)
      data.push_back(value[i]);
    return;
  }
  default:
    return;
  }
  for (size_t i = 0; i < len; i++) {
    data.push_back(val.whole_word & 255);
    val.whole_word >>= 8;
  }
}

void masm::GPCGen::add_reserved_data(std::string len, value_t type, size_t l) {
  uint64_t val;
  switch (type) {
  case VALUE_HEX: {
    val = std::strtoull(len.c_str(), NULL, 16);
    break;
  }
  case VALUE_OCTAL: {
    val = std::strtoull(len.c_str(), NULL, 8);
    break;
  }
  case VALUE_BINARY: {
    val = std::strtoull(len.c_str(), NULL, 2);
    break;
  }
  case VALUE_INTEGER: {
    val = std::strtoull(len.c_str(), NULL, 10);
    break;
  }
  default:
    return;
  }
  for (size_t i = 0; i < l * val; i++) {
    data.push_back(0);
  }
  st_address_data += l * val;
}

void masm::GPCGen::simple_instructions(uint8_t opcode) {
  Inst64 i;
  i.bytes.b0 = opcode;
  instructions.push_back(i);
}

uint8_t masm::GPCGen::token_to_regr(token_t regr) {
  switch (regr) {
  case ACC:
    return 0;
  case BP:
    return 1;
  case SP:
    return 2;
  case R0:
    return 3;
  case R1:
    return 4;
  case R2:
    return 5;
  case R3:
    return 6;
  case R4:
    return 7;
  case R5:
    return 8;
  case R6:
    return 9;
  case R7:
    return 10;
  case R8:
    return 11;
  case R9:
    return 12;
  case R10:
    return 13;
  case R11:
    return 14;
  case R12:
    return 15;
  default:
    return 0;
  }
}

void masm::GPCGen::instructions_with_single_regr(uint8_t opcode, uint8_t regr) {
  Inst64 i;
  i.bytes.b0 = opcode;
  i.bytes.b7 = regr;
  instructions.push_back(i);
}

void masm::GPCGen::instructions_with_two_regr(uint8_t opcode, uint8_t reg1,
                                              uint8_t reg2) {
  Inst64 i;
  i.bytes.b0 = opcode;
  i.bytes.b6 = reg1;
  i.bytes.b7 = reg2;
}
