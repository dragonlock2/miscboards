module uart #(
	parameter CLOCK_FREQ = 100_000_000, // Hz
	parameter BAUD_RATE  = 115_200, // bps

	parameter N_DATA  = 8, // number data bits (5-8)
	parameter HAS_PAR = 0, // 0: no parity, 1: yes parity
	parameter TYP_PAR = 0, // 0: even, 1: odd, TODO add mark/space support
	parameter N_STOP  = 1  // number stop bits (1-2)
) (
	output TX,
	input  RX,
	
	input  [N_DATA-1:0] tx_data,
	output [N_DATA-1:0] rx_data,
	
	output tx_ready,
	input  tx_valid,
	input  rx_ready,
	output rx_valid,
	
	input rst,
	input clk
);

localparam integer N = 1 + N_DATA + HAS_PAR + N_STOP; // bits transmitted per transfer

localparam integer BIT_TIME = CLOCK_FREQ / BAUD_RATE;
localparam integer BIT_SAMP = CLOCK_FREQ / BAUD_RATE / 2;
localparam integer BIT_N    = $clog2(BIT_TIME);

// tx
wire [N:0] tx_data_val, tx_data_nxt;
wire       tx_data_rst, tx_data_ce;

register_r_ce #(.N(N+1), .INIT(1)) tx_data_reg ( // extra bit for IDLE
	.d(tx_data_nxt),
	.q(tx_data_val),
	.ce(tx_data_ce),
	.rst(tx_data_rst),
	.clk(clk)
);

wire [BIT_N-1:0] tx_counter_val, tx_counter_nxt;
wire             tx_counter_rst, tx_counter_ce;

register_r_ce #(.N(BIT_N)) tx_counter_reg (
	.d(tx_counter_nxt),
	.q(tx_counter_val),
	.ce(tx_counter_ce),
	.rst(tx_counter_rst),
	.clk(clk)
);

wire tx_fire     = tx_ready & tx_valid;
wire tx_bit_edge = tx_counter_val == (BIT_TIME - 1);
wire tx_parity   = (^tx_data) ^ TYP_PAR;

assign TX       = tx_data_val[0];
assign tx_ready = ~|(tx_data_val[N:1]); // any bit 1, still transmitting

assign tx_data_nxt = tx_ready ? {1'b1, {N_STOP{1'b1}}, {HAS_PAR{tx_parity}}, tx_data, 1'b0} : (tx_data_val >> 1);
assign tx_data_rst = rst;
assign tx_data_ce  = tx_fire | tx_bit_edge;

assign tx_counter_nxt = tx_counter_val + 1;
assign tx_counter_rst = rst | tx_bit_edge;
assign tx_counter_ce  = ~tx_ready;

// rx (idea to constantly shift in bits, resyncing on edges) (silently ignores errors)

wire rx_raw;

synchronizer rx_sync(.in(RX), .out(rx_raw), .clk(clk));

wire rx_raw_prev;

register_r rx_edge_reg(.d(rx_raw), .q(rx_raw_prev), .rst(rst), .clk(clk));

wire [N-1:0] rx_shift_val, rx_shift_nxt;
wire         rx_shift_rst, rx_shift_ce;

register_r_ce #(.N(N), .INIT({N{1'b1}})) rx_shift_reg (
	.d(rx_shift_nxt),
	.q(rx_shift_val),
	.ce(rx_shift_ce),
	.rst(rx_shift_rst),
	.clk(clk)
);

wire [BIT_N-1:0] rx_counter_val, rx_counter_nxt;
wire             rx_counter_rst;

register_r #(.N(BIT_N)) rx_counter_reg (
	.d(rx_counter_nxt),
	.q(rx_counter_val),
	.rst(rx_counter_rst),
	.clk(clk)
);

wire rx_fire     = rx_ready & rx_valid;
wire rx_edge     = rx_raw_prev ^ rx_raw; // resync!
wire rx_bit_edge = rx_counter_val == (BIT_TIME - 1);
wire rx_bit_samp = rx_counter_val == (BIT_SAMP - 1);
wire rx_parity   = ^(rx_shift_val[N_DATA+1:1]);

wire rx_done         = ~rx_shift_val[0];
wire rx_parity_match = HAS_PAR == 0 || rx_parity == TYP_PAR; // parity error
wire rx_stop_valid   = &(rx_shift_val[N-1:N-N_STOP]); // stop bit error
wire rx_no_error     = rx_parity_match & rx_stop_valid;

assign rx_data  = rx_shift_val[N_DATA:1];
assign rx_valid = rx_done & rx_no_error;

assign rx_shift_nxt = {rx_raw, rx_shift_val[N-1:1]};
assign rx_shift_rst = rst | rx_fire | (rx_done & ~rx_no_error) | (rx_valid & (rx_edge | rx_bit_edge)); // ~0.5 bit width to receive word
assign rx_shift_ce  = ~rx_edge & rx_bit_samp;

assign rx_counter_nxt = rx_edge ? 1 : rx_counter_val + 1; // reset to 1 if see edge (only matters at high baud rate)
assign rx_counter_rst = rst | rx_bit_edge;

endmodule
