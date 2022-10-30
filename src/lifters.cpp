#include <vm-lifter.hpp>

#define LIFTER_LAMBDA [](vtil::basic_block* block, vm::instrs::vinstr_t instr)

namespace vm::lifters
{
  fn_lifter add = LIFTER_LAMBDA {
    auto [add_tmp_0, add_tmp_1, add_tmp_2] = block->tmp( instr.stack_size,
                                                                 instr.stack_size,
                                                                 instr.stack_size );
    auto [add_fl_0, add_fl_1, add_fl_2, add_fl_3] = block->tmp( 1, 1, 1, 1 );

    block
      // add_tmp_0 := [rsp]
      // add_tmp_1 := [rsp+*]
      ->pop( add_tmp_0 )
      ->pop( add_tmp_1 )

      // add_tmp_1 += add_tmp_0
      ->mov( add_tmp_2, add_tmp_1 )
      ->add( add_tmp_1, add_tmp_0 )

      // Update flags.
      // SF = r < 0
      ->tl( FLAG_SF, add_tmp_1, 0 )
      // ZF = r == 0
      ->te( FLAG_ZF, add_tmp_1, 0 )
      // CF = r < a
      ->tul( FLAG_CF, add_tmp_1, add_tmp_2 )
      // add_fl_0 = (a < 0) == (b < 0)
      ->tl( add_fl_2, add_tmp_2, 0 )
      ->tl( add_fl_3, add_tmp_0, 0 )
      ->te( add_fl_0, add_fl_2, add_fl_3 )
      // add_fl_1 = (a < 0) != (r < 0)
      ->tl( add_fl_2, add_tmp_2, 0 )
      ->tl( add_fl_3, add_tmp_1, 0 )
      ->tne( add_fl_1, add_fl_2, add_fl_3 )
      // OF = add_fl_0 & add_fl_1
      ->mov( FLAG_OF, add_fl_0 )
      ->band( FLAG_OF, add_fl_1 )

      // [rsp] := flags
      // [rsp+8] := add_tmp_1
      ->push( add_tmp_1 )
      ->pushf();
  };
  fn_lifter _and = LIFTER_LAMBDA
  {
    auto [and_tmp_0, and_tmp_1, and_tmp_2] = block->tmp(64, instr.stack_size, instr.stack_size);
    block
      ->pop(and_tmp_0)
      ->pop(and_tmp_1)
      ->ldd(and_tmp_2, and_tmp_0, 0)
      ->band(and_tmp_2, and_tmp_1)
      ->str(and_tmp_0, 0, and_tmp_2)
      ->mov(FLAG_OF, 0)
      ->mov(FLAG_CF, 0)
      ->te(FLAG_ZF, and_tmp_2, 0)
      ->tl(FLAG_SF, and_tmp_2, 0)
      ->pushf();
  };
  fn_lifter imul = LIFTER_LAMBDA
  {
    
  }
}