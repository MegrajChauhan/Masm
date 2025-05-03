#include <filecontext.hpp>

masm::FileContext::FileContext(std::vector<std::filesystem::path> &i_paths)
    : include_paths{i_paths} {}

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

std::unordered_map<std::string, std::string>
masm::FileContext::get_CONSTANTS() {
  return std::move(CONSTANTS);
}

std::unordered_set<std::string> masm::FileContext::get_LABELS() {
  return std::move(LABELS);
}

std::unordered_set<std::filesystem::path> masm::FileContext::get_imports() {
  return std::move(imports);
}

masm::SymbolTable masm::FileContext::get_symtable() {
  return std::move(symtable);
}

std::vector<masm::Node> masm::FileContext::get_nodes() {
  return std::move(nodes);
}

void masm::FileContext::set_CONSTANTS(
    std::unordered_map<std::string, std::string> &&constants) {
  CONSTANTS = std::move(constants);
}

void masm::FileContext::set_LABELS(std::unordered_set<std::string> &&labels) {
  LABELS = std::move(labels);
}

void masm::FileContext::set_imports(
    std::unordered_set<std::filesystem::path> &&f) {
  imports = std::move(f);
}

void masm::FileContext::set_symtable(SymbolTable &&table) {
  symtable = std::move(table);
}

void masm::FileContext::set_nodes(std::vector<Node> &&n) {
  nodes = std::move(n);
}

bool masm::FileContext::file_process(std::string input_file) {
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
  if (file_already_imported(working_path))
    return true;
  imports.insert(working_path);
  switch (type) {
  case GPC:
    return file_process_GPC(working_path);
  }

  return true;
}

bool masm::FileContext::file_process_GPC(std::filesystem::path working_path) {
  GPCParser parser(working_path.string());

  if (!parser.parse())
    return false;

  std::vector<Node> N = parser.getNodes();

  // Further process the nodes

  for (Node &n : N) {
    switch (n.type) {
    case INCLUDE_DIR:
      if (!file_includes_another_file(n)) {
        simple_message("While processing %s", working_path.c_str());
        return false;
      }
      break;
    case CONST_DEF:
      constant_definition(n);
      break;
    default:
      nodes.push_back(std::move(n));
      break;
    }

    // After all of that is done, we finally begin processing the
    // file before preparing to emit binary code.
    // Now, we will use dedicated handlers to take in 'nodes'.
    // These handling agents will populate the symboltable table,
    // check the code for syntax errors and perform other tasks.
  }

  return true;
}

bool masm::FileContext::file_includes_another_file(Node &node) {
  NodeIncDir *dir = (NodeIncDir *)node.node.get();
  FileContext child(include_paths);

  child.set_CONSTANTS(std::move(CONSTANTS));
  child.set_LABELS(std::move(LABELS));
  child.set_imports(std::move(imports));
  child.set_symtable(std::move(symtable));
  child.set_nodes(std::move(nodes));

  if (!child.file_process(dir->path_included)) {
    return false;
  }

  CONSTANTS = child.get_CONSTANTS();
  LABELS = child.get_LABELS();
  imports = child.get_imports();
  symtable = child.get_symtable();
  nodes = child.get_nodes();

  return true;
}

bool masm::FileContext::constant_definition(Node &node) {
  NodeConstDef *def = (NodeConstDef *)node.node.get();

  // Re-declaring a constant will just change its value.
  // We just have to check that the constant is not defined as a variable
  // already But we cannot check that just yet since we have yet to populate the
  // symtable. This part of processing is only to resolve include dependencies
  // and constant values
  CONSTANTS[def->const_name] = def->const_value;
  return true;
}
