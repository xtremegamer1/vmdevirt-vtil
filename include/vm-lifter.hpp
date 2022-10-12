#include <vtil\vtil>
#include <vmemu_t.hpp>
#include <vector>

namespace vm
{
class lifter
{
  public:
  lifter(const vm::instrs::vrtn_t* const routine);
  bool lift();
  const vtil::routine& get_routine();  
  private:
  const vm::instrs::vrtn_t* const vmp_routine;
  vtil::routine* vtil_routine;
  vtil::basic_block* current_block;
  uint64_t reg_id;
  vtil::register_desc* scratch_regs[32];
  void lift_handler(vm::instrs::vinstr_t v_instr);
};
}