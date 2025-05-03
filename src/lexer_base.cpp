#include <lexer_base.hpp>

std::pair<bool, masm::token_t> masm::belongs_to_keymap(std::string name) {
  auto res = map.find(name);
  return std::make_pair(res != map.end(),
                        (res != map.end()) ? res->second : TOKEN_ERROR);
}
