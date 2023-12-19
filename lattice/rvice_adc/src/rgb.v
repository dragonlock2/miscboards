module RGB #(
    parameter PERIOD = 1000,
    parameter WIDTH  = $clog2(PERIOD),
) (
    input rst, clk,
    input [WIDTH-1:0] r,
    input [WIDTH-1:0] g,
    input [WIDTH-1:0] b,
    output [2:0] rgb,
);

// period counter
wire [WIDTH-1:0] ctr;
wire overflow = ctr == PERIOD - 1;

register_r #(.N(WIDTH)) ctr_reg (
    .d(overflow ? 0 : ctr + 1),
    .q(ctr),
    .rst(rst),
    .clk(clk),
);

// duty buffers
wire [WIDTH-1:0] buf_r, buf_g, buf_b;

register_r_ce #(.N(WIDTH)) buf_r_reg ( .d(r), .q(buf_r), .ce(overflow), .rst(rst), .clk(clk), );
register_r_ce #(.N(WIDTH)) buf_g_reg ( .d(g), .q(buf_g), .ce(overflow), .rst(rst), .clk(clk), );
register_r_ce #(.N(WIDTH)) buf_b_reg ( .d(b), .q(buf_b), .ce(overflow), .rst(rst), .clk(clk), );

// output match
wire out_r, out_g, out_b;

wire out_r_nonzero = buf_r != 0;
wire out_g_nonzero = buf_g != 0;
wire out_b_nonzero = buf_b != 0;

wire out_r_match = ctr == buf_r;
wire out_g_match = ctr == buf_g;
wire out_b_match = ctr == buf_b;

register_r_ce out_r_reg ( .d(overflow & out_r_nonzero), .q(out_r), .ce(overflow | out_r_match), .rst(rst), .clk(clk), );
register_r_ce out_g_reg ( .d(overflow & out_g_nonzero), .q(out_g), .ce(overflow | out_g_match), .rst(rst), .clk(clk), );
register_r_ce out_b_reg ( .d(overflow & out_b_nonzero), .q(out_b), .ce(overflow | out_b_match), .rst(rst), .clk(clk), );

// driver IP
wire ledpu;

SB_LED_DRV_CUR LED_DRV_CUR (
    .EN(1'b1),
    .LEDPU(ledpu)
);

SB_RGB_DRV #(
    .RGB0_CURRENT("0b000001"), // 4mA
    .RGB1_CURRENT("0b000011"), // 8mA
    .RGB2_CURRENT("0b001111"), // 16mA
) RGB_DRV (
    .RGB0(rgb[0]),
    .RGB1(rgb[1]),
    .RGB2(rgb[2]),
    .RGBLEDEN(1'b1),
    .RGB0PWM(out_g),
    .RGB1PWM(out_b),
    .RGB2PWM(out_r),
    .RGBPU(ledpu)
);

endmodule
