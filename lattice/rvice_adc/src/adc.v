module adc #(
    parameter CLK_FREQ    = 1_000_000,
    parameter SAMPLE_RATE = 1_000,
) (
    input rst, clk,
    output [127:0] data,
    output data_valid,
    input [7:0] sdo,
    output clkout, cnv,
);

// state registers
localparam STATE_ACQ  = 3'b001;
localparam STATE_CONV = 3'b010;
localparam STATE_READ = 3'b100;

wire [2:0] state, state_next;
register_r #(.N(3), .INIT(STATE_ACQ)) state_reg ( .d(state_next), .q(state), .rst(rst), .clk(clk), );

wire clkout_next, cnv_next;
register_r clkout_reg ( .d(clkout_next), .q(clkout), .rst(rst), .clk(clk), );
register_r cnv_reg ( .d(cnv_next), .q(cnv), .rst(rst), .clk(clk), );

wire [127:0] data_next;
register_r #(.N(128)) data_reg ( .d(data_next), .q(data), .rst(rst), .clk(clk), );

// sample rate trigger
localparam RATE_CTR_MAX   = CLK_FREQ / SAMPLE_RATE - 1;
localparam RATE_CTR_WIDTH = $clog2(CLK_FREQ / SAMPLE_RATE);

wire [RATE_CTR_WIDTH-1:0] rate_ctr, rate_ctr_next;
wire start_conv;
register_r #(.N(RATE_CTR_WIDTH)) rate_ctr_reg ( .d(rate_ctr_next), .q(rate_ctr), .rst(rst), .clk(clk), );
assign rate_ctr_next = start_conv ? 0 : (rate_ctr + 1);
assign start_conv    = rate_ctr == RATE_CTR_MAX;

// conversion timer
localparam CONV_CTR_MAX   = (CLK_FREQ / 1_000_000) * 450 / 1_000 + 4; // >450ns (careful of integer overflow!)
localparam CONV_CTR_WIDTH = $clog2(CONV_CTR_MAX + 1);

wire [CONV_CTR_WIDTH-1:0] conv_ctr, conv_ctr_next;
register_r #(.N(CONV_CTR_WIDTH)) conv_ctr_reg ( .d(conv_ctr_next), .q(conv_ctr), .rst(rst), .clk(clk), );

// bit counter
wire [4:0] bit_ctr, bit_ctr_next;
register_r #(.N(5)) bit_ctr_reg ( .d(bit_ctr_next), .q(bit_ctr), .rst(rst), .clk(clk), );

// state logic
integer i;

always @(*) begin
    data_next     = data;
    data_valid    = 0;
    state_next    = state;
    conv_ctr_next = conv_ctr;
    bit_ctr_next  = bit_ctr;
    clkout_next   = 0;
    cnv_next      = 0;

    case (state)
        STATE_ACQ: begin
            if (start_conv) begin
                state_next    = STATE_CONV;
                conv_ctr_next = 0;
            end else begin
                cnv_next = 1;
            end
        end

        STATE_CONV: begin
            conv_ctr_next = conv_ctr + 1;
            if (conv_ctr == CONV_CTR_MAX) begin
                bit_ctr_next = 0;
                state_next   = STATE_READ;
            end
        end

        STATE_READ: begin
            if (clkout == 0) begin
                if (bit_ctr == 16) begin
                    data_valid = 1;
                    state_next = STATE_ACQ;
                end else begin
                    for (i = 0; i < 8; i = i + 1) begin
                        data_next[(16*i + 15):(16*i)] = { data_next[(16*i + 14):(16*i)], sdo[i] };
                    end

                    bit_ctr_next = bit_ctr + 1;
                    clkout_next  = 1;
                end
            end
        end
    endcase
end

endmodule
