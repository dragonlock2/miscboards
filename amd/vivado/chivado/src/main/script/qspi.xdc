set_property CFGBVS VCCO [current_design]
set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 4 [current_design]
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]

set_property -dict {PACKAGE_PIN L11 IOSTANDARD LVCMOS33} [get_ports cs]
set_property -dict {PACKAGE_PIN H14 IOSTANDARD LVCMOS33} [get_ports d0]
set_property -dict {PACKAGE_PIN H15 IOSTANDARD LVCMOS33} [get_ports d1]
set_property -dict {PACKAGE_PIN J12 IOSTANDARD LVCMOS33} [get_ports d2]
set_property -dict {PACKAGE_PIN K13 IOSTANDARD LVCMOS33} [get_ports d3]