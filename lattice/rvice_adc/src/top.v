module top (
    input mco,
    output [2:0] rgb,

    input  mosi1,
    output miso1,
    input  sck1,
    output cs1,
);

localparam CLK_FREQ        = 36_000_000;
localparam SAMPLE_RATE     = 48_000;
localparam NUM_CHANNELS    = 8;
localparam BITS_PER_SAMPLE = 16;

localparam SAMPLE_WIDTH = NUM_CHANNELS * BITS_PER_SAMPLE;

/* clock gen */
wire lock, rst, clk;

SB_PLL40_CORE #( // icepll -i 72 -o 36
    .FEEDBACK_PATH("SIMPLE"),
    .PLLOUT_SELECT("GENCLK"),
    .DIVR(4'b0000),
    .DIVF(7'b0000111),
    .DIVQ(3'b100),
    .FILTER_RANGE(3'b101),
) pll (
    .RESETB(1'b1),
    .BYPASS(1'b0),
    .REFERENCECLK(mco),
    .PLLOUTCORE(clk),
    .LOCK(lock),
);

assign rst = ~lock;

/* spi output */
wire [SAMPLE_WIDTH-1:0] spi_data;
wire resync, spi_ready;

spi #(
    .SAMPLE_WIDTH(SAMPLE_WIDTH),
) spi_mod (
    .rst(rst),
    .clk(clk),
    .data(spi_data),
    .ready(spi_ready),
    .resync(resync),
    .mosi(mosi1),
    .miso(miso1),
    .sck(sck1),
);

// TODO use block RAM to make 8-wide FIFO
// TODO assert resync when >=1ms of samples, flush entire fifo if full just in case
// TODO read from adc
wire [SAMPLE_WIDTH-1:0] test0 = {16'd80, 16'd70, 16'd60, 16'd50, 16'd40, 16'd30, 16'd20, 16'd10};
wire [SAMPLE_WIDTH-1:0] test1 = {-16'd80, -16'd70, -16'd60, -16'd50, -16'd40, -16'd30, -16'd20, -16'd10};
register_r_ce #(.N(SAMPLE_WIDTH)) test_reg ( .d(spi_data == test0 ? test1 : test0), .q(spi_data), .ce(spi_ready), .rst(rst), .clk(clk), );

// temporary 1kHz cs signal
wire [31:0] ctr2, ctr2_next;

register_r #(.N(32)) ctr2_reg ( .d(ctr2_next), .q(ctr2), .rst(rst), .clk(clk), );

assign ctr2_next = ctr2 == (CLK_FREQ / 1000 - 1) ? 0 : ctr2 + 1;

assign resync = ctr2 == 0;
assign cs1    = ctr2 == 1; // one cycle delay not strictly necessary

// RGB testing
reg [2:0] shift = 3'b001;
reg [31:0] ctr = 0;

always @(posedge clk) begin
    if (ctr == (CLK_FREQ * 1 / 2) - 1) begin
        shift = {shift[1:0], shift[2]};
        ctr = 0;
    end else
        ctr = ctr + 1;
end

RGB rgb_pwm (
    .rst(rst),
    .clk(clk),
    .r(shift[0] ? 10 : 0),
    .g(shift[1] ? 10 : 0),
    .b(shift[2] ? 10 : 0),
    .rgb(rgb),
);

endmodule
