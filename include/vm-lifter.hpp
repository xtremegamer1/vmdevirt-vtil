#include <vtil\vtil>
#include <zyreg_to_register_desc.hpp>
#include <vmemu_t.hpp>
#include <vector>

namespace vm
{
typedef void(fn_lifter)(vm::instrs::vinstr_t);
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
  lifter_t(const vm::instrs::vrtn_t* const routine, const vm::vmctx_t* const ctx);
  bool lift();
  const vtil::routine* get_routine();  
  private:
  const std::uintptr_t img_base; //The original non-relocated base
  const std::uintptr_t load_delta;
  const std::array<ZydisRegister, 16> vmentry_pushes;
  const vm::instrs::vrtn_t* const vmp_routine;
  vtil::routine* vtil_routine;
  vtil::basic_block* current_block;
  bool lift_handler(vm::instrs::vinstr_t v_instr);
  fn_lifter lifter_add, lifter_and, lifter_imul, lifter_jmp, lifter_lconst, lifter_lcr0, 
    lifter_lreg, lifter_lvsp, lifter_nand, lifter_nop, lifter_nor, lifter_or, 
    lifter_read, lifter_shl, lifter_shld, lifter_shr, lifter_shrd, lifter_sreg, 
    lifter_svsp, lifter_vmexit, lifter_write, lifter_writedr7;
};
}