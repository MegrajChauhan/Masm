#include <gpc_gen.hpp>

masm::GPCGen::GPCGen(SymbolTable &t,
                     std::unordered_map<std::string, uint64_t> &laddr,
                     std::unordered_map<std::string, uint64_t> &daddr,
                     std::vector<uint8_t> &d, std::vector<uint8_t> &s,
                     uint64_t a)
    : symtable(t), st_address_data(a), label_addresses(laddr),
      data_addresses(daddr), data(d), string(s) {}

uint64_t masm::GPCGen::get_current_address_point() { return st_address_data; }

std::vector<masm::Inst64> masm::GPCGen::get_instructions() {
  return std::move(instructions);
}

void masm::GPCGen::set_final_nodes(std::vector<Node> &&nodes) {
  final_nodes = std::move(nodes);
}

std::vector<uint8_t> masm::GPCGen::get_data() { return std::move(data); }

masm::Inst64 masm::GPCGen::get_ENTRY_INSTRUCTION(size_t addr) {
  Inst64 i;
  i.whole_word = (addr & 0xFFFFFFFFFFFF) - 8;
  i.bytes.b0 = OP_JMP_ADDR;
  return i;
}

bool masm::GPCGen::first_iteration(uint64_t addr_point) {
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

  st_address_data = addr_point;
  uint64_t i = 8;
  for (Node &n : final_nodes) {
    switch (n.type) {
    case NODE_LABEL: {
      NodeLabel *lbl = (NodeLabel *)n.node.get();
      label_addresses[lbl->name] = i;
      break;
    }
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
    case NODE_DP: {
      NodeDP *dp = (NodeDP *)n.node.get();
      data_addresses[dp->name] = st_address_data;
      add_data(dp->value, dp->type, 0);
      st_address_data += 8;
      break;
    }
    case NODE_DS:
      break;
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
    case NODE_RESP:
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
    default:
      i += n.len * 8;
    }
  }
  return true;
}

bool masm::GPCGen::first_iteration_second_phase(uint64_t addr_point) {
  st_address_data = addr_point;
  for (auto &n : final_nodes) {
    if (n.type == NODE_DS) {
      NodeDS *ds = (NodeDS *)n.node.get();
      data_addresses[ds->name] = st_address_data;
      add_data(ds->value, ds->type, 0);
      st_address_data += ds->value.length();
    }
  }
  return true;
}

bool masm::GPCGen::first_iteration_third_phase(uint64_t addr_point) {
  st_address_data = addr_point;
  for (Node &n : final_nodes) {
    if (n.type == NODE_DP) {
      NodeDP *dp = (NodeDP *)n.node.get();
      Data64 d;
      d.whole_word = data_addresses[dp->value];
      size_t this_ptr = data_addresses[dp->name];
      data[this_ptr] = d.bytes.b7;
      data[this_ptr + 1] = d.bytes.b6;
      data[this_ptr + 2] = d.bytes.b5;
      data[this_ptr + 3] = d.bytes.b4;
      data[this_ptr + 4] = d.bytes.b3;
      data[this_ptr + 5] = d.bytes.b2;
      data[this_ptr + 6] = d.bytes.b1;
      data[this_ptr + 7] = d.bytes.b0;
    }
  }
  return true;
}

bool masm::GPCGen::second_iteration() {
  // Now we can go through the nodes and generate Instructions
  // First instruction is actually a jump instruction to the
  // entry procedure named main which must be defined
  // otherwise there will be errors.

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
    case NODE_DP:
    case NODE_DS:
    case NODE_DF:
    case NODE_DLF:
    case NODE_RESB:
    case NODE_RESW:
    case NODE_RESD:
    case NODE_RESQ:
    case NODE_RESP:
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
      instructions_with_single_regr(OP_INQ, token_to_regr(r->reg));
      break;
    }
    case NODE_OUTQ: {
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
      instructions_with_single_regr(OP_SOUT_REG, token_to_regr(r->reg));
      break;
    }
    case NODE_SIN_REG: {
      NodeReg *r = (NodeReg *)n.node.get();
      instructions_with_single_regr(OP_SIN_REG, token_to_regr(r->reg));
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
    case NODE_AND_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_AND_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_OR_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_OR_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_XOR_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_XOR_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_SHL_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_LSHIFT_REGR, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_SHR_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_RSHIFT_REGR, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_FCMP: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_FCMP, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_FCMP32: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_FCMP32, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_LOADB_REG: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_LOADB_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_LOADW_REG: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_LOADW_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_LOADD_REG: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_LOADD_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_LOADQ_REG: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_LOADQ_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_STOREB_REG: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_STOREB_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_STOREW_REG: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_STOREW_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_STORED_REG: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_STORED_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_STOREQ_REG: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_STOREQ_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_CMP_REGR: {
      NodeRegReg *rr = (NodeRegReg *)n.node.get();
      instructions_with_two_regr(OP_CMP_REG, token_to_regr(rr->r1),
                                 token_to_regr(rr->r2));
      break;
    }
    case NODE_JMP_IMM: {
      instructions_with_one_immediate(OP_JMP_ADDR, n, 0, true, true);
      break;
    }
    case NODE_WHDLR: {
      NodeImm *imm = (NodeImm *)n.node.get();
      auto label = label_addresses.find(imm->imm);
      Inst64 i;
      i.bytes.b0 = OP_WHDLR;
      instructions.push_back(i);
      i.whole_word = label->second;
      instructions.push_back(i);
      break;
    }
    case NODE_CALL_IMM: {
      instructions_with_one_immediate(OP_CALL, n, 0, true);
      break;
    }
    case NODE_JNZ: {
      instructions_with_one_immediate(OP_JNZ, n, 0, true, true);
      break;
    }
    case NODE_JZ: {
      instructions_with_one_immediate(OP_JZ, n, 0, true, true);
      break;
    }
    case NODE_JNE: {
      instructions_with_one_immediate(OP_JNE, n, 0, true, true);
      break;
    }
    case NODE_JE: {
      instructions_with_one_immediate(OP_JE, n, 0, true, true);
      break;
    }
    case NODE_JNC: {
      instructions_with_one_immediate(OP_JNC, n, 0, true, true);
      break;
    }
    case NODE_JC: {
      instructions_with_one_immediate(OP_JC, n, 0, true, true);
      break;
    }
    case NODE_JNO: {
      instructions_with_one_immediate(OP_JNO, n, 0, true, true);
      break;
    }
    case NODE_JO: {
      instructions_with_one_immediate(OP_JO, n, 0, true, true);
      break;
    }
    case NODE_JNN: {
      instructions_with_one_immediate(OP_JNN, n, 0, true, true);
      break;
    }
    case NODE_JN: {
      instructions_with_one_immediate(OP_JN, n, 0, true, true);
      break;
    }
    case NODE_JNG: {
      instructions_with_one_immediate(OP_JNG, n, 0, true, true);
      break;
    }
    case NODE_JG: {
      instructions_with_one_immediate(OP_JG, n, 0, true, true);
      break;
    }
    case NODE_JNS: {
      instructions_with_one_immediate(OP_JNS, n, 0, true, true);
      break;
    }
    case NODE_JS: {
      instructions_with_one_immediate(OP_JS, n, 0, true, true);
      break;
    }
    case NODE_JGE: {
      instructions_with_one_immediate(OP_JGE, n, 0, true, true);
      break;
    }
    case NODE_JSE: {
      instructions_with_one_immediate(OP_JSE, n, 0, true, true);
      break;
    }
    case NODE_PUSHB: {
      NodeImm *imm = (NodeImm *)n.node.get();
      if (imm->is_var)
        single_operand_which_is_variable(OP_PUSH_MEMB, imm->imm);
      else
        single_operand_which_is_immediate(OP_PUSH_IMM8, imm->imm, imm->type, 1);
      break;
    }
    case NODE_PUSHW: {
      NodeImm *imm = (NodeImm *)n.node.get();
      if (imm->is_var)
        single_operand_which_is_variable(OP_PUSH_MEMW, imm->imm);
      else
        single_operand_which_is_immediate(OP_PUSH_IMM16, imm->imm, imm->type,
                                          2);
      break;
    }
    case NODE_PUSHD: {
      NodeImm *imm = (NodeImm *)n.node.get();
      if (imm->is_var)
        single_operand_which_is_variable(OP_PUSH_MEMD, imm->imm);
      else
        single_operand_which_is_immediate(OP_PUSH_IMM32, imm->imm, imm->type,
                                          4);
      break;
    }
    case NODE_PUSHQ: {
      NodeImm *imm = (NodeImm *)n.node.get();
      if (imm->is_var)
        single_operand_which_is_variable(OP_PUSH_MEMQ, imm->imm);
      else
        single_operand_which_is_immediate(OP_PUSH_IMM64, imm->imm, imm->type,
                                          8);
      break;
    }
    case NODE_POPB_IMM: {
      NodeImm *imm = (NodeImm *)n.node.get();
      single_operand_which_is_variable(OP_POP_MEMB, imm->imm);
      break;
    }
    case NODE_POPW_IMM: {
      NodeImm *imm = (NodeImm *)n.node.get();
      single_operand_which_is_variable(OP_POP_MEMW, imm->imm);
      break;
    }
    case NODE_POPD_IMM: {
      NodeImm *imm = (NodeImm *)n.node.get();
      single_operand_which_is_variable(OP_POP_MEMD, imm->imm);
      break;
    }
    case NODE_POPQ_IMM: {
      NodeImm *imm = (NodeImm *)n.node.get();
      single_operand_which_is_variable(OP_POP_MEMQ, imm->imm);
      break;
    }
    case NODE_SIN_IMM:
    case NODE_SOUT_IMM:
      sin_and_sout_instructions(n);
      break;
      // INST WITH ONE REGR AND IMMEDIATE
    case NODE_ADD_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      if (imm->is_var)
        choose_opcode_according_to_variable(
            imm, {OP_ADD_MEMB, OP_ADD_MEMW, OP_ADD_MEMD, OP_ADD_MEMQ});
      else
        two_operand_second_is_immediate(OP_ADD_IMM, imm->immediate, imm->type,
                                        8, imm->regr);
      break;
    }
    case NODE_SUB_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      if (imm->is_var)
        choose_opcode_according_to_variable(
            imm, {OP_SUB_MEMB, OP_SUB_MEMW, OP_SUB_MEMD, OP_SUB_MEMQ});
      else
        two_operand_second_is_immediate(OP_SUB_IMM, imm->immediate, imm->type,
                                        8, imm->regr);
      break;
    }
    case NODE_MUL_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      if (imm->is_var)
        choose_opcode_according_to_variable(
            imm, {OP_MUL_MEMB, OP_MUL_MEMW, OP_MUL_MEMD, OP_MUL_MEMQ});
      else
        two_operand_second_is_immediate(OP_MUL_IMM, imm->immediate, imm->type,
                                        8, imm->regr);
      break;
    }
    case NODE_DIV_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      if (imm->is_var)
        choose_opcode_according_to_variable(
            imm, {OP_DIV_MEMB, OP_DIV_MEMW, OP_DIV_MEMD, OP_DIV_MEMQ});
      else
        two_operand_second_is_immediate(OP_DIV_IMM, imm->immediate, imm->type,
                                        8, imm->regr);
      break;
    }
    case NODE_MOD_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      if (imm->is_var)
        choose_opcode_according_to_variable(
            imm, {OP_MOD_MEMB, OP_MOD_MEMW, OP_MOD_MEMD, OP_MOD_MEMQ});
      else
        two_operand_second_is_immediate(OP_MOD_IMM, imm->immediate, imm->type,
                                        8, imm->regr);
      break;
    }
    case NODE_IADD_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_IADD_IMM, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_ISUB_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_ISUB_IMM, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_IMUL_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_IMUL_IMM, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_IDIV_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_IDIV_IMM, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_IMOD_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_IMOD_IMM, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_FADD32_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {OP_FADD32_MEM});
      break;
    }
    case NODE_FSUB32_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {OP_FSUB32_MEM});
      break;
    }
    case NODE_FMUL32_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {OP_FMUL32_MEM});
      break;
    }
    case NODE_FDIV32_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {OP_FDIV32_MEM});
      break;
    }
    case NODE_FADD_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {OP_FADD_MEM});
      break;
    }
    case NODE_FSUB_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {OP_FSUB_MEM});
      break;
    }
    case NODE_FMUL_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {OP_FMUL_MEM});
      break;
    }
    case NODE_FDIV_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {OP_FDIV_MEM});
      break;
    }
    case NODE_MOV: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVE_IMM_64, imm->immediate, imm->type,
                                      8, imm->regr);
      break;
    }
    case NODE_MOVF32: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVE_IMM_64, imm->immediate, imm->type,
                                      4, imm->regr);
      break;
    }
    case NODE_MOVF: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVE_IMM_64, imm->immediate, imm->type,
                                      8, imm->regr);
      break;
    }
    case NODE_MOVSXB_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate_in_same_qword(
          OP_MOVESX_IMM8, imm->regr, imm->immediate, imm->type, 1);
      break;
    }
    case NODE_MOVSXW_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate_in_same_qword(
          OP_MOVESX_IMM16, imm->regr, imm->immediate, imm->type, 2);
      break;
    }
    case NODE_MOVSXD_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate_in_same_qword(
          OP_MOVESX_IMM32, imm->regr, imm->immediate, imm->type, 4);
      break;
    }
    case NODE_MOVNZ: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVNZ, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_MOVZ: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVZ, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_MOVNE: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVNE, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_MOVE: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVE, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_MOVNC: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVNC, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_MOVC: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVC, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_MOVNO: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVNO, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_MOVO: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVO, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_MOVNN: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVNN, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_MOVN: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVN, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_MOVNG: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVNG, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_MOVG: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVG, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_MOVNS: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVNS, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_MOVS: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVS, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_MOVGE: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVGE, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_MOVSE: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_MOVSE, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_LOOP: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      auto label = label_addresses.find(imm->immediate);
      Inst64 i;
      i.bytes.b0 = OP_LOOP;
      i.bytes.b1 = token_to_regr(imm->regr);
      i.whole_word |= ((label->second & 0xFFFFFFFFFFFF));
      instructions.push_back(i);
      break;
    }
    case NODE_INT: {
      NodeImm *imm = (NodeImm *)n.node.get();
      single_operand_which_is_immediate(OP_INTR, imm->imm, imm->type, 2);
      auto i = *instructions.end();
      instructions.pop_back();
      instructions.pop_back();
      i.bytes.b0 = OP_INTR;
      instructions.push_back(i);
      break;
    }
    case NODE_LOADSB: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate_in_same_qword(
          OP_LOADSB, imm->regr, imm->immediate, imm->type, 2);
      break;
    }
    case NODE_LOADSW: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate_in_same_qword(
          OP_LOADSW, imm->regr, imm->immediate, imm->type, 2);
      break;
    }
    case NODE_LOADSD: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate_in_same_qword(
          OP_LOADSD, imm->regr, imm->immediate, imm->type, 2);
      break;
    }
    case NODE_LOADSQ: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate_in_same_qword(
          OP_LOADSQ, imm->regr, imm->immediate, imm->type, 2);
      break;
    }
    case NODE_STORESB: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate_in_same_qword(
          OP_STORESB, imm->regr, imm->immediate, imm->type, 2);
      break;
    }
    case NODE_STORESW: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate_in_same_qword(
          OP_STORESW, imm->regr, imm->immediate, imm->type, 2);
      break;
    }
    case NODE_STORESD: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate_in_same_qword(
          OP_STORESD, imm->regr, imm->immediate, imm->type, 2);
      break;
    }
    case NODE_STORESQ: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate_in_same_qword(
          OP_STORESQ, imm->regr, imm->immediate, imm->type, 2);
      break;
    }
    case NODE_AND_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_AND_IMM, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_OR_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_OR_IMM, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_XOR_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate(OP_XOR_IMM, imm->immediate, imm->type, 8,
                                      imm->regr);
      break;
    }
    case NODE_SHL_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate_in_same_qword(
          OP_LSHIFT, imm->regr, imm->immediate, imm->type, 1);
      break;
    }
    case NODE_SHR_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      two_operand_second_is_immediate_in_same_qword(
          OP_RSHIFT, imm->regr, imm->immediate, imm->type, 1);
      break;
    }
    case NODE_CMP_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      if (imm->is_var)
        choose_opcode_according_to_variable(imm,
                                            {OP_CMP_IMM_MEMB, OP_CMP_IMM_MEMW,
                                             OP_CMP_IMM_MEMD, OP_CMP_IMM_MEMQ});
      else
        two_operand_second_is_immediate(OP_CMP_IMM, imm->immediate, imm->type,
                                        8, imm->regr);
      break;
    }
    case NODE_LOADB_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {OP_LOADB});
      break;
    }
    case NODE_LOADW_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {0, OP_LOADW});
      break;
    }
    case NODE_LOADD_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {0, 0, OP_LOADD});
      break;
    }
    case NODE_LOADQ_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {0, 0, 0, OP_LOADQ});
      break;
    }
    case NODE_STOREB_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {OP_STOREB});
      break;
    }
    case NODE_STOREW_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {0, OP_STOREW});
      break;
    }
    case NODE_STORED_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {0, 0, OP_STORED});
      break;
    }
    case NODE_STOREQ_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {0, 0, 0, OP_STOREQ});
      break;
    }
    case NODE_ATM_LOADB_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {OP_ATOMIC_LOADB});
      break;
    }
    case NODE_ATM_LOADW_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {0, OP_ATOMIC_LOADW});
      break;
    }
    case NODE_ATM_LOADD_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {0, 0, OP_ATOMIC_LOADD});
      break;
    }
    case NODE_ATM_LOADQ_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {0, 0, 0, OP_ATOMIC_LOADQ});
      break;
    }
    case NODE_ATM_STOREB_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {OP_ATOMIC_STOREB});
      break;
    }
    case NODE_ATM_STOREW_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {0, OP_ATOMIC_STOREW});
      break;
    }
    case NODE_ATM_STORED_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {0, 0, OP_ATOMIC_STORED});
      break;
    }
    case NODE_ATM_STOREQ_IMM: {
      NodeRegrImm *imm = (NodeRegrImm *)n.node.get();
      choose_opcode_according_to_variable(imm, {0, 0, 0, OP_ATOMIC_STOREQ});
      break;
    }
    case NODE_LEA: {
      NodeLea *l = (NodeLea *)n.node.get();
      Inst64 i;
      i.bytes.b0 = OP_LEA;
      i.bytes.b4 = token_to_regr(l->r1);
      i.bytes.b5 = token_to_regr(l->r2);
      i.bytes.b6 = token_to_regr(l->r3);
      i.bytes.b7 = token_to_regr(l->r4);
      instructions.push_back(i);
      break;
    }
    case NODE_CMPXCHG_IMM: {
      NodeCMPXCHGImm *ci = (NodeCMPXCHGImm *)n.node.get();
      auto data = data_addresses.find(ci->imm);
      Inst64 i;
      i.bytes.b0 = OP_CMPXCHG;
      i.bytes.b6 = token_to_regr(ci->r1);
      i.bytes.b7 = token_to_regr(ci->r2);
      instructions.push_back(i);
      i.whole_word = data->second;
      instructions.push_back(i);
      break;
    }
    case NODE_CMPXCHG_REG: {
      NodeCMPXCHGReg *ci = (NodeCMPXCHGReg *)n.node.get();
      Inst64 i;
      i.bytes.b0 = OP_CMPXCHG_REGR;
      i.bytes.b6 = token_to_regr(ci->r1);
      i.bytes.b7 = token_to_regr(ci->r2);
      i.bytes.b5 = token_to_regr(ci->r3);
      instructions.push_back(i);
      break;
    }
    default:
      simple_message("Unknown NODE %u %zu", n.type, n.line);
      break;
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
    if (len == 0) {
      val.whole_word = 0;
      len = 8;
      break;
    }
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
      string.push_back(value[i]);
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
  instructions.push_back(i);
}

void masm::GPCGen::instructions_with_one_immediate(uint8_t opcode, Node &n,
                                                   size_t len, bool label,
                                                   uint8_t op2, bool jmp) {
  NodeImm *imm = (NodeImm *)n.node.get();
  Inst64 inst;
  inst.bytes.b0 = opcode;
  if (imm->is_var) {
    std::unordered_map<std::string, uint64_t>::iterator i;
    if (!label) {
      i = data_addresses.find(imm->imm);
      // i should be valid since analyzer already analyzed the
    } else {
      i = label_addresses.find(imm->imm);
    }
    inst.whole_word |= ((i->second & 0xFFFFFFFFFFFF) - ((jmp) ? 8 : 0));
  } else {
    inst.bytes.b0 = op2;
    instructions.push_back(inst);
    Inst64 val;
    std::string value = imm->imm;
    switch (imm->type) {
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
    }
    inst = val;
  }
  instructions.push_back(inst);
}

void masm::GPCGen::sin_and_sout_instructions(Node &n) {
  NodeImm *i = (NodeImm *)n.node.get();
  uint8_t op = n.type == NODE_SIN_IMM ? OP_SIN : OP_SOUT;
  single_operand_which_is_variable(op, i->imm);
}

void masm::GPCGen::single_operand_which_is_variable(uint8_t opcode,
                                                    std::string name) {
  auto var = data_addresses.find(name);
  if (var == data_addresses.end()) {
    var = label_addresses.find(name); // This shouldn't fail
  }
  Inst64 i;
  i.bytes.b0 = opcode;
  i.whole_word |= ((var->second & 0xFFFFFFFFFFFF));
  instructions.push_back(i);
}

void masm::GPCGen::single_operand_which_is_immediate(uint8_t opcode,
                                                     std::string value,
                                                     value_t type, size_t len) {
  Inst64 val;
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
  default:
    return;
  }
  Inst64 i;
  i.bytes.b0 = opcode;
  instructions.push_back(i);
  instructions.push_back(val);
}

void masm::GPCGen::two_operand_second_is_immediate(uint8_t opcode,
                                                   std::string value,
                                                   value_t type, size_t len,
                                                   token_t reg1) {
  Inst64 val;
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
  default:
    return;
  }
  Inst64 i;
  i.bytes.b0 = opcode;
  i.bytes.b7 = token_to_regr(reg1);
  instructions.push_back(i);
  instructions.push_back(val);
}

void masm::GPCGen::choose_opcode_according_to_variable(
    NodeRegrImm *n, std::vector<uint8_t> opcodes) {
  std::unordered_map<std::string, Symbol>::iterator iter =
      symtable.find_symbol(n->immediate);
  auto address = data_addresses.find(n->immediate);
  Inst64 i;
  i.bytes.b1 = token_to_regr(n->regr);
  switch (iter->second.type) {
  case BYTE:
  case STRING:
  case RESB: {
    i.bytes.b0 = opcodes[0];
    break;
  }
  case WORD:
  case RESW: {
    i.bytes.b0 = opcodes[1];
    break;
  }
  case DWORD:
  case RESD: {
    i.bytes.b0 = opcodes[2];
    break;
  }
  case QWORD:
  case POINTER:
  case RESQ: {
    i.bytes.b0 = opcodes[3];
    break;
  }
  case FLOAT: {
    i.bytes.b0 = opcodes[0];
  }
  default:
    break;
  }
  i.whole_word |= ((address->second & 0xFFFFFFFFFFFF));
  instructions.push_back(i);
}

void masm::GPCGen::two_operand_second_is_immediate_in_same_qword(
    uint8_t opcode, token_t regr, std::string imm, value_t type, size_t len) {
  Inst64 i;
  Data64 val;
  i.bytes.b0 = opcode;
  i.bytes.b1 = token_to_regr(regr);
  switch (type) {
  case VALUE_HEX: {
    val.whole_word = std::strtoull(imm.c_str(), NULL, 16);
    break;
  }
  case VALUE_OCTAL: {
    val.whole_word = std::strtoull(imm.c_str(), NULL, 8);
    break;
  }
  case VALUE_BINARY: {
    val.whole_word = std::strtoull(imm.c_str(), NULL, 2);
    break;
  }
  case VALUE_INTEGER: {
    val.whole_word = std::strtoull(imm.c_str(), NULL, 10);
    break;
  }
  default:
    return;
  }
  i.half_words.w1 = val.whole_word & 0xFFFF;
  instructions.push_back(i);
}
