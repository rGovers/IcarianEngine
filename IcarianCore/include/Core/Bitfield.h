#pragma once

#define IISBITSET(val, bit) (((val) & 0b1 << (bit)) != 0)
#define ISETBIT(val, bit) (val) |= 0b1 << (bit)
#define ICLEARBIT(val, bit) (val) &= ~(0b1 << (bit))
#define ITOGGLEBIT(state, val, bit) do { if (state) { ISETBIT(val, bit); } else { ICLEARBIT(val, bit); } } while (0)