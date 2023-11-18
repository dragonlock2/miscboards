create_clock -name raw_clk -period "12MHz" [get_ports raw_clk]

derive_pll_clocks
derive_clock_uncertainty

set_false_path -from [get_ports btn]
set_false_path -from [get_ports uart_rx]

set_false_path -from * -to [get_ports led*]
set_false_path -from * -to [get_ports uart_tx]