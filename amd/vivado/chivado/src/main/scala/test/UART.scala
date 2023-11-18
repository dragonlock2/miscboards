package test

import chisel3._
import chisel3.util._

case class UARTConfig(
  clock_freq: Int = 921600, // input clock frequency (Hz)
  baud_rate: Int  = 115200, // baud rate (bps)
  N: Int          = 8,      // # of data bits (5-9)
  parity: String  = "none", // none, odd, even
  stop: Int       = 1,      // # of stop bits (1-2)
)

/* Designed for low hardware cost (aka not runtime configurable).
   - Works down to a clock speed matching the baud rate, but at low clock
     to baud rate ratios, integer ratios are preferred due to phase shift.
   - Works in queue-less loopback (aka input/output can operate at the baud rate).
   - At 8-N-1, works with <4% clock mismatch
 */
class UART(cfg: UARTConfig) extends Module {
  val io = IO(new Bundle {
    val tx = Output(Bool())
    val rx = Input(Bool())
    val tx_data = Flipped(Decoupled(UInt(cfg.N.W)))
    val rx_data = Decoupled(UInt(cfg.N.W))
  })

  val PKT_LEN = 1 + cfg.N + (if (cfg.parity == "none") 0 else 1) + cfg.stop
  val BIT_CYCLES = cfg.clock_freq / cfg.baud_rate // floor div for faster TX

  /* TX */
  val tx_dat = RegInit(1.U(PKT_LEN.W))
  val tx_ctr = RegInit((BIT_CYCLES-1).U(math.max(log2Ceil(BIT_CYCLES),1).W))

  val tx_bits_left = tx_dat(PKT_LEN-1,1).orR
  val tx_ctr_edge = tx_ctr === (BIT_CYCLES-1).U
  val tx_parity = io.tx_data.bits.xorR ^ (cfg.parity == "odd").B

  when(tx_bits_left || !tx_ctr_edge) {
    tx_ctr := Mux(tx_ctr_edge, 0.U, tx_ctr + 1.U)
  }

  when(io.tx_data.fire) {
    tx_dat := (if (cfg.parity == "none") Cat(Fill(cfg.stop, true.B), io.tx_data.bits, false.B)
               else Cat(Fill(cfg.stop, true.B), tx_parity, io.tx_data.bits, false.B))
    tx_ctr := 0.U
  }.elsewhen(tx_bits_left && tx_ctr_edge) {
    tx_dat := tx_dat >> 1;
  }

  io.tx_data.ready := !tx_bits_left && tx_ctr_edge
  io.tx := tx_dat(0)

  /* RX */

  // 2 flop synchronizer and edge detector
  val sync0 = RegNext(io.rx, true.B)
  val sync1 = RegNext(sync0, true.B)
  val rx_bit = RegNext(sync1, true.B)
  val rx_edge = rx_bit ^ sync1

  val rx_dat = RegInit(Fill(PKT_LEN, true.B))
  val rx_ctr = RegInit(0.U(math.max(log2Ceil(BIT_CYCLES),1).W))

  val rx_ctr_edge = rx_ctr === (BIT_CYCLES-1).U
  val rx_samp = rx_ctr === (BIT_CYCLES/2).U
  rx_ctr := Mux(rx_edge && rx_dat(PKT_LEN-1,1).andR, (BIT_CYCLES>1).B, Mux(rx_ctr_edge, 0.U, rx_ctr + 1.U))
  // only sync to start bit

  val rx_done = !rx_dat(0)
  val rx_parity_match = if (cfg.parity == "none") true.B
                        else rx_dat(cfg.N+1,1).xorR ^ (cfg.parity == "even").B
  val rx_stop_valid = rx_dat(PKT_LEN-1,PKT_LEN-cfg.stop).andR
  val rx_error = rx_done && !(rx_parity_match && rx_stop_valid)

  when(rx_done && rx_samp) {
    rx_dat := Cat(rx_bit, Fill(PKT_LEN-1, true.B))
  }.elsewhen(io.rx_data.fire || rx_error) {
    rx_dat := Fill(PKT_LEN, true.B)
  }.elsewhen(rx_samp) {
    rx_dat := Cat(rx_bit, rx_dat >> 1)
  }

  io.rx_data.bits := rx_dat(cfg.N,1)
  io.rx_data.valid := rx_done && rx_parity_match && rx_stop_valid // ~1 bit period to receive word
}
