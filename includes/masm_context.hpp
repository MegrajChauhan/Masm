#ifndef _MASM_CONTEXT_
#define _MASM_CONTEXT_

#include <filecontext.hpp>
#include <output_gen.hpp>

// This is also responsible for parsing the input CMD arguments
namespace masm {

static std::string HELP_MSG =
    "Usage:\n"
    "masm [OPTIONS...] [ARGUMENTS...]\n"
    "Options:\n"
    "-h, --help              - Display this help message and exit\n"
    "-v, --version           - Display the Assembler version and exit\n"
    "-f                      - An Input file for assembling\n"
    "-DD                     - Display disclaimer message and exit\n"
    "-I                      - Add a new include path\n"
    "-o                      - Provide a output path along for the generated "
    "binary\n"
    "\nMasm - An assembler for the Merry Virtual Machine\n";

static std::string VERSION =
    "Merry Version: v0.0.0[no-rel,no-beta,no-alpha]\n"
    "TEST PHASE 0\n"; // Phase indicating the number of different test version

static std::string DISCLAIMER =
    "DISCLAIMER:\n"
    "Masm is an assembler for the Merry Virtual Machine "
    "with no intention of being used "
    "for building real world applications. The entire "
    "existence of Masm is to allow for "
    "testing of the Virtual Machine and its features. Any "
    "and all features are for the sole"
    " purpose of testing.\n"
    "With the documentation of the Virtual Machine, that "
    "I intend to provide, a usable assembler"
    " for real world application may be prepared.\n"
    "Masm solely exists to make sure that those useful "
    "assemblers have a dependable platform"
    " that does what it is supposed to do.\n";

class MasmContext {
  uint64_t d_address = 0;

  std::unordered_map<std::string, std::pair<value_t, std::string>> CONSTANTS;
  std::unordered_set<std::string> LABELS;
  SymbolTable symtable;
  std::unordered_map<std::string, uint64_t> label_addresses;
  std::unordered_map<std::string, uint64_t> data_addresses;
  std::vector<std::filesystem::path> include_paths;
  std::vector<uint8_t> data;
  std::vector<uint8_t> string;

  std::vector<FileContext> contexts;
  std::vector<std::string> input_files;

  std::string output_file = "./M.mbin";

  std::vector<std::string> cmd_options;

  std::unordered_set<file_t> is_already_used;

  GeneratorDetails details;

  struct {
    bool help = false, version = false;
    bool disclaimer = false;
  } CMD;

public:
  MasmContext(int, char **);

  void display_help();

  void display_disclaimer();

  void display_version();

  // preparing for assembling
  bool parse_cmd_options();

  bool prepare_for_assembling();

  // start assembling
  bool assemble();

  // pre-output generation
  bool prepare_for_emiting();

  // emit
  bool emit();
};
}; // namespace masm

#endif
