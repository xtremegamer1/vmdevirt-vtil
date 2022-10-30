#include <zyreg_to_register_desc.hpp> //It is very important this be included before vtil/vtil so that the cast can be visible to vtil
#include <vtil\vtil>
#include <vmemu_t.hpp>
#include <vector>
#include <functional>

namespace vm
{
namespace lifters
{  
using fn_lifter = std::function<void(::vtil::basic_block*, ::vm::instrs::vinstr_t v_instr)>;
static constexpr vtil::register_desc FLAG_CF = vtil::REG_FLAGS.select( 1, 0 );
static constexpr vtil::register_desc FLAG_PF = vtil::REG_FLAGS.select( 1, 2 );
static constexpr vtil::register_desc FLAG_AF = vtil::REG_FLAGS.select( 1, 4 );
static constexpr vtil::register_desc FLAG_ZF = vtil::REG_FLAGS.select( 1, 6 );
static constexpr vtil::register_desc FLAG_SF = vtil::REG_FLAGS.select( 1, 7 );
static constexpr vtil::register_desc FLAG_DF = vtil::REG_FLAGS.select( 1, 10 );
static constexpr vtil::register_desc FLAG_OF = vtil::REG_FLAGS.select( 1, 11 );
}
class lifter_t
{
  public:
  lifter_t(const vm::instrs::vrtn_t* const routine, const vm::vmctx_t* const ctx);
  bool lift();
  const vtil::routine& get_routine();  
  private:
  const std::uintptr_t load_delta;
  const std::array<ZydisRegister, 16> vmentry_pushes;
  const vm::instrs::vrtn_t* const vmp_routine;
  vtil::routine* vtil_routine;
  vtil::basic_block* current_block;
  uint64_t reg_id = 0;
  vtil::register_desc* scratch_regs[32];
  void lift_handler(vm::instrs::vinstr_t v_instr);
};
}