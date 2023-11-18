// adapted from https://github.com/trabucayre/openFPGALoader
module qspi(
  output cs,
  output d0,
  input  d1,
  output d2,
  output d3
);

  wire capture, drck, sel, update;
  wire runtest;
  wire tdi;
  reg fsm_csn;

  assign cs = fsm_csn;
  assign d0 = tdi;
  assign d2 = 1'b1;
  assign d3 = 1'b1;
  wire tdo = (sel) ? d1 : tdi;

  wire tmp_cap_s = capture && sel;
  wire tmp_up_s = update && sel;

  always @(posedge drck, posedge runtest) begin
    if (runtest) begin
      fsm_csn <= 1'b1;
    end else begin
      if (tmp_cap_s) begin
        fsm_csn <= 1'b0;
      end else if (tmp_up_s) begin
        fsm_csn <= 1'b1;
      end else begin
        fsm_csn <= fsm_csn;
      end
    end
  end

  STARTUPE2 #(
    .PROG_USR("FALSE"),  // Activate program event security feature. Requires encrypted bitstreams.
    .SIM_CCLK_FREQ(0.0)  // Set the Configuration Clock Frequency(ns) for simulation.
  ) startupe2_inst (
    .CFGCLK   (),     // 1-bit output: Configuration main clock output
    .CFGMCLK  (),     // 1-bit output: Configuration internal oscillator clock output
    .EOS      (),     // 1-bit output: Active high output signal indicating the End Of Startup.
    .PREQ     (),     // 1-bit output: PROGRAM request to fabric output
    .CLK      (1'b0), // 1-bit input: User start-up clock input
    .GSR      (1'b0), // 1-bit input: Global Set/Reset input (GSR cannot be used for the port name)
    .GTS      (1'b0), // 1-bit input: Global 3-state input (GTS cannot be used for the port name)
    .KEYCLEARB(1'b0), // 1-bit input: Clear AES Decrypter Key input from Battery-Backed RAM (BBRAM)
    .PACK     (1'b1), // 1-bit input: PROGRAM acknowledge input
    .USRCCLKO (drck), // 1-bit input: User CCLK input
    .USRCCLKTS(1'b0), // 1-bit input: User CCLK 3-state enable input
    .USRDONEO (1'b1), // 1-bit input: User DONE pin output control
    .USRDONETS(1'b1)  // 1-bit input: User DONE 3-state enable output
  );

  BSCANE2 #(
    .JTAG_CHAIN(1)  // Value for USER command.
  ) bscane2_inst (
    .CAPTURE(capture), // 1-bit output: CAPTURE output from TAP controller.
    .DRCK   (drck),    // 1-bit output: Gated TCK output. When SEL
               //               is asserted, DRCK toggles when
               //               CAPTURE or SHIFT are asserted.
    .RESET  (),        // 1-bit output: Reset output for TAP controller.
    .RUNTEST(runtest), // 1-bit output: Output asserted when TAP
               //               controller is in Run Test/Idle state.
    .SEL     (sel),    // 1-bit output: USER instruction active output.
    .SHIFT   (),       // 1-bit output: SHIFT output from TAP controller.
    .TCK     (),       // 1-bit output: Test Clock output.
               //               Fabric connection to TAP Clock pin.
    .TDI     (tdi),    // 1-bit output: Test Data Input (TDI) output
               //               from TAP controller.
    .TMS     (),       // 1-bit output: Test Mode Select output.
               //               Fabric connection to TAP.
    .UPDATE  (update), // 1-bit output: UPDATE output from TAP controller
    .TDO     (tdo)     // 1-bit input: Test Data Output (TDO) input
               //              for USER function.
  );

endmodule
