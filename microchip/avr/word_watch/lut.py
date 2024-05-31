import sys
from pathlib import Path

# parameters
PINMAP = [
    ("A", 5),
    ("A", 6),
    ("A", 7),
    ("B", 5),
    ("B", 4),
    ("A", 4),
    ("A", 3),
    ("A", 2),
    ("B", 1),
    ("B", 0),
]

# generator
if __name__ == "__main__":
    N = len(PINMAP)

    with open(sys.argv[1], "w") as f:
        f.write("#pragma once\n\n")
        f.write(
            "typedef struct {\n"
            "    uint8_t porta_outset;\n"
            "    uint8_t porta_outclr;\n"
            "    uint8_t portb_outset;\n"
            "    uint8_t portb_outclr;\n"
            "    uint8_t porta_dirset;\n"
            "    uint8_t portb_dirset;\n"
            "} matrix_lut_t;\n"
            "\n"
        )
        f.write(
            f"#define MATRIX_WIDTH  ({N})\n"
            f"#define MATRIX_HEIGHT ({N - 1})\n"
            f"#define MATRIX_LENGTH (MATRIX_WIDTH * MATRIX_HEIGHT)\n"
            f"\n"
        )
        f.write(
            f"#define MATRIX_PORTA_CLR (0x{sum(1 << p[1] for p in PINMAP if p[0] == 'A'):02x})\n"
            f"#define MATRIX_PORTB_CLR (0x{sum(1 << p[1] for p in PINMAP if p[0] == 'B'):02x})\n"
            f"\n"
        )
        f.write(
            "const matrix_lut_t MATRIX_LUT[MATRIX_LENGTH] = {\n"
        )
        for y in range(N - 1):
            for x in range(N):
                # get positive pin
                if x == N - 2:
                    pos = N - 1
                else:
                    pos = y
                pos = PINMAP[pos]

                # get negative pin
                if x == N - 1:
                    neg = N - 1
                elif x == N - 2:
                    neg = y
                elif y <= x:
                    neg = x + 1
                elif y >= x + 1:
                    neg = x
                neg = PINMAP[neg]

                # convert to lut entry
                porta_outset = 1 << pos[1] if pos[0] == 'A' else 0
                porta_outclr = 1 << neg[1] if neg[0] == 'A' else 0
                portb_outset = 1 << pos[1] if pos[0] == 'B' else 0
                portb_outclr = 1 << neg[1] if neg[0] == 'B' else 0
                f.write(
                    f"    {{\n"
                    f"        .porta_outset = 0x{porta_outset:02x},\n"
                    f"        .porta_outclr = 0x{porta_outclr:02x},\n"
                    f"        .portb_outset = 0x{portb_outset:02x},\n"
                    f"        .portb_outclr = 0x{portb_outclr:02x},\n"
                    f"        .porta_dirset = 0x{porta_outset + porta_outclr:02x},\n"
                    f"        .portb_dirset = 0x{portb_outset + portb_outclr:02x},\n"
                    f"    }},\n"
                )
        f.write(
            "};\n"
        )
