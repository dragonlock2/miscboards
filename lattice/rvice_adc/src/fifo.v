module basic_fifo #(
    parameter WIDTH = 16,
) (
    input rst, clk,
    input [WIDTH-1:0] in,
    input in_valid,
    output [WIDTH-1:0] out,
    input out_ready,
    output error,
);

/*
 * Very basic FIFO implementation without full ready/valid. If it becomes empty/full, which
 * shouldn't happen in normal operation, it will reset, output a couple bad samples as buffer,
 * and flag an error. Since EBR needs a clock edge for read and we want to ease timing,
 * don't pull things out in successive cycles.
 */

localparam NUM_EBR = (WIDTH + 16 - 1) / 16;

wire [7:0] read_addr, write_addr;
wire write_enable;

genvar i;
generate for (i = 0; i < NUM_EBR; i = i + 1) begin
    SB_RAM40_4K #(
        .WRITE_MODE(0), // 256x 16-bit elements
        .READ_MODE(0),
    ) ebr (
        .RDATA(out[(16*i + 15):(16*i)]),
        .RADDR(read_addr),
        .WADDR(write_addr),
        .MASK(16'h0000),
        .WDATA(in[(16*i + 15):(16*i)]),
        .RCLKE(1'b1),
        .RCLK(clk),
        .RE(1'b1),
        .WCLKE(1'b1),
        .WCLK(clk),
        .WE(write_enable),
    );
end endgenerate

wire [7:0] in_ptr, in_ptr_next, out_ptr, out_ptr_next;
wire in_ce, out_ce;

register_r_ce #(.N(8)) in_ptr_reg ( .d(in_ptr_next), .q(in_ptr), .ce(in_ce), .rst(rst), .clk(clk), );
register_r_ce #(.N(8)) out_ptr_reg ( .d(out_ptr_next), .q(out_ptr), .ce(out_ce), .rst(rst), .clk(clk), );

assign in_ptr_next = error ? 4 : (in_ptr + 1); // reset with small buffer if error
assign in_ce       = error || in_valid;

assign out_ptr_next = error ? 0 : (out_ptr + 1);
assign out_ce       = error || out_ready;

assign read_addr    = out_ptr;
assign write_addr   = in_ptr;
assign write_enable = in_ce;

assign error = in_ptr == out_ptr; // either full or empty

endmodule
