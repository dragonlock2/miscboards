module top (
    input mco,
    output [2:0] rgb,

    input  mosi1,
    output miso1,
    input  sck1,
    output cs1,
);

localparam CLK_FREQ = 36_000_000;

/* clock gen */
wire lock, rst, clk;

SB_PLL40_CORE #( // icepll -i 72 -o 36
    .FEEDBACK_PATH("SIMPLE"),
    .PLLOUT_SELECT("GENCLK"),
    .DIVR(4'b0000),
    .DIVF(7'b0000111),
    .DIVQ(3'b100),
    .FILTER_RANGE(3'b101),
) uut (
    .RESETB(1'b1),
    .BYPASS(1'b0),
    .REFERENCECLK(mco),
    .PLLOUTCORE(clk),
    .LOCK(lock),
);

assign rst = ~lock;

// TODO use hardened SPI IP
assign miso1 = mosi1;

// TODO assert signal when >=1ms of samples, flush entire fifo if full just in case
    // signal also resyncs bits
// TODO use block RAM to make 8 FIFOs

// temporary 1kHz cs signal
reg cs = 0;
reg [31:0] ctr2 = 0;

always @(posedge clk) begin
    if (ctr2 == (CLK_FREQ / 2000) - 1) begin
        cs = ~cs;
        ctr2 = 0;
    end else
        ctr2 = ctr2 + 1;
end

assign cs1 = cs;

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
