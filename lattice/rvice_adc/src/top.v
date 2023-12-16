module top (
    input raw_clk,
    output [2:0] led,

    input  mosi1,
    output miso1,
    input  sck1,
    output cs1,
);

localparam FREQ = 8_000_000;

wire clk = raw_clk; // TODO PLL at 36MHz? 24MHz?

// TODO use hardened SPI IP
// TODO assert signal when >=1ms of samples, flush entire fifo if full just in case
    // signal also resyncs bits
// TODO use block RAM to make 8 FIFOs
assign miso1 = mosi1;

// temporary 1kHz cs signal
reg cs = 0;
reg [31:0] ctr2 = 0;

always @(posedge clk) begin
    if (ctr2 == (FREQ / 2000) - 1) begin
        cs = ~cs;
        ctr2 = 0;
    end else
        ctr2 = ctr2 + 1;
end

assign cs1 = cs;

// RGB testing
reg [2:0] shift = 3'b100;
reg [31:0] ctr = 0;

always @(posedge clk) begin
    if (ctr == (FREQ * 1 / 2) - 1) begin
        shift = {shift[1:0], shift[2]};
        ctr = 0;
    end else
        ctr = ctr + 1;
end

wire ledpu;

SB_LED_DRV_CUR LED_DRV_CUR (
    .EN(1'b1),
    .LEDPU(ledpu)
);

SB_RGB_DRV #(
    .RGB0_CURRENT("0b000001"), // 4mA
    .RGB1_CURRENT("0b000001"),
    .RGB2_CURRENT("0b000001"),
) RGB_DRV (
    .RGB0(led[0]),
    .RGB1(led[1]),
    .RGB2(led[2]),
    .RGBLEDEN(1'b1),
    .RGB0PWM(shift[0]),
    .RGB1PWM(shift[1]),
    .RGB2PWM(shift[2]),
    .RGBPU(ledpu)
);

endmodule
