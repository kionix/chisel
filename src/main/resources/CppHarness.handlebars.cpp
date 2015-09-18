#include "V{{m.moduleName}}.h"
#include "vl.h"
{{#driver.isVCD?}}
#include "verilated_vcd_c.h"{{/driver.isVCD?}}

vluint64_t main_time = 0;

int main(int argc, char **argv, char **env) {
    Verilated::commandArgs(argc, argv);
    auto top = new V{{m.moduleName}}();
    auto vl_api = new vl_api_t<V{{m.moduleName}}>(top);

{{#each ins}}
    vl_api->add_input(top->{{name}}, {{width}});{{/each}}

{{#each outs}}
    vl_api->add_output(top->{{name}}, {{width}});{{/each}}

{{#each resets}}
    vl_api->add_reset(top->{{name}});{{/each}}

{{#driver.isVCD?}}
    Verilated::traceEverOn(true);
    VerilatedVcdC *tfp = new VerilatedVcdC;
    top->trace(tfp, 0);
    tfp->open("{{driver.targetDir}}/V{{m.moduleName}}.vcd");{{/driver.isVCD?}}

    while (!Verilated::gotFinish()) {
        vl_api->tick();
{{#driver.isVCD?}}
        tfp->dump(main_time);{{/driver.isVCD?}}
        main_time++;
    }

    top->final();
{{#driver.isVCD?}}
    tfp->close();{{/driver.isVCD?}}
    delete vl_api;
    delete top;
    return 0;
}