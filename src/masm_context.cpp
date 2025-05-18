#include <masm_context.hpp>

masm::MasmContext::MasmContext(int argc, char **argv) {
  for (size_t i = 0; i < (size_t)argc; i++) {
    cmd_options.push_back(std::string(argv[i]));
  }
}

bool masm::MasmContext::parse_cmd_options() {
  for (size_t i = 0; i < cmd_options.size(); i++) {
    if (cmd_options[i] == "-h" || cmd_options[i] == "--help") {
      CMD.help = true;
      break;
    } else if (cmd_options[i] == "-v" || cmd_options[i] == "--version") {
      CMD.version = true;
      break;
    } else if (cmd_options[i] == "-DD") {
      CMD.disclaimer = true;
      break;
    } else if (cmd_options[i] == "-f") {
      if (!((i + 1) < cmd_options.size())) {
        simple_message("Expected file path after -f but got EOF.", NULL);
        return false;
      }
      i++;
      input_files.push_back(cmd_options[i]);
    } else if (cmd_options[i] == "-I") {
      if (!((i + 1) < cmd_options.size())) {
        simple_message("Expected include path after -I but got EOF.", NULL);
        return false;
      }
      i++;
      include_paths.push_back(cmd_options[i]);
    } else {
      simple_message("Unknown Option: %s", cmd_options[i].c_str());
      return false;
    }
  }
  return true;
}

void masm::MasmContext::display_disclaimer() {
  simple_message("%s", DISCLAIMER.c_str());
}

void masm::MasmContext::display_help() {
  simple_message("%s", HELP_MSG.c_str());
}

void masm::MasmContext::display_version() {
  simple_message("%s", VERSION.c_str());
}

bool masm::MasmContext::prepare_for_assembling() {
  include_paths.push_back("./");
  // more for the standard library
  // the first paths get precendance over the later ones

  if (!parse_cmd_options()) {
    display_help();
    return false;
  }

  if (CMD.help) {
    display_help();
    exit(0);
  }

  if (CMD.version) {
    display_version();
    exit(0);
  }

  if (CMD.disclaimer) {
    display_disclaimer();
    exit(0);
  }

  if (input_files.size() == 0) {
    simple_message("ERROR: No Input File provided", NULL);
    display_help();
    return false;
  }

  // We initialize all FileContext here
  for (auto path : input_files) {
    FileContext cont(include_paths, CONSTANTS, LABELS, symtable,
                     label_addresses, data_addresses, data, string, 0);

    if (!cont.file_prepare(path) || !cont.should_process_file())
      return false;
    contexts.push_back(std::move(cont));
  }
  return true;
}

bool masm::MasmContext::assemble() {
  // We will have to loop through quite a lot of times here
  // First step is to do parse
  for (FileContext &c : contexts) {
    if (!c.parse_file() || !c.pre_analysis())
      return false;
  }

  // Analyze one by one
  // First finish the first step for the variables, labels and constants
  for (FileContext &c : contexts) {
    if (!c.analyze_file_first_step())
      return false;
  }

  // For pointers specifically
  for (FileContext &c : contexts) {
    if (!c.analyze_file_first_step_second_phase())
      return false;
  }

  // Time for instructions
  for (FileContext &c : contexts) {
    if (!c.analyze_file_second_step())
      return false;
  }

  // Generating variables
  for (FileContext &c : contexts) {
    if (!c.gen_file_first_step())
      return false;
  }

  // Generating strings
  for (FileContext &c : contexts) {
    if (!c.gen_file_first_step_second_phase())
  }

  return true;
}
