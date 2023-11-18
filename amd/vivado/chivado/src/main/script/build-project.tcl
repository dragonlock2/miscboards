if {$argc != 6} {
    puts "Expected: <part num> <proj name> <top module> <top filename> <xdc filename> <num cpu>"
    exit
}

set part_num  [lindex $argv 0]
set proj_name [lindex $argv 1]
set top_mod   [lindex $argv 2]
set top_file  [lindex $argv 3]
set xdc_file  [lindex $argv 4]
set num_cpu   [lindex $argv 5]

create_project $proj_name $proj_name -part $part_num
update_ip_catalog

source ip.tcl

add_files -fileset constrs_1 -norecurse $xdc_file
foreach source [glob -directory "verilog" -- "*.v"] {
    add_files -norecurse $source
}

import_files -force -norecurse
import_files -fileset constrs_1 -force -norecurse $xdc_file

set_property top $top_mod [current_fileset]
set_property top_file $top_file [current_fileset]
update_compile_order -fileset sources_1

launch_runs synth_1 -jobs $num_cpu
wait_on_run synth_1
launch_runs impl_1 -to_step write_bitstream -jobs $num_cpu
wait_on_run impl_1
