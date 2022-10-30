#include <vm-lifter.hpp>

namespace vm
{
  lifter_t::lifter_t(const vm::instrs::vrtn_t* const routine, const vm::vmctx_t* const ctx) : 
    vmp_routine(routine), vmentry_pushes(ctx->get_vmentry_push_order()), load_delta(ctx->m_image_load_delta) {};

  static inline void allocate_scratch(vtil::register_desc* scratch_regs[], uint64_t& reg_id){
    for (int i = 0; i < 32; ++i)
      scratch_regs[i] = new vtil::register_desc(vtil::register_virtual, reg_id++, 64);
  }
  static inline void deallocate_scratch(vtil::register_desc* scratch_regs[]) {
    for (int i = 0; i < 32; ++i)
      delete scratch_regs[i];
  }

  bool lifter_t::lift()
  {
    allocate_scratch(scratch_regs, reg_id);
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
      for (const auto& instr : blk.m_vinstrs)
      {
        lift_handler(instr);
      }
    }
    for (auto r_it = vmentry_pushes.rbegin(); r_it != vmentry_pushes.rend(); ++r_it)
    {
      current_block->pop(*r_it);
    }
    deallocate_scratch(scratch_regs);
    return true;
  }

  void lifter_t::lift_handler(vm::instrs::vinstr_t v_instr)
  {
    switch (v_instr.mnemonic)
    {

    }
  }
}