@(
moduleName: String,
ins: Array[(String,Int)],
outs: Array [(String,Int)],
mainClk: String,
clocks: Array[String],
resets: Array[String],
isVcd: Boolean
)

#include "V@{moduleName}.h"
#include "verilated_vcd_c.h"
#include "vl.h"

#include <iostream>


using namespace std;

vluint64_t main_time = 0;

int main(int argc, char **argv, char **env) {
  Verilated::commandArgs(argc, argv);
  auto top = new V@{moduleName }
  ();
  auto vl_api = new vl_api_t<V@{moduleName }>(top);

  top->eval();

  @for((in, w) <-ins) {
    vl_api->add_input(top->@{in}, @{w}, "@{in}");
  }

  @for((out, w) <-outs) {
    vl_api->add_output(top->@{out}, @{w}, "@{out}");
  }

  @for(rst <-resets) {
    vl_api->add_reset(top->@{rst}, "@{rst}");
  }


  @if(isVcd) {
    Verilated::traceEverOn(true);
    VerilatedVcdC *tfp = new VerilatedVcdC;
    top->trace(tfp, 0);
    tfp->open("V@{moduleName}.vcd");
  }

  while (!Verilated::gotFinish()) {
    vl_api->tick();
    @if(isVcd) {
      tfp->dump(main_time);
    }
    main_time++;
  }

  top->final();
  @if(isVcd) {
    tfp->close();
  }
  delete vl_api;
  delete top;
  return 0;
}


