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
	cell_t cells[LINE_LENGTH][LINE_LENGTH];
} board_t;

#include "am_common.h"
#include "sdk_common.h"

void cell_init(cell_t* cell);
uint8_t cell_choose(cell_t* cell);
void cell_forbid(cell_t* cell, uint8_t value);
void cell_assign(cell_t* cell, uint8_t value);
void cell_fix(cell_t* cell);
void cell_allow(cell_t* cell, uint8_t value);
void cell_clear(cell_t* cell);

void board_print(board_t* board, char* msg);
void board_init(board_t* b);
void board_assign(board_t* board, uint8_t i, uint8_t j, uint8_t value);
void board_fix(board_t* board);
void board_clear(board_t* board, uint8_t i, uint8_t j);

#endif /* __SDK_COMMON__ */
