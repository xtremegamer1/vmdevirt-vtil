#pragma once
#include <zyreg_to_register_desc.hpp> //It is very important this be included before vtil/vtil so that the cast can be visible to vtil
#include <vtil\vtil>
#include <zyreg_to_register_desc.hpp>
#include <vmemu_t.hpp>
#include <vector>

namespace vm
{
typedef void(fn_lifter)(vm::instrs::vinstr_t, vtil::basic_block*);
class lifter_t
{  
  static constexpr vtil::register_desc FLAG_CF = vtil::REG_FLAGS.select( 1, 0 );
  static constexpr vtil::register_desc FLAG_PF = vtil::REG_FLAGS.select( 1, 2 );
  static constexpr vtil::register_desc FLAG_AF = vtil::REG_FLAGS.select( 1, 4 );
  static constexpr vtil::register_desc FLAG_ZF = vtil::REG_FLAGS.select( 1, 6 );
  static constexpr vtil::register_desc FLAG_SF = vtil::REG_FLAGS.select( 1, 7 );
  static constexpr vtil::register_desc FLAG_DF = vtil::REG_FLAGS.select( 1, 10 );
  static constexpr vtil::register_desc FLAG_OF = vtil::REG_FLAGS.select( 1, 11 );

  public:
  lifter_t(const vm::instrs::vrtn_t* routine, const vm::vmctx_t* const ctx);
  bool lift();
  const vtil::routine* get_routine();  
  private:
  const vm::vmctx_t* const m_ctx;
  const vm::instrs::vrtn_t* vmp_routine;
  vtil::routine* vtil_routine;
  bool lift_handler(const vm::instrs::vinstr_t v_instr, vtil::basic_block* vtil_block);
  bool recursive_lifter(const instrs::vblk_t*, vtil::basic_block*, 
    std::unordered_map<std::uintptr_t, const instrs::vblk_t*>&);
  fn_lifter lifter_add, lifter_and, lifter_imul, lifter_jmp, lifter_lconst, lifter_lcr0, 
    lifter_lreg, lifter_lvsp, lifter_nand, lifter_nop, lifter_nor, lifter_or, 
    lifter_read, lifter_shl, lifter_shld, lifter_shr, lifter_shrd, lifter_sreg, 
    lifter_svsp, lifter_vmexit, lifter_write, lifter_writedr7;
};
}