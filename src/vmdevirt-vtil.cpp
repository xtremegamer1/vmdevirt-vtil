#include <vmdevirt-vtil.hpp>
#include <vtil-compile.hpp>

int __cdecl main(int argc, const char *argv[])
{
  argparse::argument_parser_t parser("VMProtect 3 Static Devirtualization",
                                     "vmdevirt");
  for (int i = 0; i < argc; i++)
  {
    std::cout << argv[i] << '\n';
  }

  parser.add_argument()
      .name("--vmentry")
      .description("relative virtual address to a vm entry...")
      .required(false);
  parser.add_argument()
      .name("--bin")
      .description("path to unpacked virtualized binary...")
      .required(true);
  parser.add_argument().name("--out").description("output file name...");
  vm::utils::init();
  parser.enable_help();
  auto result = parser.parse(argc, argv);
  if (result || parser.exists("help"))
  {
    parser.print_help();
    return -1;
  }
  std::cout << parser.get<std::string>("bin") << '\n';
  std::vector<std::uint8_t> module_data, tmp, unpacked_bin;
  if (!vm::utils::open_binary_file(parser.get<std::string>("bin"),
                                   module_data))
  {
    std::printf("[!] failed to open binary file...\n");
    return -1;
  }
  auto img = reinterpret_cast<win::image_t<> *>(module_data.data());
  auto image_size = img->get_nt_headers()->optional_header.size_image;
  const auto image_base = img->get_nt_headers()->optional_header.image_base;
  // page align the vector allocation so that unicorn-engine is happy girl...
  tmp.resize(image_size + PAGE_4KB);
  const std::uintptr_t module_base =
      reinterpret_cast<std::uintptr_t>(tmp.data()) +
      (PAGE_4KB - (reinterpret_cast<std::uintptr_t>(tmp.data()) & 0xFFFull));
  std::memcpy((void *)module_base, module_data.data(), 0x1000);
  std::for_each(img->get_nt_headers()->get_sections(),
                img->get_nt_headers()->get_sections() +
                    img->get_nt_headers()->file_header.num_sections,
                [&](const auto &section_header)
                {
                  std::memcpy(
                      (void *)(module_base + section_header.virtual_address),
                      module_data.data() + section_header.ptr_raw_data,
                      section_header.size_raw_data);
                });
  auto win_img = reinterpret_cast<win::image_t<> *>(module_base);
  auto basereloc_dir =
      win_img->get_directory(win::directory_id::directory_entry_basereloc);
  auto reloc_dir = reinterpret_cast<win::reloc_directory_t *>(
      basereloc_dir->rva + module_base);
  win::reloc_block_t *reloc_block = &reloc_dir->first_block;
  // apply relocations to all sections...
  while (reloc_block->base_rva && reloc_block->size_block)
  {
    std::for_each(reloc_block->begin(), reloc_block->end(),
                  [&](win::reloc_entry_t &entry)
                  {
                    switch (entry.type)
                    {
                    case win::reloc_type_id::rel_based_dir64:
                    {
                      auto reloc_at = reinterpret_cast<std::uintptr_t *>(
                          entry.offset + reloc_block->base_rva + module_base);
                      *reloc_at = module_base + ((*reloc_at) - image_base);
                      break;
                    }
                    default:
                      break;
                    }
                  });
    reloc_block = reloc_block->next();
  }
  std::printf("> image base = %p, image size = %p, module base = %p\n",
              image_base, image_size, module_base);
  if (!image_base || !image_size || !module_base)
  {
    std::printf("[!] failed to open binary on disk...\n");
    return -1;
  }
  std::vector<std::uintptr_t> vm_entry_rvas;
  auto vmentry_paramter = parser.get<std::string>("vmentry");
  if (!vmentry_paramter.empty())
    vm_entry_rvas.emplace_back(std::strtoull(parser.get<std::string>("vmentry").c_str(), nullptr, 16));
  else
  {
    auto vm_entries =  vm::locate::get_vm_entries(module_base, image_size);
    std::for_each(vm_entries.begin(), vm_entries.end(),
                                      [&](const vm::locate::vm_enter_t& vmenter) {
                                        static int vmenter_count = 0;
                                        std::printf("[%.4d]  Discovered vmenter at rva: %p\n", 
                                                    ++vmenter_count, vmenter.rva);
                                        vm_entry_rvas.emplace_back(vmenter.rva);
                                      });
  }
  std::filesystem::path routines_folder(parser.get<std::string>("bin"));
  routines_folder = routines_folder.remove_filename() / "vms";
  std::filesystem::path bin_name = std::filesystem::path(parser.get<std::string>("bin")).filename();
  std::filesystem::remove_all(routines_folder);
  std::filesystem::create_directory(routines_folder);
  
  //module_data still contains the raw binary loaded from disk
  //TODO: Handle the rare edge case that there isn't room for another section header
  win::section_header_t* new_header_addr = 
    img->get_nt_headers()->get_sections() + img->get_nt_headers()->file_header.num_sections;
  img->get_nt_headers()->file_header.num_sections += 1;
  memset(new_header_addr, 0, sizeof(win::section_header_t));
  new_header_addr->virtual_address = 0x29420000;
  new_header_addr->name = { 'v', 't', 'i', 'l', '_', 'a', 's', 'm' };
  uint32_t section_alignment = img->get_nt_headers()->optional_header.section_alignment;
  uint32_t file_alignment = img->get_nt_headers()->optional_header.file_alignment;
  uint32_t old_size = module_data.size();
  old_size = (old_size + file_alignment) & ~static_cast<size_t>(file_alignment - 1);
  new_header_addr->ptr_raw_data = old_size;
  
  for (int i = 0; i < img->get_nt_headers()->file_header.num_sections; ++i)
  {
    for (int j = 0; j < 8; ++j)
    {
      std::cout << img->get_nt_headers()->get_section(i)->name.short_name[j];
    }
    std::cout << "\n";
  }
  
  std::vector<uint8_t> assembly;
  assembly.reserve(100'000'000);
  for (const auto& vm_entry_rva : vm_entry_rvas)
  {
    vm::vmctx_t vmctx(module_base, image_base, image_size, vm_entry_rva);
    if (!vmctx.init())
    {
      std::printf(
          "[!] failed to init vmctx... this can be for many reasons..."
          " try validating your vm entry rva... make sure the binary is "
          "unpacked and is"
          "protected with VMProtect 3...\n");
      return -1;
    }
    vm::emu_t emu(&vmctx);
    if (!emu.init())
    {
      std::printf(
          "[!] failed to init vm::emu_t... read above in the console for the "
          "reason...\n");
      return -1;
    }
    vm::instrs::vrtn_t virt_rtn;
    if (!emu.emulate(vm_entry_rva, virt_rtn))
    {
      std::printf(
          "[!] failed to emulate virtualized routine... read above in the "
          "console for the reason...\n");
    }
    std::printf("> traced %d virtual code blocks... \n", virt_rtn.m_blks.size());
   
    std::stringstream hex_rva;
    hex_rva << std::hex << vm_entry_rva;
    auto save_to = routines_folder / (bin_name.string() + "-" + hex_rva.str() + ".txt");
    std::ofstream virtual_assembly(save_to);
    for (auto it = virt_rtn.m_blks.begin(); it != virt_rtn.m_blks.end(); ++it)
    {
      virtual_assembly << "BLOCK_" << it - virt_rtn.m_blks.begin() << ":\n";
      for (auto instr : it->m_vinstrs)
      {
        virtual_assembly << "SIZE:\t"<< std::dec  << +instr.stack_size << " " << vm::instrs::get_profile(instr.mnemonic)->name;
        if (instr.imm.has_imm)
          virtual_assembly << "\t" << std::hex << +instr.imm.val;
        virtual_assembly << "\n";
      }
      virtual_assembly << '\n';
    }
    virtual_assembly.flush();
    
    vm::lifter_t lifter(&virt_rtn, &vmctx);
    if (!lifter.lift())
    {
      std::printf(
            "[!] failed to lift virtual routine to VTIL... read above in "
            "the console for the reason...\n");
      return -1;
    }

    // Replace VMENTER with a jmp to the compiled asm
    // Find file offset for virtual address
    auto* vm_entry_section = std::find_if(img->get_nt_headers()->get_sections(), 
      img->get_nt_headers()->get_sections() + img->get_nt_headers()->file_header.num_sections - 1,
      [vm_entry_rva](const win::section_header_t& scn)
      {
        if (vm_entry_rva >= scn.virtual_address && vm_entry_rva < scn.virtual_address + scn.virtual_size)
          return true;
        else return false;
      }
    );
    if (vm_entry_section == img->get_nt_headers()->get_sections() + img->get_nt_headers()->file_header.num_sections - 1)
    {
      std::printf("[!] couldn't find file offset for RVA %.8X\n", vm_entry_rva);
      return -1;
    }
    uint32_t vm_entry_file_offset = 
      vm_entry_rva - vm_entry_section->virtual_address + vm_entry_section->ptr_raw_data;
    // change 10-byte PUSH XXXXXXXX CALL XXXXXXXX with JMP [devirtualized]
    memset(module_data.data() + vm_entry_file_offset, 0x90, 10);
    module_data[vm_entry_file_offset] = 0xe9;
    uint32_t relative_offset = (new_header_addr->virtual_address - vm_entry_rva) +
      assembly.size() - 5;
    *reinterpret_cast<uint32_t*>(&module_data[vm_entry_file_offset + 1]) = relative_offset;
    
    save_to = routines_folder / (bin_name.string() + "-" + hex_rva.str() + "-premature" + ".vtil");
    vtil::save_routine(lifter.get_routine(), save_to);
    save_to = routines_folder / (bin_name.string() + "-" + hex_rva.str() + "-optimized" + ".vtil");
    vtil::routine* optimized = lifter.get_routine()->clone();
    vtil::optimizer::apply_all_profiled(optimized);
    vtil::save_routine(optimized, save_to);
    // TODO: Handle failure of compilation
    compile(optimized, assembly);

    //Replace address of jmp generated by compiler
    uint32_t ret_addr_offset = (vm_entry_rva + 10) - (new_header_addr->virtual_address + assembly.size() + 4);
    *reinterpret_cast<std::byte*>(&assembly[assembly.size() - 1]) = (std::byte)0xe9;
    std::printf("Size of assembly is: %llX\n", assembly.size());
    assembly.resize(assembly.size() + 4);
    *reinterpret_cast<uint32_t*>(&assembly[assembly.size() - 4]) = ret_addr_offset;
  }
  new_header_addr->virtual_size = ((assembly.size() & 
    ~static_cast<size_t>(section_alignment)) + section_alignment);
  new_header_addr->size_raw_data = (assembly.size() & ~static_cast<size_t>(file_alignment - 1)) + file_alignment;
  module_data.resize(((old_size + assembly.size()) & ~static_cast<size_t>(file_alignment - 1)) + file_alignment);
  memcpy(module_data.data() + old_size, assembly.data(), assembly.size());
  auto decompiled_bin_name = std::filesystem::path(parser.get<std::string>("bin")).remove_filename() /
    (std::filesystem::path(bin_name).replace_extension("").string() + "-devirtualized" + bin_name.extension().string());
  std::filesystem::remove(decompiled_bin_name);
  std::ofstream decompiled_bin_file(decompiled_bin_name, std::ios::binary);
  
  decompiled_bin_file.write(reinterpret_cast<const char*>(module_data.data()), module_data.size());
}