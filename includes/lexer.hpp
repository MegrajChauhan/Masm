#ifndef _LEXER_
#define _LEXER_

#include <fstream>
#include <lexer_base.hpp>
#include <string>
#include <utils.hpp>

namespace masm {
class Lexer {
  std::string stream;
  std::string::iterator iter;
  size_t line = 0;
  std::string file;

public:
  Lexer(std::string path, bool *err);

  Token next_token();

  Token peek_token();

  /* Lexing numbers */
  std::pair<bool, token_t> lex_number();

  bool is_an_integer();

  bool is_hexadecimal();

  bool is_octal();

  bool is_binary();

  bool is_float_or_double();

  /* Lexing Strings */
  std::pair<bool, std::string> lex_string();

  /* Lexing identifiers and keywords */
  token_t lex_identifier_or_token();

  /* Lexing operators */
  std::pair<bool, token_t> lex_operators();
};
}; // namespace masm

#endif
