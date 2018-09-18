#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/time.h>

#include "sdk_common.h"


static inline void cell_init(cell_t* cell)
{
	memset(cell->possible_values, 1, sizeof(cell->possible_values));
	cell->n_possible_values = LINE_LENGTH;
	cell->value = UNASSIGNED;
}

static inline uint8_t cell_choose(cell_t* cell)
{
	uint8_t value = UNASSIGNED;

	if (cell->n_possible_values == 0) {
		err("Cannot choose from no possible values\n");
		return UNASSIGNED;
	}

	while (cell->possible_values[(value = (rand() % LINE_LENGTH))] == 0);
	return value;
}

static inline void cell_forbid(cell_t* cell, uint8_t value)
{
	if (cell->possible_values[value] != 0) {
		cell->possible_values[value] = 0;
		cell->n_possible_values -= 1;
	}
}

static inline void cell_assign(cell_t* cell, uint8_t value)
{
	cell->value = value;
}

static inline void cell_fix(cell_t* cell)
{
	memset(cell->possible_values, 0, sizeof(cell->possible_values));
	cell->n_possible_values = 0;
}

static inline void cell_allow(cell_t* cell, uint8_t value)
{
	if (cell->possible_values[value])
		return;
	cell->possible_values[value] = 1;
	cell->n_possible_values += 1;
}

static inline void cell_clear(cell_t* cell)
{
	cell->value = UNASSIGNED;
}





static inline void board_init(board_t* b)
{
	for (int i = 0; i < LINE_LENGTH * LINE_LENGTH; i++)
		cell_init(&b->cells[i]);
}

static void board_assign(board_t* board, uint8_t i, uint8_t j, uint8_t value)
{
	uint8_t block_i = i - (i % 3);
	uint8_t block_j = j - (j % 3);

	cell_assign(&board->cells[i][j], value);
	for (int x = 0; x < LINE_LENGTH; x++) {
		cell_forbid(&board->cells[x][j], value);
		cell_forbid(&board->cells[i][x], value);
		cell_forbid(&board->cells[block_i + (x % 3)][block_j + (x / 3)], value);
	}
}

static int board_find_arrangement(board_t* board)
{
	uint8_t value;

	for (int i = 0; i < LINE_LENGTH; i++) {
		for (int j = 0; j < LINE_LENGTH; j++) {
			value = cell_choose(&board->cells[i][j]);
			if (value == UNASSIGNED) {
				err("Failed to select value for cell %u %u\n", i, j);
				return -1;
			}
			board_assign(board, j, i, value);
		}
	}
	return 0;
}

static inline void board_fix(board_t* board)
{
	for (int i = 0; i < LINE_LENGTH; i++) {
		for (int j = 0; j < LINE_LENGTH; j++) {
			cell_fix(&board->cells[i][j]);
		}
	}
	board->fixed = LINE_LENGTH * LINE_LENGTH;
}

static void board_print(board_t* board, char* msg)
{
	log("%s:\n", msg);
	for (int i = 0; i < LINE_LENGTH; i++) {
		for (int j = 0; j < LINE_LENGTH; j++) {
			log("%3u ", board->cells[j][i].value);
			if (j == 2 || j == 5)
				log(" ");
		}
		log("\n");
		if (i == 2 || i == 5 || i == (LINE_LENGTH - 1))
				log("\n");
	}
}

static void board_generate_full(board_t* board)
{
	srand(get_time());
	do {
		board_init(board);
	} while (board_find_arrangement(board));
	board_fix(board);
	board_print(board, "Done generating board");
}

static void board_drop_one(board_t* board)
{
	uint64_t idx;
	uint8_t i;
	uint8_t j;
	uint8_t value;
	uint8_t block_i;
	uint8_t block_j;

	do {
		idx = rand() % (LINE_LENGTH * LINE_LENGTH);
		i = idx % LINE_LENGTH;
		j = idx / LINE_LENGTH;
	} while (board->cells[i][j].value == UNASSIGNED);


	value = board->cells[i][j].value;
	cell_clear(&board->cells[i][j]);

	block_i = i - (i % 3);
	block_j = j - (j % 3);
	// Horizontal && vertival
	for (int x = 0; x < LINE_LENGTH; x++) {
		cell_allow(&board->cells[x][j], value);
		cell_allow(&board->cells[i][x], value);
		cell_allow(&board->cells[block_i + (x % 3)][block_j + (x / 3)], value);
	}

	board->fixed -= 1;
	log("Value %u cleared from %u %u\n", value, i, j);
}

static uint64_t board_count_solutions_helper(board_t* b, uint8_t i, uint8_t j)
{
	cell_t* c;
	board_t b_tmp;
	uint64_t solutions;
	uint8_t i_next = i + (j == (LINE_LENGTH - 1) ? 1 : 0);
	uint8_t j_next = (j == (LINE_LENGTH - 1) ? 0 : j + 1);

	if (i >= LINE_LENGTH) {
		board_print(b, "Possible solution");
		return 1;
	}

	c = &b->cells[i][j];
	if (c->is_fixed) {
		return board_count_solutions_helper(b, i_next, j_next);
	}

	// Not fixed. Try all possible values
	solutions = 0;
	for (int v = 0; v < LINE_LENGTH; v++) {
		if (c->possible_values[i] == 0)
			continue;
		b_tmp = *b;
		board_assign(&b_tmp, i, j, v);
		solutions += board_count_solutions_helper(&b_tmp, i_next, j_next);
	}
	return solutions;
}

static uint64_t board_count_solutions(board_t* b)
{
	board_print(b, "Attempting to solve board");
	return board_count_solutions_helper(b, 0, 0);
}

static void baord_ommit(board_t* b)
{
	board_t copy;
	do {
		copy = *b;
		board_drop_one(b);
	} while (board_count_solutions(b) == 1);

	board_print(&copy, "Removal from this board failed");
}

int main()
{
	board_t b;
	board_generate_full(&b);

	//baord_ommit(&b);
}
