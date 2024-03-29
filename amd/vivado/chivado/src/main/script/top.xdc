set_property CFGBVS VCCO [current_design]
set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 4 [current_design]

create_clock -period 41.667 [get_ports clock]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN R13} [get_ports clock]

# reset unused
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN L10} [get_ports reset]
set_property PULLDOWN TRUE [get_ports reset]

set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN N12} [get_ports io_led[7]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN M13} [get_ports io_led[6]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN L12} [get_ports io_led[5]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN P14} [get_ports io_led[4]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN N14} [get_ports io_led[3]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN M14} [get_ports io_led[2]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN L14} [get_ports io_led[1]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN K14} [get_ports io_led[0]]

set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN N13} [get_ports io_btn[7]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN L13} [get_ports io_btn[6]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN P15} [get_ports io_btn[5]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN N15} [get_ports io_btn[4]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN M15} [get_ports io_btn[3]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN L15} [get_ports io_btn[2]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN K15} [get_ports io_btn[1]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN J15} [get_ports io_btn[0]]

set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN R12} [get_ports io_tx[0]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN R7 } [get_ports io_tx[1]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN N6 } [get_ports io_tx[2]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN P12} [get_ports io_rx[0]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN R8 } [get_ports io_rx[1]]
set_property -dict {IOSTANDARD LVCMOS33 PACKAGE_PIN N7 } [get_ports io_rx[2]]
