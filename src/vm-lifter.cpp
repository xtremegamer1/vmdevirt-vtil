#include <vm-lifter.hpp>

namespace vm
{
  lifter::lifter(std::vector<uint8_t> bytecode, std::uintptr_t initial_vip) :
                                          block {vtil::basic_block::begin(initial_vip  + 4)}
  {
    vm_bytecode = bytecode;
  }
  lifter::lifter(std::ifstream& devirt_file)
  {
    vm::devirt_file_header header;
    devirt_file.read(reinterpret_cast<char*>(&header), sizeof(header));
    std::size_t bytecode_size = header.filesize - sizeof(header);
    vm_bytecode.resize(bytecode_size);
    devirt_file.read(reinterpret_cast<char*>(vm_bytecode.data()), bytecode_size);
    block = vtil::basic_block::begin(header.starting_vip + 4);
  }
  bool lifter::lift()
  {
    //do some stuff
    return true;
  }
}