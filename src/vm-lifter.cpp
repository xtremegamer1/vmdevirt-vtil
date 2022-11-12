#include <vm-lifter.hpp>
#include <functional>

namespace vm
{
  lifter_t::lifter_t(const vm::instrs::vrtn_t* routine, const vm::vmctx_t* const ctx) : 
    vmp_routine(routine), m_ctx(ctx) {};
  const vtil::routine* lifter_t::get_routine()
  {
    return vtil_routine;
  }

  bool lifter_t::recursive_lifter(const instrs::vblk_t* vmp_blk, vtil::basic_block* vtil_blk, 
    std::unordered_map<std::uintptr_t, const instrs::vblk_t*>& unexplored_branches)
  {
    for (const auto& instr : vmp_blk->m_vinstrs)
    {
      if(!lift_handler(instr, vtil_blk)) {
        std::printf("[!] unrecognized instruction encountered\n");
        return false;
      }
    }
    if (vmp_blk->branch_type == instrs::vbranch_type::absolute ||
        vmp_blk->branch_type == instrs::vbranch_type::jcc)
    {
      std::uintptr_t branch_addr = vmp_blk->branches[0] - m_ctx->m_module_base + m_ctx->m_image_base;
      vtil::basic_block* new_vtil_block = vtil_blk->fork(branch_addr);
      if (new_vtil_block) {
        const auto new_vmp_blk_it = unexplored_branches.find(branch_addr);
        const instrs::vblk_t* new_vmp_blk = new_vmp_blk_it->second;
        // recurse
        if (!recursive_lifter(new_vmp_blk, new_vtil_block, unexplored_branches))
          return false;
      }
    }
    // Exactly the same thing but for the second branch
    if (vmp_blk->branch_type == instrs::vbranch_type::jcc)
    {
      std::uintptr_t branch_addr = vmp_blk->branches[1] - m_ctx->m_module_base + m_ctx->m_image_base;
      vtil::basic_block* new_vtil_block = vtil_blk->fork(branch_addr);
      if (new_vtil_block) {
        const auto new_vmp_blk_it = unexplored_branches.find(branch_addr);
        const instrs::vblk_t* new_vmp_blk = new_vmp_blk_it->second;
        // recurse
        if (!recursive_lifter(new_vmp_blk, new_vtil_block, unexplored_branches))
          return false;
      }
    }
    return true;
  }

  bool lifter_t::lift()
  {
    //The rva of first block is the first instruction after the vmenter
    auto* root_block = vtil::basic_block::begin(vmp_routine->m_blks[0].m_vip.rva);
    //VMENTER pushes all gp registers and RFLAGS to the stack which is inherited by the virtual stack.
    auto& vmentry_push_order = m_ctx->get_vmentry_push_order();
    for (auto zyreg : vmentry_push_order)
    {
      root_block->push(zyreg);
    }
    //Load delta is pushed to the stack
    root_block->push(vtil::make_imm<uint64_t>(0));
    vtil_routine = root_block->owner;
    //Lift the handlers basically
    // TODO: Use branch info in the blocks in the routine to make sure jcc are forked properly for better optimization!
    // A map of vip addresses to basic_blocks will probably be good for this
    std::unordered_map<std::uintptr_t, const instrs::vblk_t*> unexplored;
    for (auto& vmp_blk : vmp_routine->m_blks)
    {
      unexplored.insert(std::pair(vmp_blk.m_vip.img_based, &vmp_blk));
    }
    return recursive_lifter(&vmp_routine->m_blks[0], root_block, unexplored);
  };


  bool lifter_t::lift_handler(vm::instrs::vinstr_t v_instr, vtil::basic_block* vtil_block)
  {
    // I really should have found a less bloated way to implement this
    switch (v_instr.mnemonic)
    {
      case vm::instrs::mnemonic_t::add:
        lifter_add(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::_and:
        lifter_and(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::imul:
        lifter_imul(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::jmp:
        lifter_jmp(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::lconst:
        lifter_lconst(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::lcr0:
        lifter_lcr0(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::lreg:
        lifter_lreg(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::lvsp:
        lifter_lvsp(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::nand:
        lifter_nand(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::nop:
        lifter_nop(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::nor:
        lifter_nor(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::_or:
        lifter_or(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::read:
        lifter_read(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::shl:
        lifter_shl(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::shr:
        lifter_shr(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::shld:
        lifter_shld(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::shrd:
        lifter_shrd(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::sreg:
        lifter_sreg(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::svsp:
        lifter_svsp(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::vmexit:
        lifter_vmexit(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::write:
        lifter_write(v_instr, vtil_block);
        break;
      case vm::instrs::mnemonic_t::writedr7:
        lifter_writedr7(v_instr, vtil_block);
        break;
      default:
        return false;
    }
    return true;
  }
}