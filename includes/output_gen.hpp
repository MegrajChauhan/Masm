#ifndef _OUTPUT_GEN_
#define _OUTPUT_GEN_

#include <consts.hpp>
#include <filesystem>
#include <fstream>
#include <gen_base.hpp>
#include <utils.hpp>
#include <vector>

#define ITIT_HEADER_LEN 16
#define DATA_HEADER_LEN 8
#define STRING_HEADER_LEN 8
#define PAGE_LEN 1048576
#define PAGE_LEN_BYTES 1048576

namespace masm {
struct GeneratorDetails {
  std::string magic = "beb"; // bROADLY eMTTED bINARY
  output_file_t type = NORMAL_FILE_TYPE;
  size_t number_of_different_ISA_used; // FOR ITIT
  size_t data_section_length;
  size_t string_section_length;
  size_t DIT_len = 0; // For proper Assemblers
  std::vector<std::pair<file_t, std::vector<Inst64>>> instructions;
  std::vector<uint8_t> data;
  std::vector<uint8_t> string;

  Inst64 entry_inst;
  std::string output_file_path;
};

class Generator {
  std::fstream file;
  GeneratorDetails &details;

public:
  Generator(GeneratorDetails &);

  bool pre_emission();

  bool emit_header();

  bool emit_ITIT();

  bool emit_Instructions();

  bool emit_data_section();

  bool emit_string_section();
};

}; // namespace masm

#endif
