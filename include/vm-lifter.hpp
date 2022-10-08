#include <vtil\vtil>
#include <devirt-file.hpp>
#include <vector>

namespace vm
{
class lifter
{
  public:
  lifter(std::vector<uint8_t> bytecode, std::uintptr_t initial_vip);
  lifter(std::ifstream& devirt_file);
  bool lift();
  private:
  vtil::basic_block* block;
  std::vector<uint8_t> vm_bytecode;
};
}