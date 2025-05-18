#include <filecontext.hpp>

masm::FileContext::FileContext(
    std::vector<std::filesystem::path> &i_paths,
    std::unordered_map<std::string, std::pair<value_t, std::string>> &C,
    std::unordered_set<std::string> &L, SymbolTable &sym,
    std::unordered_map<std::string, uint64_t> &laddr,
    std::unordered_map<std::string, uint64_t> &daddr, std::vector<uint8_t> &D,
    std::vector<uint8_t> &S, uint64_t d_addr)
    : CONSTANTS(C), LABELS(L), symtable(sym), label_addresses(laddr),
      data_addresses(daddr), include_paths(i_paths), data(D), string(S),
      analyzer(C, L, sym), gen(sym, laddr, daddr, D, S, d_addr) {
  this->d_addr = d_addr;
}

bool masm::FileContext::is_file_a_directory(std::filesystem::path path) {
  return std::filesystem::is_directory(path);
}

bool masm::FileContext::file_exists(std::filesystem::path path) {
  return std::filesystem::exists(path);
}

bool masm::FileContext::deduce_file_type(std::filesystem::path path) {
  std::string fpath = path.string();

  if (fpath.ends_with(".gpc.masm"))
    type = GPC;
  else {
    simple_message("Unknown File Type: %s", fpath.c_str());
    return false;
  }
  return true;
}

std::pair<bool, size_t>
masm::FileContext::find_the_parent_directory(std::filesystem::path path) {
  size_t i = 0;
  for (std::filesystem::path &p : include_paths) {
    if (file_exists(p / path))
      return std::make_pair(true, i);
    i++;
  }
  return std::make_pair(false, i);
}

bool masm::FileContext::file_already_imported(std::filesystem::path path) {
  return imports.find(path) != imports.end();
}

std::unordered_set<std::filesystem::path> masm::FileContext::get_imports() {
  return std::move(imports);
}

std::vector<masm::Node> masm::FileContext::get_nodes() {
  return std::move(nodes);
}

uint64_t masm::FileContext::get_d_addr() { return d_addr; }

void masm::FileContext::set_imports(
    std::unordered_set<std::filesystem::path> &&f) {
  imports = std::move(f);
}

void masm::FileContext::set_nodes(std::vector<Node> &&n) {
  nodes = std::move(n);
}

bool masm::FileContext::file_prepare(std::string input_file) {
  std::filesystem::path working_path = input_file;
  std::pair<bool, size_t> child_of = find_the_parent_directory(working_path);
  if (!child_of.first) {
    // Not in the include paths
    simple_message("The file '%s' doesn't exist in any of the include paths.",
                   input_file.c_str());
    return false;
  } else {
    working_path = include_paths[child_of.second] / working_path;
  }
  if (is_file_a_directory(working_path)) {
    simple_message(
        "The given file '%s' is not a valid file but a directory instead.",
        working_path.c_str());
    return false;
  }
  if (!deduce_file_type(working_path))
    return false;
  wp = working_path;
  return true;
}

bool masm::FileContext::child_file_type_valid(file_t parent) {
  return type == parent;
}

bool masm::FileContext::should_process_file() {
  if (file_already_imported(wp))
    return false;
  return true;
}

bool masm::FileContext::parse_file() {

  GPCParser parser(wp.string());

  if (!parser.parse()) {
    simple_message("While processing file %s...", wp.c_str());
    return false;
  }

  tmp = parser.getNodes();

  return true;
}

bool masm::FileContext::pre_analysis() {
  for (auto &n : tmp) {
    switch (n.type) {
    case INCLUDE_DIR: {
      if (!file_includes_another_file(n))
        return false;
      break;
    }
    case CONST_DEF: {
      if (!constant_definition(n))
        return false;
      break;
    }
    default:
      nodes.push_back(std::move(n));
    }
  }
  return true;
}

bool masm::FileContext::analyze_file_first_step() {
  analyzer.set_nodes(std::move(nodes));
  return analyzer.first_loop();
}

bool masm::FileContext::analyze_file_first_step_second_phase() {
  return analyzer.first_loop_second_phase();
}

bool masm::FileContext::analyze_file_second_step() {
  return analyzer.second_loop();
}

bool masm::FileContext::gen_file_first_step() {
  gen.set_final_nodes(analyzer.get_result());
  bool ret = gen.first_iteration();
  d_addr = gen.get_current_address_point();
  return ret;
}

bool masm::FileContext::gen_file_first_step_second_phase(uint64_t addr) {
  bool ret = gen.first_iteration_second_phase(addr);
  d_addr = gen.get_current_address_point();
  return ret;
}

bool masm::FileContext::gen_file_first_step_third_phase(uint64_t addr) {
  bool ret = gen.first_iteration_third_phase(addr);
  d_addr = gen.get_current_address_point();
  return ret;
}

bool masm::FileContext::gen_file_second_step() {
  return gen.second_iteration();
}

std::vector<masm::Inst64> masm::FileContext::get_instructions() {
  return gen.get_instructions();
}

std::vector<uint8_t> masm::FileContext::get_data() { return gen.get_data(); }

bool masm::FileContext::file_includes_another_file(Node &node) {
  NodeIncDir *dir = (NodeIncDir *)node.node.get();
  FileContext child(include_paths, CONSTANTS, LABELS, symtable, label_addresses,
                    data_addresses, data, string, d_addr);

  if (!child.file_prepare(dir->path_included)) {
    simple_message("While processing file %s...", wp.c_str());
    return false;
  }

  if (!child.should_process_file())
    return true;

  if (!child.child_file_type_valid(type)) {
    detailed_message(wp.c_str(), node.line,
                     "Included file is not of the same type as parent[%s].",
                     dir->path_included.c_str());
    return false;
  }

  imports.insert(wp);

  child.set_imports(std::move(imports));
  child.set_nodes(std::move(nodes));

  if (!child.parse_file()) {
    simple_message("While processing file %s...", wp.c_str());
    return false;
  }

  // We do not want to do anything more than parsing at this point

  imports = child.get_imports();
  nodes = child.get_nodes();
  d_addr = child.get_d_addr();

  return true;
}

bool masm::FileContext::constant_definition(Node &node) {
  NodeConstDef *def = (NodeConstDef *)node.node.get();

  // Re-declaring a constant will just change its value.
  // We just have to check that the constant is not defined as a variable
  // already But we cannot check that just yet since we have yet to populate the
  // symtable. This part of processing is only to resolve include dependencies
  // and constant values
  CONSTANTS[def->const_name] = std::make_pair(def->type, def->const_value);
  return true;
}
