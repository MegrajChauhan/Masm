#include <filecontext.hpp>

int main() {
  uint64_t addr = 0;
  std::vector<std::filesystem::path> ipaths = {"./", "./tests"};
  masm::FileContext c(ipaths, addr);

  return (c.file_process("test1.gpc.masm"));
}
