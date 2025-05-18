#ifndef _OUTPUT_GEN_
#define _OUTPUT_GEN_

#include <consts.hpp>
#include <fstream>
#include <utils.hpp>

namespace masm {
struct GeneratorDetails {
  std::string magic = "beb";
  output_file_t type = NORMAL_FILE_TYPE;
  size_t number_of_different_ISA;
};

class Generator {};
}; // namespace masm

#endif
