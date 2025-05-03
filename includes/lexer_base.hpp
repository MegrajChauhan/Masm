#ifndef _LEXER_BASE_
#define _LEXER_BASE_

#include <string>
#include <unordered_map>

namespace masm {

enum token_t {
  TOKEN_EOF,
  TOKEN_ERROR,
  TOKEN_IDENTIFIER,
  TOKEN_INTEGER,
  TOKEN_HEX,
  TOKEN_OCTAL,
  TOKEN_BINARY,
  TOKEN_FLOAT,
  TOKEN_STRING,
  TOKEN_COLON,
  TOKEN_INCLUDE,
  TOKEN_DEFINE,
  TOKEN_DB,
  TOKEN_DW,
  TOKEN_DD,
  TOKEN_DQ,
  TOKEN_DF,
  TOKEN_DS,
  TOKEN_DLF,
  TOKEN_RESB,
  TOKEN_RESW,
  TOKEN_RESD,
  TOKEN_RESF,
  TOKEN_RESQ,
  TOKEN_RESLF,

  // Registers
  // GPC
  R0,
  R1,
  R2,
  R3,
  R4,
  R5,
  R6,
  R7,
  R8,
  R9,
  R10,
  R11,
  R12,
  SP,
  BP,

  // Instructions
  // GPC
  TOKEN_NOP,
  ACC,
  TOKEN_HALT,
  TOKEN_ADD,
  TOKEN_SUB,
  TOKEN_MUL,
  TOKEN_DIV,
  TOKEN_MOD,
  TOKEN_IADD,
  TOKEN_ISUB,
  TOKEN_IMUL,
  TOKEN_IDIV,
  TOKEN_IMOD,
  TOKEN_FADD,
  TOKEN_FUSB,
  TOKEN_FMUL,
  TOKEN_FDIV,
  TOKEN_FADD32,
  TOKEN_FSUB32,
  TOKEN_FMUL32,
  TOKEN_FDIV32,
  TOKEN_INC,
  TOKEN_DEC,
  TOKEN_MOV,
  TOKEN_MOVQ,
  TOKEN_MOVB,
  TOKEN_MOVW,
  TOKEN_MOVD,
  TOKEN_MOVF,
  TOKEN_MOVF32,
  TOKEN_MOVSXB,
  TOKEN_MOVSXW,
  TOKEN_MOVSXD,
  TOKEN_EXCGB,
  TOKEN_EXCGW,
};

struct Token {
  token_t type;
  std::string value;
  size_t line;
};

static std::unordered_map<std::string, token_t> map = {
    {"include", TOKEN_INCLUDE}, {":", TOKEN_COLON},
    {"define", TOKEN_DEFINE},   {"db", TOKEN_DB},
    {"dw", TOKEN_DW},           {"dd", TOKEN_DD},
    {"dq", TOKEN_DQ},           {"df", TOKEN_DF},
    {"ds", TOKEN_DS},           {"df", TOKEN_DF},
    {"dlf", TOKEN_DLF},         {"resb", TOKEN_RESB},
    {"resw", TOKEN_RESW},       {"resd", TOKEN_RESD},
    {"resq", TOKEN_RESQ},       {"resf", TOKEN_RESF},
    {"reslf", TOKEN_RESLF},
};

std::pair<bool, token_t> belongs_to_keymap(std::string name);
}; // namespace masm

#endif
