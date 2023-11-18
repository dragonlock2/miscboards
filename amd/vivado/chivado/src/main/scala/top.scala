import chisel3.stage.ChiselStage

object Top extends App {
  (new ChiselStage).emitVerilog(new test.Test, args)
}
