#include <lexer.hpp>

masm::Lexer::Lexer(std::string path, bool *err) {
  std::fstream fd(path, std::ios::in);
  if (!fd.is_open()) {
    simple_message("Failed to OPEN file '%s'", path.c_str());
    *err = false;
    return;
  }
  while (!fd.eof()) {
    std::string tmp;
    std::getline(fd, tmp);
    stream += tmp + '\n';
  }

  iter = stream.begin();
  *err = true;
  fd.close();
  file = path;
  line = 1;
}

masm::Token masm::Lexer::next_token() {
  Token res;
  std::string::iterator st, ed;

  while (isspace(*iter) || *iter == ';' || *iter == ',') {
    if (*iter == ';') {
      while (iter != stream.end() && *iter != '\n')
        iter++;
    } else {
      if (*iter == '\n')
        line++;
      iter++;
    }
  }
  res.line = line;
  if (iter == stream.end()) {
    res.type = TOKEN_EOF;
  } else if (isalpha(*iter) || *iter == '_') {
    st = iter;
    res.type = lex_identifier_or_token();
    ed = iter;
    if (res.type == TOKEN_IDENTIFIER)
      res.value = std::string(st, ed);
  } else if (*iter == '-' || (*iter >= '0' && *iter <= '9')) {
    st = iter;
    if (*iter == '-')
      iter++;
    std::pair<bool, token_t> num = lex_number();
    if (!num.first) {
      res.type = TOKEN_ERROR;
    } else {
      res.type = num.second;
      ed = iter;
      res.value = std::string(st, ed);
    }
  } else if (*iter == '"') {
    std::pair<bool, std::string> str = lex_string();
    if (!str.first) {
      res.type = TOKEN_ERROR;
    } else {
      res.type = TOKEN_STRING;
      res.value = str.second;
    }
  } else {
    // could be an operator
    st = iter;
    std::pair<bool, token_t> oper = lex_operators();
    if (!oper.first) {
      detailed_message(file.c_str(), line,
                       "Couldn't build a token from this '%c'.", *iter);
      res.type = TOKEN_ERROR;
    } else {
      res.type = oper.second;
      ed = iter;
    }
  }

  return res;
}

masm::Token masm::Lexer::peek_token() {
  std::string::iterator curr = iter;
  size_t l = line;
  Token res = next_token();
  iter = curr;
  line = l;
  return res;
}

std::pair<bool, masm::token_t> masm::Lexer::lex_number() {
  // 0x for hex, 0o for octal, 0b for binary
  if (*iter == '0' && (iter + 1) != stream.end() && isalpha(*(iter + 1))) {
    iter++;
    switch (*iter) {
    case 'x':
      return (is_hexadecimal()) ? std::make_pair(true, TOKEN_HEX)
                                : std::make_pair(false, TOKEN_ERROR);
    case 'o':
      return (is_octal()) ? std::make_pair(true, TOKEN_OCTAL)
                          : std::make_pair(false, TOKEN_ERROR);
    case 'b':
      return (is_binary()) ? std::make_pair(true, TOKEN_BINARY)
                           : std::make_pair(false, TOKEN_ERROR);
    default:
      if (is_an_integer()) {
        return std::make_pair(true, TOKEN_INTEGER);
      } else if (is_float_or_double())
        return std::make_pair(true, TOKEN_FLOAT);
      else {
        detailed_message(file.c_str(), line,
                         "Unknown NUMBER TYPE: %c. Use 'x', 'o' or 'b'.",
                         *iter);
        return std::make_pair(false, TOKEN_ERROR);
      }
    }
  } else if (is_an_integer())
    return std::make_pair(true, TOKEN_INTEGER);
  else if (is_float_or_double())
    return std::make_pair(true, TOKEN_FLOAT);
  return std::make_pair(false, TOKEN_ERROR);
}

bool masm::Lexer::is_an_integer() {
  while (iter != stream.end() && *iter >= '0' && *iter <= '9')
    iter++;
  return true;
}

bool masm::Lexer::is_hexadecimal() {
  while (iter != stream.end() &&
         ((*iter >= '0' && *iter <= '9') || (*iter >= 'a' && *iter <= 'f')))
    iter++;
  return true;
}

bool masm::Lexer::is_octal() {
  while (iter != stream.end() && *iter >= '0' && *iter <= '7')
    iter++;
  return true;
}

bool masm::Lexer::is_binary() {
  while (iter != stream.end() && (*iter == '0' || *iter == '1'))
    iter++;
  return true;
}

bool masm::Lexer::is_float_or_double() {
  size_t dot_count = 0;
  while (iter != stream.end() &&
         ((*iter >= '0' && *iter <= '9') || *iter == '.')) {
    if (*iter == '.' && dot_count > 1) {
      detailed_message(file.c_str(), line,
                       "Invalid Floating point number. More than one '%c'",
                       *iter);
      return false;
    } else {
      dot_count++;
    }
    iter++;
  }
  return true;
}

std::pair<bool, std::string> masm::Lexer::lex_string() {
  std::string str;
  iter++;
  bool end_found = false;
  while (true) {
    if (iter == stream.end())
      break;
    if (*iter == '\n')
      line++;
    if (*iter == '"') {
      end_found = true;
      break;
    } else {
      if (*iter == '\\') {
        iter++;
        if (iter == stream.end()) {
          detailed_message(
              file.c_str(), line,
              "Expected something after the escape sequence. Got nothing!",
              NULL);
          break;
        }
        switch (*iter) {
        case 'n':
          str += '\n';
          break;
        case 'r':
          str += '\r';
          break;
        case 't':
          str += '\t';
          break;
        case '\\':
          str += '\\';
          break;
        case '0':
          str += '\0';
          break;
        default:
          str += '\\';
          str += *iter;
          break;
        }
      } else {
        str += *iter;
      }
      iter++;
    }
  }
  if (!end_found) {
    detailed_message(file.c_str(), line,
                     "Expected STRING Termination but didn't found one.", NULL);
    return std::make_pair(false, std::string());
  }
  iter++;
  return std::make_pair(true, str);
}

masm::token_t masm::Lexer::lex_identifier_or_token() {
  token_t res;
  std::string::iterator st = iter, ed;

  while (iter != stream.end() &&
         (isalpha(*iter) || *iter == '_' || (*iter >= '0' && *iter <= '9'))) {
    iter++;
  }
  ed = iter;
  std::string tok(st, ed);
  std::pair<bool, token_t> lookup_res = belongs_to_keymap(tok);
  if (lookup_res.first)
    res = lookup_res.second;
  else
    res = TOKEN_IDENTIFIER;

  return res;
}

std::pair<bool, masm::token_t> masm::Lexer::lex_operators() {
  std::pair<bool, token_t> res;
  res.first = true;
  switch (*iter) {
  case ':':
    res.second = TOKEN_COLON;
    break;
  default:
    res.first = false;
    iter--;
  }
  iter++;
  return res;
}
