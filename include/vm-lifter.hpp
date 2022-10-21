#include <vtil\vtil>
#include <vmemu_t.hpp>
#include <vector>

namespace vm
{
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