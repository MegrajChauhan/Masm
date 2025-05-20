#ifndef _ANALYZER_BASE_
#define _ANALYZER_BASE_

#include <nodes.hpp>
#include <vector>

namespace masm {
class Analyzer {
public:
  Analyzer() = default;

  virtual ~Analyzer() = default;

  virtual void set_nodes(std::vector<Node> &&nodes) = 0;

  virtual std::vector<Node> get_result() = 0;

  virtual bool first_loop() = 0;

  virtual bool first_loop_second_phase() = 0;

  virtual bool second_loop() = 0;
};
}; // namespace masm

#endif
