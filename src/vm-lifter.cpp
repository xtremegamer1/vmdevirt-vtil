/* #include <vm-lifter.hpp>

  static constexpr vtil::register_desc FLAG_CF = vtil::REG_FLAGS.select( 1, 0 );
  static constexpr vtil::register_desc FLAG_PF = vtil::REG_FLAGS.select( 1, 2 );
  static constexpr vtil::register_desc FLAG_AF = vtil::REG_FLAGS.select( 1, 4 );
  static constexpr vtil::register_desc FLAG_ZF = vtil::REG_FLAGS.select( 1, 6 );
  static constexpr vtil::register_desc FLAG_SF = vtil::REG_FLAGS.select( 1, 7 );
  static constexpr vtil::register_desc FLAG_DF = vtil::REG_FLAGS.select( 1, 10 );
  static constexpr vtil::register_desc FLAG_OF = vtil::REG_FLAGS.select( 1, 11 );

namespace vm
{
  lifter::lifter(const vm::instrs::vrtn_t* const routine) : vmp_routine(routine), reg_id(0) {};

  bool lifter::lift()
  {
    for (int i = 0; i < 32; ++i)
    {
      scratch_regs[i] = new vtil::register_desc(vtil::register_virtual, reg_id++, 64);
    }
    current_block = vtil::basic_block::begin(0);
    vtil_routine = current_block->owner;
    return true;
  }

  void lifter::lift_handler(vm::instrs::vinstr_t v_instr)
  {
    switch (v_instr.mnemonic)
    {
      case instrs::mnemonic_t::sreg:
        current_block->pop(scratch_regs[v_instr.imm.val / 8]->resize(v_instr.stack_size));
        break;
      case instrs::mnemonic_t::lreg:
        current_block->push(scratch_regs[v_instr.imm.val / 8]->resize(v_instr.stack_size));
        break;
      case instrs::mnemonic_t::lconst:
        current_block->push({v_instr.imm.val, v_instr.stack_size});
        break;
      case instrs::mnemonic_t::add:
        auto [add_tmp_0, add_tmp_1, add_tmp_2] = current_block->tmp( v_instr.stack_size,
                                                                     v_instr.stack_size,
                                                                     v_instr.stack_size );
        auto [add_fl_0, add_fl_1, add_fl_2, add_fl_3] = current_block->tmp( 1, 1, 1, 1 );

        current_block
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
        break;
      case instrs::mnemonic_t::div: //unsigned division
        auto [a0, a1, d, c] = current_block->tmp( v_instr.stack_size, 
                                       v_instr.stack_size, 
                                       v_instr.stack_size, 
                                       v_instr.stack_size );

        current_block
          // t0 := [rsp]
          // t1 := [rsp+*]
          // t2 := [rsp+2*]
          ->pop( d ) // d
          ->pop( a0 ) // a
          ->pop( c ) // c
          ->mov( a1, a0 )

          // div 
          ->div( a0, d, c )
          ->rem( a1, d, c )

          // [rsp] := flags
          // [rsp+8] := t0
          // [rsp+8+*] := t1
          ->push( a0 )
          ->push( a1 )
          ->pushf();
        break;
      case instrs::mnemonic_t::idiv:
        auto [a0, a1, d, c] = current_block->tmp( v_instr.stack_size, 
                                                  v_instr.stack_size, 
                                                  v_instr.stack_size, 
                                                  v_instr.stack_size );

        current_block
          // t0 := [rsp]
          // t1 := [rsp+*]
          // t2 := [rsp+2*]
          ->pop( d ) // d
          ->pop( a0 ) // a
          ->pop( c ) // c
          ->mov( a1, a0 )

          // div 
          ->div( a0, d, c )
          ->rem( a1, d, c )

          // [rsp] := flags
          // [rsp+8] := t0
          // [rsp+8+*] := t1
          ->push( a0 )
          ->push( a1 )
          ->pushf();
        break;
      }
    }
  }
} */