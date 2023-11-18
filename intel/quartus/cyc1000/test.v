// Copyright (C) 2020  Intel Corporation. All rights reserved.
// Your use of Intel Corporation's design tools, logic functions 
// and other software and tools, and any partner logic 
// functions, and any output files from any of the foregoing 
// (including device programming or simulation files), and any 
// associated documentation or information are expressly subject 
// to the terms and conditions of the Intel Program License 
// Subscription Agreement, the Intel Quartus Prime License Agreement,
// the Intel FPGA IP License Agreement, or other applicable license
// agreement, including, without limitation, that your use is for
// the sole purpose of programming logic devices manufactured by
// Intel and sold by Intel or its authorized distributors.  Please
// refer to the applicable agreement for further details, at
// https://fpgasoftware.intel.com/eula.

module test (
// {ALTERA_ARGS_BEGIN} DO NOT REMOVE THIS LINE!
input        btn,
output [7:0] led,

output uart_tx,
input  uart_rx,

input raw_clk // 12MHz
// {ALTERA_ARGS_END} DO NOT REMOVE THIS LINE!

);

// {ALTERA_IO_BEGIN} DO NOT REMOVE THIS LINE!
// {ALTERA_IO_END} DO NOT REMOVE THIS LINE!
// {ALTERA_MODULE_BEGIN} DO NOT REMOVE THIS LINE!

wire rst;
synchronizer rst_sync(.in(~btn), .out(rst), .clk(clk));

wire clk;
pll p(raw_clk, clk); // 100MHz

// uart
wire uart_tx_ready, uart_tx_valid;
wire uart_rx_ready, uart_rx_valid;

wire [7:0] uart_tx_data, uart_rx_data;

uart #(
	.BAUD_RATE(3_000_000)
) serial (
	.TX(uart_tx),
	.RX(uart_rx),
	.tx_data(uart_tx_data),
	.rx_data(uart_rx_data),
	.tx_ready(uart_tx_ready),
	.tx_valid(uart_tx_valid),
	.rx_ready(uart_rx_ready),
	.rx_valid(uart_rx_valid),
	.rst(rst),
	.clk(clk)
);

wire [7:0] fifo_out;
wire fifo_full, fifo_empty;

fifo serial_buff( // 1024 bytes deep
	.data(uart_rx_data),
	.rdreq(uart_tx_ready),
	.wrreq(uart_rx_valid),
	.empty(fifo_empty),
	.full(fifo_full),
	.q(fifo_out),
	.sclr(rst),
	.clock(clk)
);

assign uart_tx_valid = ~fifo_empty;
assign uart_rx_ready = ~fifo_full;

assign uart_tx_data  = ((fifo_out >= 65 && fifo_out <= 90) || (fifo_out >= 97 && fifo_out <= 122)) ? fifo_out ^ 8'h20 : fifo_out;
assign led[6:0] = uart_tx_data[6:0];

// flasher

wire [31:0] counter_val, counter_nxt;
wire        counter_rst;

register_r #(.N(32)) counter_reg (.d(counter_nxt), .q(counter_val), .rst(counter_rst), .clk(clk));

wire flash_val, flash_nxt, flash_ce;

register_r_ce flash_reg (.d(flash_nxt), .q(flash_val), .ce(flash_ce), .rst(rst), .clk(clk));

assign counter_nxt = counter_val + 1;
assign counter_rst = rst | counter_val == (25_000_000 - 1);
assign flash_nxt   = ~flash_val;
assign flash_ce    = counter_rst;

assign led[7] = flash_val;

// {ALTERA_MODULE_END} DO NOT REMOVE THIS LINE!
endmodule
