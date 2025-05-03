#include <filecontext.hpp>

int main() {
  std::vector<std::filesystem::path> ipaths = {"./", "./tests"};
  masm::FileContext c(ipaths);

  return (c.file_process("test1.gpc.masm"));
}
