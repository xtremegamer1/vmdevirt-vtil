#include <vm-lifter.hpp>

// A lot of stuff in this file is copied from novmp
static constexpr vtil::register_desc make_virtual_register( uint8_t context_offset, uint8_t size )
{
	fassert( ( ( context_offset & 7 ) + ( size / 8) ) <= 8 && size && (size % 8) == 0 );

	return {
		vtil::register_virtual,
		( size_t ) context_offset / 8,
		size,
		( context_offset % 8 ) * 8
	};
}

void vm::lifter_t::lifter_add(vm::instrs::vinstr_t instr) {
  auto [add_tmp_0, add_tmp_1, add_tmp_2] = current_block->tmp( instr.stack_size,
                                                               instr.stack_size,
                                                               instr.stack_size );
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
};
void vm::lifter_t::lifter_and(vm::instrs::vinstr_t instr)
{
  auto [and_tmp_0, and_tmp_1, and_tmp_2] = current_block->tmp(64, instr.stack_size, instr.stack_size);
  current_block
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
void vm::lifter_t::lifter_imul(vm::instrs::vinstr_t instr)
{
  std::printf("This lifter is not implemented\n");    
  exit(ERROR_CALL_NOT_IMPLEMENTED);
};
void vm::lifter_t::lifter_jmp(vm::instrs::vinstr_t instr)
{
  auto tmp = current_block->tmp(64);
  current_block
    ->pop(tmp)
    ->jmp(tmp);
};
void vm::lifter_t::lifter_lconst(vm::instrs::vinstr_t instr)
{
  auto tmp = current_block->tmp(instr.imm.size);
  current_block
    ->mov(tmp, instr.imm.val)
    ->push(tmp); 
};
void vm::lifter_t::lifter_lcr0(vm::instrs::vinstr_t instr)
{
  current_block->push(ZYDIS_REGISTER_CR0);
};
void vm::lifter_t::lifter_lreg(vm::instrs::vinstr_t instr)
{
  current_block->push(make_virtual_register(instr.imm.val, instr.stack_size));
};
void vm::lifter_t::lifter_sreg(vm::instrs::vinstr_t instr)
{
  current_block->pop(make_virtual_register(instr.imm.val, instr.stack_size));
}
void vm::lifter_t::lifter_lvsp(vm::instrs::vinstr_t instr)
{
  current_block->push(vtil::REG_SP);  
};
void vm::lifter_t::lifter_nand(vm::instrs::vinstr_t instr)
{
  auto [tmp0, tmp1, tmp2, fl0, fl1] = current_block->tmp(instr.stack_size, instr.stack_size, instr.stack_size, 1, 1);
  current_block
    ->pop(tmp1)
    ->pop(tmp0)
    ->bnot(tmp0)
    ->bnot(tmp1)
    ->mov(tmp2, tmp0)
    ->bor(tmp0, tmp1)

    ->mov(FLAG_OF, 0)
    ->mov(FLAG_CF, 0)
    ->te(FLAG_ZF, tmp2, 0)
    ->tl(FLAG_SF, tmp0, 0)
    ->pushf();
};
void vm::lifter_t::lifter_nop(vm::instrs::vinstr_t instr) 
{
  current_block->nop();
};
void vm::lifter_t::lifter_nor(vm::instrs::vinstr_t instr)
{
  auto [tmp0, tmp1, tmp2, fl0, fl1] = current_block->tmp(instr.stack_size, instr.stack_size, instr.stack_size, 1, 1);
  current_block
    ->pop(tmp1)
    ->pop(tmp0)
    ->bnot(tmp0)
    ->bnot(tmp1)
    ->mov(tmp2, tmp0)
    ->band(tmp0, tmp1)

    ->mov(FLAG_OF, 0)
    ->mov(FLAG_CF, 0)
    ->te(FLAG_ZF, tmp2, 0)
    ->tl(FLAG_SF, tmp0, 0)
    ->pushf();
};
void vm::lifter_t::lifter_or(vm::instrs::vinstr_t instr)
{
  auto [and_tmp_0, and_tmp_1, and_tmp_2] = current_block->tmp(64, instr.stack_size, instr.stack_size);
  current_block
    ->pop(and_tmp_0)
    ->pop(and_tmp_1)
    ->ldd(and_tmp_2, and_tmp_0, 0)
    ->bor(and_tmp_2, and_tmp_1)
    ->str(and_tmp_0, 0, and_tmp_2)

    ->mov(FLAG_OF, 0)
    ->mov(FLAG_CF, 0)
    ->te(FLAG_ZF, and_tmp_2, 0)
    ->tl(FLAG_SF, and_tmp_2, 0)
    ->pushf();
};
void vm::lifter_t::lifter_read(vm::instrs::vinstr_t instr)
{
  auto [tmp0, tmp1] = current_block->tmp(64, instr.stack_size);
  current_block
    ->pop(tmp0)
    ->ldd(tmp1, tmp0, 0)
    ->push(tmp1);
};
void vm::lifter_t::lifter_shl(vm::instrs::vinstr_t instr)
{
  auto [tmp0, tmp1, tmp2] = current_block->tmp(instr.stack_size, instr.stack_size, 8);
  auto cf = tmp2.select(1, tmp2.bit_count - 1); // Set to most significant bit 
  auto ofx = tmp0.select(1, tmp0.bit_count - 1);
  current_block
    ->pop(tmp0)
    ->pop(tmp1)
    
    ->mov(tmp2, tmp1)
    ->bshl(tmp0, tmp1)
    
    ->tl(FLAG_SF, tmp0, 0)
    ->te(FLAG_SF, tmp0, 0)
    ->mov(FLAG_OF, ofx)
    ->mov(FLAG_CF, cf)
    ->bxor(FLAG_OF, cf)

    ->push(tmp0)
    ->pushf();
};

void vm::lifter_t::lifter_shr(vm::instrs::vinstr_t instr)
{
  auto [tmp0, tmp1, tmp2] = current_block->tmp(instr.stack_size, instr.stack_size, 8);
  auto cf = tmp2.select(1, tmp2.bit_count - 1); // Set to most significant bit 
  auto ofx = tmp0.select(1, tmp0.bit_count - 1);
  current_block
    ->pop(tmp0)
    ->pop(tmp1)
    
    ->mov(tmp2, tmp1)
    ->bshr(tmp0, tmp1)
    
    ->tl(FLAG_SF, tmp0, 0)
    ->te(FLAG_SF, tmp0, 0)
    ->mov(FLAG_OF, ofx)
    ->mov(FLAG_CF, cf)
    ->bxor(FLAG_OF, cf)

    ->push(tmp0)
    ->pushf();
};

void vm::lifter_t::lifter_shrd(vm::instrs::vinstr_t instr)
{
  auto [tmp0, tmp1, tmp2, tmp3] = current_block->tmp(instr.stack_size, instr.stack_size, 8, 8);
  current_block
    ->pop(tmp0)
    ->pop(tmp1)
    ->pop(tmp2)

    ->bshr(tmp0, tmp2)

    // find out how many bits to shift the shiftin value by
    ->mov(tmp3, vtil::make_imm<uint8_t>(instr.stack_size))
    ->sub(tmp3, tmp2)
    
    ->bshl(tmp1, tmp3)
    ->bor(tmp0, tmp1)
    
    ->te(FLAG_ZF, tmp0, 0)
    ->tl(FLAG_SF, tmp0, 0)
    ->mov(FLAG_OF, vtil::UNDEFINED)
    ->mov(FLAG_CF, vtil::UNDEFINED)

    ->push(tmp0)
    ->pushf();
}

void vm::lifter_t::lifter_shld(vm::instrs::vinstr_t instr)
{
  auto [tmp0, tmp1, tmp2, tmp3] = current_block->tmp(instr.stack_size, instr.stack_size, 8, 8);
  current_block
    ->pop(tmp0)
    ->pop(tmp1)
    ->pop(tmp2)

    ->bshl(tmp0, tmp2)

    // find out how many bits to shift the shiftin value by
    ->mov(tmp3, vtil::make_imm<uint8_t>(instr.stack_size))
    ->sub(tmp3, tmp2)
    
    ->bshr(tmp1, tmp3)
    ->bor(tmp0, tmp1)
    
    ->te(FLAG_ZF, tmp0, 0)
    ->tl(FLAG_SF, tmp0, 0)
    ->mov(FLAG_OF, vtil::UNDEFINED)
    ->mov(FLAG_CF, vtil::UNDEFINED)

    ->push(tmp0)
    ->pushf();
}

void vm::lifter_t::lifter_svsp(vm::instrs::vinstr_t instr)
{
  auto tmp = current_block->tmp(64);
  current_block
    ->ldd(tmp, vtil::REG_SP, 0)
    ->mov(vtil::REG_SP, tmp);
}

void vm::lifter_t::lifter_write(vm::instrs::vinstr_t instr)
{
  auto [tmp0, tmp1] = current_block->tmp(64, instr.stack_size);
  current_block
    ->pop(tmp0)
    ->pop(tmp1)
    ->str(tmp0, 0, tmp1); 
}

void vm::lifter_t::lifter_writedr7(vm::instrs::vinstr_t instr)
{
  auto tmp = current_block->tmp(64);
  current_block
    ->pop(tmp)
    ->mov(X86_REG_DR7, tmp);
}

void vm::lifter_t::lifter_vmexit(vm::instrs::vinstr_t instr)
{
  for (auto it = vmentry_pushes.rbegin(); it != vmentry_pushes.rend(); it++)
    current_block->pop(*it);
  auto tmp = current_block->tmp(64);
  current_block
    ->pop(tmp)
    ->vexit(tmp);
};