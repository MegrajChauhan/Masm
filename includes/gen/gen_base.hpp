#ifndef _GEN_BASE_
#define _GEN_BASE_

#include <cstdint>
#include <nodes.hpp>
#include <vector>

namespace masm {
union Inst64 {
#if __BYTE_ORDER == __LITTLE_ENDIAN
  struct {
    uint8_t b7;
    uint8_t b6;
    uint8_t b5;
    uint8_t b4;
    uint8_t b3;
    uint8_t b2;
    uint8_t b1;
    uint8_t b0;
  } bytes;

  struct {
    uint16_t w3;
    uint16_t w2;
    uint16_t w1;
    uint16_t w0;
  } half_half_words;

  struct {
    uint32_t w1;
    uint32_t w0;
  } half_words;
#else
  struct {
    uint8_t b0;
    uint8_t b1;
    uint8_t b2;
    uint8_t b4;
    uint8_t b5;
    uint8_t b3;
    uint8_t b6;
    uint8_t b7;
  } bytes;

  struct {
    uint16_t w0;
    uint16_t w1;
    uint16_t w2;
    uint16_t w3;
  } half_half_words;

  struct {
    uint32_t w0;
    uint32_t w1;
  } half_words;
#endif
  uint64_t whole_word = 0;
};

class Gen {
public:
  Gen() = default;

  virtual ~Gen() = default;

  virtual void set_final_nodes(std::vector<Node> &&nodes) = 0;

  virtual uint64_t get_current_address_point() = 0;

  virtual std::vector<Inst64> get_instructions() = 0;

  virtual std::vector<uint8_t> get_data() = 0;

  virtual Inst64 get_ENTRY_INSTRUCTION(size_t addr) = 0;

  virtual bool first_iteration(uint64_t addr_point) = 0;

  virtual bool first_iteration_second_phase(uint64_t addr_point) = 0;

  virtual bool first_iteration_third_phase(uint64_t addr_point) = 0;

  virtual bool second_iteration() = 0;
};

}; // namespace masm

#endif
