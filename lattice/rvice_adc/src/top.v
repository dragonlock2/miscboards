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
localparam FLUSH_RATE      = 1_000;
localparam NUM_CHANNELS    = 8;
localparam BITS_PER_SAMPLE = 16;

localparam FLUSH_CTR_MAX   = SAMPLE_RATE / FLUSH_RATE - 1;
localparam FLUSH_CTR_WIDTH = $clog2(SAMPLE_RATE / FLUSH_RATE);
localparam SAMPLE_WIDTH    = NUM_CHANNELS * BITS_PER_SAMPLE;

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

/* fifo */
wire [SAMPLE_WIDTH-1:0] sample_data;
wire sample_valid, sample_error;

basic_fifo #(
    .WIDTH(SAMPLE_WIDTH),
) sample_fifo (
    .rst(rst),
    .clk(clk),
    .in(sample_data),
    .in_valid(sample_valid),
    .out(spi_data),
    .out_ready(spi_ready),
    .error(sample_error),
);

/* adc */
// TODO stream samples into fifo

// assert cs when 1ms of samples pushed
wire [FLUSH_CTR_WIDTH-1:0] flush_ctr, flush_ctr_next;
wire flush_ctr_done, flush_ctr_ce;
register_r_ce #(.N(FLUSH_CTR_WIDTH)) flush_ctr_reg ( .d(flush_ctr_next), .q(flush_ctr), .ce(flush_ctr_ce), .rst(rst), .clk(clk), );

assign flush_ctr_done = flush_ctr == FLUSH_CTR_MAX;
assign flush_ctr_next = flush_ctr_done ? 0 : (flush_ctr + 1);
assign flush_ctr_ce   = sample_valid;

wire resync_next;
register_r resync_reg ( .d(resync_next), .q(resync), .rst(rst), .clk(clk), );

assign resync_next = flush_ctr_done && flush_ctr_ce;
assign cs1 = resync;

/* rgb status indicator */
wire [7:0] err_ctr, err_ctr_next;
wire err_ctr_ce;
register_r_ce #(.N(8), .INIT(10)) err_ctr_reg ( .d(err_ctr_next), .q(err_ctr), .ce(err_ctr_ce), .rst(rst), .clk(clk), );
assign err_ctr_next = err_ctr + 1;
assign err_ctr_ce   = sample_error;

wire [31:0] rgb_ctr, rgb_ctr_next;
register_r #(.N(32)) rgb_ctr_reg ( .d(rgb_ctr_next), .q(rgb_ctr), .rst(rst), .clk(clk), );
assign rgb_ctr_next = rgb_ctr == (CLK_FREQ / 2 - 1) ? 0 : (rgb_ctr + 1);

wire [2:0] rgb_shift, rgb_shift_next;
wire rgb_shift_ce;
register_r_ce #(.N(3), .INIT(3'b100)) rgb_shift_reg ( .d(rgb_shift_next), .q(rgb_shift), .ce(rgb_shift_ce), .rst(rst), .clk(clk), );
assign rgb_shift_next = { rgb_shift[1:0], rgb_shift[2] };
assign rgb_shift_ce   = rgb_ctr == 0;

RGB rgb_pwm (
    .rst(rst),
    .clk(clk),
    .r(rgb_shift[0] ? err_ctr : 0),
    .g(rgb_shift[1] ? err_ctr : 0),
    .b(rgb_shift[2] ? err_ctr : 0),
    .rgb(rgb),
);


// TODO remove
wire [31:0] audio_ctr, audio_ctr_next;
register_r #(.N(32)) audio_ctr_reg ( .d(audio_ctr_next), .q(audio_ctr), .rst(rst), .clk(clk), );
assign audio_ctr_next = audio_ctr == (CLK_FREQ / SAMPLE_RATE - 1) ? 0 : (audio_ctr + 1);

assign sample_valid = audio_ctr == 0;

wire [SAMPLE_WIDTH-1:0] sample_data_next;
register_r_ce #(.N(SAMPLE_WIDTH)) sample_reg ( .d(sample_data_next), .q(sample_data), .ce(sample_valid), .rst(rst), .clk(clk), );

genvar i;
generate for (i = 0; i < NUM_CHANNELS; i = i + 1) begin
    assign sample_data_next[(16*i + 15):(16*i)] = sample_data[(16*i + 15):(16*i)] + (2*i);
end endgenerate

endmodule
