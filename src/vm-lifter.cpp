#include <vm-lifter.hpp>

namespace vm
{
  lifter_t::lifter_t(const vm::instrs::vrtn_t* const routine, const vm::vmctx_t* const ctx) : 
    vmp_routine(routine), vmentry_pushes(ctx->get_vmentry_push_order()), img_base(ctx->m_image_base), 
    load_delta(ctx->m_image_load_delta) {};
  const vtil::routine* lifter_t::get_routine()
  {
    return vtil_routine;
  }

  bool lifter_t::lift()
  {
    //The rva of first block is the first instruction after the vmenter
    current_block = vtil::basic_block::begin(vmp_routine->m_blks[0].m_vip.rva);
    //VMENTER pushes all gp registers and RFLAGS to the stack which is inherited by the virtual stack.
    for (auto zyreg : vmentry_pushes)
    {
      current_block->push(zyreg);
    }
    //Load delta is pushed to the stack
    current_block->push(load_delta);
    vtil_routine = current_block->owner;
    //Lift the handlers basically
    for (const auto& blk : vmp_routine->m_blks)
    {
      if (&blk != &vmp_routine->m_blks[0])
        current_block = current_block->fork(blk.m_vip.img_based);
      for (const auto& instr : blk.m_vinstrs)
      {
        if(!lift_handler(instr)) {
          std::printf("[!] unrecognized instruction encountered\n");
          return false;
        }
      }
    }
    return true;
  }

  bool lifter_t::lift_handler(vm::instrs::vinstr_t v_instr)
  {
    // I really should have found a less bloated way to implement this
    switch (v_instr.mnemonic)
    {
      case vm::instrs::mnemonic_t::add:
        lifter_add(v_instr);
        break;
      case vm::instrs::mnemonic_t::_and:
        lifter_and(v_instr);
        break;
      case vm::instrs::mnemonic_t::imul:
        lifter_imul(v_instr);
        break;
      case vm::instrs::mnemonic_t::jmp:
        lifter_jmp(v_instr);
        break;
      case vm::instrs::mnemonic_t::lconst:
        lifter_lconst(v_instr);
        break;
      case vm::instrs::mnemonic_t::lcr0:
        lifter_lcr0(v_instr);
        break;
      case vm::instrs::mnemonic_t::lreg:
        lifter_lreg(v_instr);
        break;
      case vm::instrs::mnemonic_t::lvsp:
        lifter_lvsp(v_instr);
        break;
      case vm::instrs::mnemonic_t::nand:
        lifter_nand(v_instr);
        break;
      case vm::instrs::mnemonic_t::nop:
        lifter_nop(v_instr);
        break;
      case vm::instrs::mnemonic_t::nor:
        lifter_nor(v_instr);
        break;
      case vm::instrs::mnemonic_t::_or:
        lifter_or(v_instr);
        break;
      case vm::instrs::mnemonic_t::read:
        lifter_read(v_instr);
        break;
      case vm::instrs::mnemonic_t::shl:
        lifter_shl(v_instr);
        break;
      case vm::instrs::mnemonic_t::shr:
        lifter_shr(v_instr);
        break;
      case vm::instrs::mnemonic_t::shld:
        lifter_shld(v_instr);
        break;
      case vm::instrs::mnemonic_t::shrd:
        lifter_shrd(v_instr);
        break;
      case vm::instrs::mnemonic_t::sreg:
        lifter_sreg(v_instr);
        break;
      case vm::instrs::mnemonic_t::svsp:
        lifter_svsp(v_instr);
        break;
      case vm::instrs::mnemonic_t::vmexit:
        lifter_vmexit(v_instr);
        break;
      case vm::instrs::mnemonic_t::write:
        lifter_write(v_instr);
        break;
      case vm::instrs::mnemonic_t::writedr7:
        lifter_writedr7(v_instr);
        break;
      default:
        return false;
    }
    return true;
  }
}