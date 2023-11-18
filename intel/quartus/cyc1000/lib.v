// registers so you don't gotta use inference (from EE151)

module register #(
	parameter N = 1
) (
	input      [N-1:0] d,
	output reg [N-1:0] q,
	input clk
);
initial q = {N{1'b0}};
always @(posedge clk)
	q <= d;
endmodule

module register_ce #(
	parameter N = 1
) (
	input      [N-1:0] d,
	output reg [N-1:0] q,
	input ce, clk
);
initial q = {N{1'b0}};
always @(posedge clk)
	if (ce)
		q <= d;
endmodule

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

// sychronizer for async inputs

module synchronizer #(
	parameter N = 1
) (
	input  [N-1:0] in,
	output [N-1:0] out,
	input clk
); // using 2 FFs

wire [N-1:0] mid;

register #(.N(N)) f1 (.d(in),  .q(mid), .clk(clk));
register #(.N(N)) f2 (.d(mid), .q(out), .clk(clk));

endmodule

