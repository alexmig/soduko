#include "sdk_common.h"

static void game_drop_random_digit(board_t* b)
{
	uint8_t i;
	uint8_t j;

	do {
		coords_rand(&i, &j);
	} while (b->cells[i][j].value == UNASSIGNED);

	board_clear(b, i, j);
}

static uint64_t game_count_solutions(board_t* b, uint8_t i, uint8_t j)
{
	cell_t* c;
	board_t b_tmp;
	uint8_t i_next = i;
	uint8_t j_next = j;
	uint64_t solutions = 0;

	coords_next_overflow(&i_next, &j_next);

	if (i >= LINE_LENGTH) {
		return 1;
	}

	c = &b->cells[i][j];
	if (c->value != UNASSIGNED)
		return game_count_solutions(b, i_next, j_next);

	for (uint8_t v = 0; v < LINE_LENGTH; v++) {
		if (!c->possible_values[v])
			continue;
		if(!board_is_possible(b, i, j, v))
			continue;
		b_tmp = *b;
		board_assign(&b_tmp, i, j, v);
		solutions += game_count_solutions(&b_tmp, i_next, j_next);
	}
	return solutions;
}

static void game_strip(board_t* b, const uint8_t n_retries)
{
	uint64_t solutions = 0;
	uint64_t retries = 0;
	board_t b_tmp;

	while (retries < n_retries) {
		b_tmp = *b;
		game_drop_random_digit(&b_tmp);
		solutions = game_count_solutions(&b_tmp, 0, 0);
		if (solutions == 1) {
			*b = b_tmp;
			retries = 0;
		}
		else {
			retries++;
		}
	}

	board_print(b, "OK number of solutions");
}

static void game_max(board_t* b)
{
	uint64_t solutions = 0;
	uint8_t i;
	uint8_t j;
	cell_t* c;
	board_t b_tmp;

	coords_rand(&i, &j);
	for (int x = 0; x < BOARD_SIZE; x++, coords_next_wrap(&i, &j)) {
		b_tmp = *b;
		c = &b_tmp.cells[i][j];
		if (c->value == UNASSIGNED)
			continue;
		board_clear(&b_tmp, i, j);
		solutions = game_count_solutions(&b_tmp, 0, 0);
		if (solutions == 1) {
			log("Found max-strip candidate at (%u, %u)\n", i, j);
			*b = b_tmp;
		}
	}

	board_print(b, "Stripped to the maximum");
}

int main()
{
	board_t* b = file_read("Generated_board.full", NULL);

	srand(get_time());
	game_strip(b, 10);
	game_max(b);
	file_write("Generated_board.stripped", b, sizeof(*b));
	return 0;
}
