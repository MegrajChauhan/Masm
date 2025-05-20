#include <masm_context.hpp>

masm::MasmContext::MasmContext(int argc, char **argv) {
  for (size_t i = 1; i < (size_t)argc; i++) {
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
    } else if (cmd_options[i] == "-o") {
      if (!((i + 1) < cmd_options.size())) {
        simple_message("Expected output path after -o but got EOF.", NULL);
        return false;
      }
      i++;
      output_file = cmd_options[i];
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
    if (is_already_used.find(cont.get_file_type()) != is_already_used.end()) {
      simple_message(
          "Multiple input files of the same type cannot be provided.", NULL);
      return false;
    }
    is_already_used.insert(cont.get_file_type());
    contexts.push_back(std::move(cont));
  }
  return true;
}

bool masm::MasmContext::assemble() {
  // We will have to loop through file contexts quite a lot of times here
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
    if (!c.gen_file_first_step(d_address))
      return false;
    d_address = c.get_d_addr();
  }

  // Generating strings
  for (FileContext &c : contexts) {
    if (!c.gen_file_first_step_second_phase(d_address))
      return false;
    d_address = c.get_d_addr();
  }

  // For pointers
  for (FileContext &c : contexts) {
    if (!c.gen_file_first_step_third_phase(d_address))
      return false;
    d_address = c.get_d_addr();
  }

  // For Instructions
  for (FileContext &c : contexts) {
    if (!c.gen_file_second_step())
      return false;
  }

  return true;
}

bool masm::MasmContext::prepare_for_emiting() {
  details.data = data;
  details.output_file_path = output_file;
  details.string = string;

  for (auto &cont : contexts) {
    details.instructions.push_back(
        std::make_pair(cont.get_file_type(), cont.get_instructions()));
  }

  auto main_proc = label_addresses.find("main");
  if (main_proc == label_addresses.end()) {
    simple_message(
        "Entry PROC not found. Expected a main procedure to be defined.", NULL);
    return false;
  }

  details.entry_inst = contexts[0].get_ENTRY_INSTRUCTION(main_proc->second);
  return true;
}

bool masm::MasmContext::emit() {
  Generator GENERATE(details);
  if (!GENERATE.pre_emission() || !GENERATE.emit_header() ||
      !GENERATE.emit_ITIT() || !GENERATE.emit_Instructions() ||
      !GENERATE.emit_data_section() || !GENERATE.emit_string_section())
    return false;
  return true;
}
