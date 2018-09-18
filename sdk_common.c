#include "am_common.h"
#include "sdk_common.h"

void cell_init(cell_t* cell)
{
	memset(cell->possible_values, 1, sizeof(cell->possible_values));
	cell->n_possible_values = LINE_LENGTH;
	cell->value = UNASSIGNED;
}

uint8_t cell_choose(cell_t* cell)
{
	uint8_t value;
	while (cell->n_possible_values != 0) {
		value = rand() % LINE_LENGTH;
		if (cell->possible_values[value])
			return value;
	}
	return UNASSIGNED;
}

void cell_forbid(cell_t* cell, uint8_t value)
{
	if (cell->possible_values[value] != 0) {
		cell->possible_values[value] = 0;
		cell->n_possible_values -= 1;
	}
}

void cell_assign(cell_t* cell, uint8_t value)
{
	cell->value = value;
}

void cell_fix(cell_t* cell)
{
	memset(cell->possible_values, 0, sizeof(cell->possible_values));
	cell->n_possible_values = 0;
}

void cell_allow(cell_t* cell, uint8_t value)
{
	if (cell->possible_values[value])
		return;
	cell->possible_values[value] = 1;
	cell->n_possible_values += 1;
}

void cell_clear(cell_t* cell)
{
	cell->value = UNASSIGNED;
}




void board_print(board_t* board, char* msg)
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

void board_init(board_t* b)
{
	for (int i = 0; i < LINE_LENGTH; i++)
		for (int j = 0; j < LINE_LENGTH; j++)
			cell_init(&b->cells[i][j]);
}

void board_assign(board_t* board, uint8_t i, uint8_t j, uint8_t value)
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

void board_fix(board_t* board)
{
	for (int i = 0; i < LINE_LENGTH; i++)
		for (int j = 0; j < LINE_LENGTH; j++)
			cell_fix(&board->cells[i][j]);
}

void board_clear(board_t* board, uint8_t i, uint8_t j)
{
	uint8_t value;
	uint8_t block_i = i - (i % 3);
	uint8_t block_j = j - (j % 3);

	value = board->cells[i][j].value;
	cell_clear(&board->cells[i][j]);
	for (int x = 0; x < LINE_LENGTH; x++) {
		cell_allow(&board->cells[x][j], value);
		cell_allow(&board->cells[i][x], value);
		cell_allow(&board->cells[block_i + (x % 3)][block_j + (x / 3)], value);
	}
}
