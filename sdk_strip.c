#include "sdk_common.h"

static void game_drop_random_digit(board_t* b)
{
	uint64_t idx;
	uint8_t i;
	uint8_t j;
	
	do {
		idx = rand() % (LINE_LENGTH * LINE_LENGTH);
		i = idx % LINE_LENGTH;
		j = idx / LINE_LENGTH;
	} while (b->cells[i][j].value == UNASSIGNED);

	board_clear(b, i, j);
}

static uint64_t game_count_solutions(board_t* b, uint8_t i, uint8_t j)
{
	cell_t* c;
	board_t b_tmp;
	uint8_t i_next = i + (j == (LINE_LENGTH - 1) ? 1 : 0);
	uint8_t j_next = (j == (LINE_LENGTH - 1) ? 0 : j + 1);
	uint64_t solutions = 0;
	
	if (i >= LINE_LENGTH) {
		board_print(b, "Possible solution");
		return 1;
	}

	c = &b->cells[i][j];
	if (c->value != UNASSIGNED)
		return game_count_solutions(b, i_next, j_next);

	for (uint8_t v = 0; v < LINE_LENGTH; v++) {
		if (!c->possible_values[v])
			continue;
		b_tmp = *b;
		board_assign(&b_tmp, i, j, v);
		solutions += game_count_solutions(&b_tmp, i_next, j_next);
	}
	return solutions;
}

static void game_strip(board_t* b)
{
	uint64_t solutions;
	board_t b_tmp = *b;
	
	do {
		*b = b_tmp;
		game_drop_random_digit(&b_tmp);
		board_print(&b_tmp, "TMP BOARD");
		solutions = game_count_solutions(&b_tmp, 0, 0);
	} while (solutions == 1);
	log("Stopped with %lu solutions to board\n", solutions);
	board_print(&b_tmp, "Abnormal number of solutions");
	board_print(b, "OK number of solutions");
}

int main()
{
	board_t* b = file_read("Generated_board.full", NULL);

	srand(get_time());
	game_strip(b);
	file_write("Generated_board.stripped", b, sizeof(*b));
	return 0;
}
