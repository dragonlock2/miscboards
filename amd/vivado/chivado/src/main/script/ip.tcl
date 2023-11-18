# Clocking Wizard (clk_wiz_0)
create_ip -name clk_wiz -vendor xilinx.com -library ip -version 6.0 -module_name clk_wiz_0
set_property -dict [list CONFIG.Component_Name {clk_wiz_0} \
                         CONFIG.ENABLE_CLOCK_MONITOR {false} \
                         CONFIG.PRIM_IN_FREQ {24.000} \
                         CONFIG.PRIMITIVE {MMCM} \
                         CONFIG.CLKIN1_JITTER_PS {416.66} \
                         CONFIG.MMCM_CLKFBOUT_MULT_F {40.625} \
                         CONFIG.MMCM_CLKIN1_PERIOD {41.667} \
                         CONFIG.MMCM_CLKIN2_PERIOD {10.0} \
                         CONFIG.MMCM_CLKOUT0_DIVIDE_F {9.750} \
                         CONFIG.CLKOUT1_JITTER {235.888} \
                         CONFIG.CLKOUT1_PHASE_ERROR {245.431} \
                         CONFIG.USE_LOCKED {false} \
                         CONFIG.USE_RESET {false}] \
                         [get_ips clk_wiz_0]

generate_target all [get_ips]
