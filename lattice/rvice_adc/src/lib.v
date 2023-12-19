// explicitly define registers, taken from EE151

module register_r #(
	parameter N    = 1,
	parameter INIT = {N{1'b0}}
) (
	input      [N-1:0] d,
	output reg [N-1:0] q,
	input rst, clk
);
initial q = INIT;
always @(posedge clk)
	if (rst)
		q <= INIT;
	else
		q <= d;
endmodule

module register_r_ce #(
	parameter N    = 1,
	parameter INIT = {N{1'b0}}
) (
	input      [N-1:0] d,
	output reg [N-1:0] q,
	input rst, ce, clk
);
initial q = INIT;
always @(posedge clk)
	if (rst)
		q <= INIT;
	else if (ce)
		q <= d;
endmodule
