#include <devirt-file.hpp>

namespace vm
{
  devirt_file::devirt_file(const std::vector<uint8_t>& bytecode, std::uintptr_t vmentry, std::uintptr_t initial_vip)
  {
    header.starting_vip = initial_vip;
    header.filesize = sizeof(header) + bytecode.size();
    header.vmenter_rva = vmentry;
  }
  bool devirt_file::save_file(std::ofstream& out_file)
  {
    //sanity check
    if (header.filesize < sizeof(header))
        return false;

    out_file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    if (out_file.fail()) return false;
    out_file.write(reinterpret_cast<const char*>(bytecode), header.filesize - sizeof(header));
    if (out_file.fail()) return false;
    else return true;
  }
  bool devirt_file::save_file(std::string out_file_name)
  {
    std::ofstream out_file(out_file_name.c_str());
    if (!out_file.is_open())
      return false;
    return save_file(out_file);
  }
}