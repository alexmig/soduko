#ifndef __SDK_COMMON__
#define __SDK_COMMON__

#include <inttypes.h>

#include "am_common.h"

enum {
	LINE_LENGTH = 9,
	UNASSIGNED = 255,
};

typedef struct {
	uint8_t possible_values[LINE_LENGTH];
	uint8_t n_possible_values;
	uint8_t value;
} cell_t;

typedef struct {
	cell_t cells[LINE_LENGTH * LINE_LENGTH];
} board_t;

#endif /* __SDK_COMMON__ */
