module spi #(
    parameter SAMPLE_WIDTH = 16,
) (
    input  rst, clk,
    output ready,
    input  resync,
    input [SAMPLE_WIDTH-1:0] data,

    input  mosi,
    output miso,
    input  sck,
);

localparam COUNTER_WIDTH = $clog2(SAMPLE_WIDTH);

/*
 * Need to shift out on sck falling edge. However, clock speed too low to have
 * input buffer and achieve latency requirements. Instead, we shift out on rising edge which
 * should still satisfy t_hold since it doesn't happen until next clock edge after rising edge
 * is detected.
 */
wire sck_buf, sck_prev;
wire sck_shift = !sck_prev && sck_buf;

register_r sck_buf_reg ( .d(sck),     .q(sck_buf),  .rst(rst), .clk(clk), );
register_r sck_reg     ( .d(sck_buf), .q(sck_prev), .rst(rst), .clk(clk), );

/*
 * Counts the number of bits already shifted out. If in the middle of a word when resync asserted,
 * then dump current word and pull next one in.
 */
wire [COUNTER_WIDTH-1:0] ctr, ctr_next;
wire ctr_ce, ctr_done;

register_r_ce #(.N(COUNTER_WIDTH)) ctr_reg ( .d(ctr_next), .q(ctr), .ce(ctr_ce), .rst(rst), .clk(clk), );

assign ctr_next = ctr_done ? (SAMPLE_WIDTH - 1) : (ctr - 1);
assign ctr_ce   = resync || sck_shift;
assign ctr_done = resync || (ctr == 0);

/*
 * Stores bits to be outputted.
 */
wire [SAMPLE_WIDTH-1:0] out, out_next;
wire out_ce;

register_r_ce #(.N(SAMPLE_WIDTH)) out_reg ( .d(out_next), .q(out), .ce(out_ce), .rst(rst), .clk(clk), );

assign out_next = ctr_done ? data : (out >> 1);
assign out_ce   = ready || sck_shift;

assign ready = (resync && (ctr != SAMPLE_WIDTH - 1)) || ((ctr == 0) && sck_shift);
assign miso  = out[0];

endmodule
