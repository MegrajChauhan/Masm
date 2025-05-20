#include <masm_context.hpp>

int main(int argc, char **argv) {
  masm::MasmContext context(argc, argv);
  if (!context.prepare_for_assembling() || !context.assemble() ||
      !context.prepare_for_emiting() || !context.emit())
    return -1;
  return 0;
}
