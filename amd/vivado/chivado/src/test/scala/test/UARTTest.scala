package test

import chisel3._
import chisel3.util._
import chiseltest._
import org.scalatest.freespec.AnyFreeSpec

import scala.util.{Random => rand}

class UARTTest extends AnyFreeSpec with ChiselScalatestTester {
  val NUM_BYTES = 4
  val MAX_RATIO = 8
  val MAX_DELAY = 16

  def genEvenByte: Int = {
    val num = rand.nextInt() & 0x7F
    var x = num
    x ^= x >> 4
    x ^= x >> 2
    x ^= x >> 1
    return num | ((x << 7) & 0xFF)
  }

  def genBits(bites: Seq[Int], stops: Seq[Int], ratio: Int): Seq[Int] = { // 8-N-1
    (bites zip stops).map { case (b, s) =>
      (Seq(0) ++ Seq.tabulate(8)(i => (b >> i) & 0x1) ++ Seq(1)).flatMap(i => Seq.fill(ratio)(i)) ++ Seq.fill(s)(1)
    }.flatten
  }

  "Output at maximum speed" in {
    for (ratio <- 1 to MAX_RATIO) { // various clock to baud rate ratios
      test(new UART(UARTConfig(clock_freq=ratio, baud_rate=1))) { dut =>
        val nums = Seq.tabulate(NUM_BYTES)(_ => rand.nextInt() & 0xFF)
        val expect_bits = genBits(nums, Seq.fill(NUM_BYTES)(0), ratio)

        dut.io.tx_data.initSource()
        dut.io.tx_data.setSourceClock(dut.clock)

        fork {
          dut.io.tx_data.enqueueSeq(nums.map(i => i.U))
        }.fork {
          val actual_bits = Seq.tabulate(expect_bits.length) { _ =>
            dut.clock.step()
            dut.io.tx.peek().litValue.toInt
          }
          assert(expect_bits == actual_bits)
        }.join()
      }
    }
  }

  "Output with some time in between" in {
    for (ratio <- 1 to MAX_RATIO) {
      test(new UART(UARTConfig(clock_freq=ratio, baud_rate=1))) { dut =>
        val nums = Seq.tabulate(NUM_BYTES)(_ => rand.nextInt() & 0xFF)
        val stops = Seq.tabulate(NUM_BYTES)(_ => rand.nextInt() % MAX_DELAY + 1)
        val expect_bits = genBits(nums, stops, ratio)

        dut.io.tx_data.initSource()
        dut.io.tx_data.setSourceClock(dut.clock)

        fork {
          (nums zip stops).map { case (n, s) =>
            dut.io.tx_data.enqueue(n.U)
            dut.clock.step(10 * ratio + s - 1) // 8-N-1 = 10bits/byte
          }
        }.fork {
          val actual_bits = Seq.tabulate(expect_bits.length) { _ =>
            dut.clock.step()
            dut.io.tx.peek().litValue.toInt
          }
          assert(expect_bits == actual_bits)
        }.join()
      }
    }
  }

  "Receive bytes at maximum speed" in {
    for (ratio <- 1 to MAX_RATIO) {
      test(new UART(UARTConfig(clock_freq=ratio, baud_rate=1))) { dut =>
        val nums = Seq.tabulate(NUM_BYTES)(_ => rand.nextInt() & 0xFF)
        val rx_bits = genBits(nums, Seq.fill(NUM_BYTES)(0), ratio)

        dut.io.rx_data.initSink()
        dut.io.rx_data.setSinkClock(dut.clock)

        fork {
          rx_bits.map { b =>
            dut.io.rx.poke(b.B)
            dut.clock.step()
          }
          dut.clock.step(MAX_RATIO) // leave a few at end for input delay
        }.fork {
          dut.io.rx_data.expectDequeueSeq(nums.map(i => i.U))
        }.join()
      }
    }
  }

  "Receive bytes with some time in between" in {
    for (ratio <- 1 to MAX_RATIO) {
      test(new UART(UARTConfig(clock_freq=ratio, baud_rate=1))) { dut =>
        val nums = Seq.tabulate(NUM_BYTES)(_ => rand.nextInt() & 0xFF)
        val stops = Seq.tabulate(NUM_BYTES)(_ => rand.nextInt() % MAX_DELAY + 1)
        val rx_bits = genBits(nums, stops, ratio)

        dut.io.rx_data.initSink()
        dut.io.rx_data.setSinkClock(dut.clock)

        fork {
          rx_bits.map { b =>
            dut.io.rx.poke(b.B)
            dut.clock.step()
          }
          dut.clock.step(MAX_RATIO) // leave a few at end for input delay
        }.fork {
          dut.io.rx_data.expectDequeueSeq(nums.map(i => i.U))
        }.join()
      }
    }
  }

  "Received byte should be available ~1 bit period after arrival" in {
    test(new UART(UARTConfig(clock_freq=MAX_RATIO, baud_rate=1))) { dut =>
      val nums = Seq(rand.nextInt() & 0xFF)
      val rx_bits = genBits(nums, Seq(MAX_RATIO), MAX_RATIO)

      dut.io.rx_data.initSink()
      dut.io.rx_data.setSinkClock(dut.clock)

      fork {
        rx_bits.map { b =>
          dut.io.rx.poke(b.B)
          dut.clock.step()
        }
        dut.clock.step(MAX_RATIO) // leave a few at end for input delay
      }.fork {
        while(dut.io.rx_data.valid.peek().litToBoolean == false) {
          dut.clock.step()
        }
        dut.clock.step(MAX_RATIO - 1)
        dut.io.rx_data.expectDequeueNow(nums(0).U)
      }.join()
    }
  }

  "Receives input with <4% clock mismatch" in {
    test(new UART(UARTConfig(clock_freq=25, baud_rate=1))) { dut =>
      val nums = Seq.tabulate(NUM_BYTES)(_ => rand.nextInt() & 0xFF)
      val rx_bits = genBits(nums, Seq.fill(NUM_BYTES)(0), 24) ++
                    genBits(nums, Seq.fill(NUM_BYTES)(0), 26)

      dut.io.rx_data.initSink()
      dut.io.rx_data.setSinkClock(dut.clock)

      fork {
        rx_bits.map { b =>
          dut.io.rx.poke(b.B)
          dut.clock.step()
        }
        dut.clock.step(32) // leave a few at end for input delay
      }.fork {
        dut.io.rx_data.expectDequeueSeq(nums.map(i => i.U))
        dut.io.rx_data.expectDequeueSeq(nums.map(i => i.U))
      }.join()
    }
  }

  "Ditches bytes with bad parity and takes good ones" in {
    test(new UART(UARTConfig(clock_freq=1, baud_rate=1, N=7, parity="even"))) { dut =>
      val nums = Seq.tabulate(NUM_BYTES)(_ => genEvenByte)
      val mixed_nums = nums.flatMap(i => Seq(i, i ^ 0x1))
      val stops = Seq.tabulate(2*NUM_BYTES)(_ => rand.nextInt() % MAX_DELAY)
      val rx_bits = genBits(mixed_nums, stops, 1)

      dut.io.rx_data.initSink()
      dut.io.rx_data.setSinkClock(dut.clock)

      fork {
        rx_bits.map { b =>
          dut.io.rx.poke(b.B)
          dut.clock.step()
        }
        dut.clock.step(MAX_RATIO) // leave a few at end for input delay
      }.fork {
        dut.io.rx_data.expectDequeueSeq(nums.map(i => (i & 0x7F).U))
      }.join()
    }
  }

  "Test loopback with 2 UARTs" in {
    for (ratio <- 1 to MAX_RATIO) {
      test(new UARTLoopbackHarness(ratio)) { dut =>
        val nums = Seq.tabulate(NUM_BYTES)(_ => rand.nextInt() & 0xFF)
        val stops = Seq.tabulate(NUM_BYTES)(_ => rand.nextInt() % MAX_DELAY + 1)

        dut.io.in.initSource()
        dut.io.in.setSourceClock(dut.clock)
        dut.io.out.initSink()
        dut.io.out.setSinkClock(dut.clock)

        fork {
          (nums zip stops).map { case (n, s) =>
            dut.io.in.enqueue(n.U)
            dut.clock.step(10 * ratio + s - 1) // 8-N-1 = 10bits/byte
          }
        }.fork {
          dut.io.out.expectDequeueSeq(nums.map(i => i.U))
        }.join()
      }
    }
  }
}

class UARTLoopbackHarness(ratio: Int) extends Module {
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(UInt(8.W)))
    val out = Decoupled(UInt(8.W))
  })

  val uart0 = Module(new UART(UARTConfig(clock_freq=ratio, baud_rate=1)))
  val uart1 = Module(new UART(UARTConfig(clock_freq=ratio, baud_rate=1)))

  uart0.io.tx_data <> io.in
  uart1.io.rx := uart0.io.tx
  uart1.io.tx_data <> uart1.io.rx_data
  uart0.io.rx := uart1.io.tx
  io.out <> uart0.io.rx_data
}
