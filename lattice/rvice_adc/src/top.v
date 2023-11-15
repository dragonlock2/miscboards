module top (
    input clk,
    output [2:0] led
);

localparam FREQ = 8_000_000;

reg [2:0] shift = 3'b100;
reg [31:0] ctr = 0;

always @(posedge clk) begin
    if (ctr == (FREQ * 1 / 4) - 1) begin
        shift = {shift[0], shift[2:1]};
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
