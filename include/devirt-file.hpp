#include <cstddef>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

namespace vm
{
struct devirt_file_header 
{
  const char magic[8] = {'V', 'M', 'D', 'E', 'V', 'I', 'R', 'T'};   
  std::uintptr_t starting_vip;
  uint32_t vmenter_rva; // RVA of the PUSH instruction
  uint32_t filesize; //Total file size, including header
};
class devirt_file
{
  public:
  devirt_file(const std::vector<uint8_t>& bytecode, std::uintptr_t vmentry, uint64_t initial_vip);
  bool save_file(std::ofstream& out_file);
  bool save_file(std::string out_file_name);
  private:
  devirt_file_header header;
  uint8_t bytecode[];
};
}