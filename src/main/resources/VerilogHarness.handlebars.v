module test;
{{#each ins}}
    reg[{{msb}}:0] {{name}} = 0;{{/each}}
{{#each outs}}
    wire[{{msb}}:0] {{name}};{{/each}}
{{#each clocks}}
    reg {{name}} = 0;{{/each}}
{{#each resets}}
    reg {{name}} = 1;{{/each}}
{{#each clocks}}
    integer {{name}}_len;
    always #{{name}}_len {{name}} = ~{{name}};{{/each}}
{{#clocks.1}}
    integer min = 1 << 31 - 1;
{{#each clocks}}
    integer {{name}}_cnt;{{/each}}
{{/clocks.1}}

    /*** DUT instantiation ***/
    {{m.moduleName}} {{m.name}}(
        {{#each clocks}}.{{name}}({{name}}),
        {{/each}}
        {{#each resets}}.{{name}}({{name}}),
        {{/each}}
        {{#each ins}}.{{name}}({{name}}),
        {{/each}}
        {{#each outs}}.{{name}}({{name}}){{^last}}, {{/last}}
        {{/each}});

    initial begin
        {{#each clocks}}
        {{name}}_len = {{period}};
        {{/each}}

        {{#clocks.1}}
        {{#each clocks}}
        {{name}}_cnt = {{name}}_len;
        {{/each}}
        {{/clocks.1}}
        $init_clks({{#each clocks}}{{name}}_len{{^last}}, {{/last}}{{/each}});
        $init_rsts({{#each resets}}{{name}}{{^last}}, {{/last}}{{/each}});
        $init_ins({{#each ins}}{{name}}{{^last}}, {{/last}}{{/each}});
        $init_outs({{#each outs}}{{name}}{{^last}}, {{/last}}{{/each}});
        $init_sigs({{m.name}});

        {{#each driver.vcd}}
        {{{.}}}{{/each}}
    end

    {{#clocks.1}}
    initial forever begin
        {{#each clocks}}
        if ({{name}}_cnt < min) min = {{name}}_cnt;
        {{name}}_cnt = {{name}}_cnt - min;
        if ({{name}}_cnt == 0) {{name}}_cnt = {{name}}_len;{{/each}}
        #min $tick();
        #min ;
    end
    {{/clocks.1}}
    {{^clocks.1}}
    always @(negedge {{mainClk}}) begin
        $tick();
    end
    {{/clocks.1}}
endmodule