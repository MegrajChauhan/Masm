#include <output_gen.hpp>

masm::Generator::Generator(GeneratorDetails &det) : details(det) {}

bool masm::Generator::pre_emission() {
  if (std::filesystem::is_directory(details.output_file_path)) {
    simple_message("Given output file: %s : is a directory that exists.",
                   details.output_file_path.c_str());
    return false;
  }

  details.data_section_length = details.data.size();
  details.string_section_length = details.string.size();
  details.number_of_different_ISA_used = details.instructions.size();

  file.open(details.output_file_path, std::ios::out | std::ios::binary);
  return file.is_open();
}

bool masm::Generator::emit_header() {
  file << 'b' << 'e' << 'b' << (unsigned char)(details.type) << (char)0
       << (char)0 << (char)0 << (char)0;
  Inst64 i;
  i.whole_word = details.number_of_different_ISA_used * ITIT_HEADER_LEN;
  file << i.bytes.b7 << i.bytes.b6 << i.bytes.b5 << i.bytes.b4 << i.bytes.b3
       << i.bytes.b2 << i.bytes.b1 << i.bytes.b0;

  i.whole_word = details.data_section_length;
  file << i.bytes.b7 << i.bytes.b6 << i.bytes.b5 << i.bytes.b4 << i.bytes.b3
       << i.bytes.b2 << i.bytes.b1 << i.bytes.b0;

  i.whole_word = details.string_section_length;
  file << i.bytes.b7 << i.bytes.b6 << i.bytes.b5 << i.bytes.b4 << i.bytes.b3
       << i.bytes.b2 << i.bytes.b1 << i.bytes.b0;

  // For DIT, it is all 0 as Masm doesn't support Debug information yet.
  file << (char)0 << (char)0 << (char)0 << (char)0 << (char)0 << (char)0
       << (char)0 << (char)0;
  return true;
}

bool masm::Generator::emit_ITIT() {
  Inst64 i;
  for (auto I : details.instructions) {
    i.whole_word = (uint64_t)I.first;
    file << i.bytes.b7 << i.bytes.b6 << i.bytes.b5 << i.bytes.b4 << i.bytes.b3
         << i.bytes.b2 << i.bytes.b1 << i.bytes.b0;
    i.whole_word = (I.second.size() + 1) * 8;
    file << i.bytes.b7 << i.bytes.b6 << i.bytes.b5 << i.bytes.b4 << i.bytes.b3
         << i.bytes.b2 << i.bytes.b1 << i.bytes.b0;
  }
  return true;
}

bool masm::Generator::emit_Instructions() {
  Inst64 bef = details.entry_inst;
  for (auto I : details.instructions) {
    file << bef.bytes.b7 << bef.bytes.b6 << bef.bytes.b5 << bef.bytes.b4
         << bef.bytes.b3 << bef.bytes.b2 << bef.bytes.b1 << bef.bytes.b0;

    for (auto i : I.second) {
      file << i.bytes.b7 << i.bytes.b6 << i.bytes.b5 << i.bytes.b4 << i.bytes.b3
           << i.bytes.b2 << i.bytes.b1 << i.bytes.b0;
    }
    bef.whole_word = 0;
  }
  return true;
}

bool masm::Generator::emit_data_section() {
  for (auto i : details.data) {
    file << i;
  }
  return true;
}

bool masm::Generator::emit_string_section() {
  for (auto i : details.string) {
    file << i;
  }
  return true;
}
