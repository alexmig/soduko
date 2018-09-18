#include "sdk_common.h"


static int game_generate(board_t* b, uint8_t i, uint8_t j)
{
	cell_t* c;
	board_t b_tmp;
	uint8_t i_next = i + (j == (LINE_LENGTH - 1) ? 1 : 0);
	uint8_t j_next = (j == (LINE_LENGTH - 1) ? 0 : j + 1);
	
	if (i >= LINE_LENGTH) {
		return 1;
	}

	c = &b->cells[i][j];
	
	// TODO: Randomize
	for (uint8_t v = 0; v < LINE_LENGTH; v++) {
		if (!c->possible_values[v])
			continue;
		b_tmp = *b;
		board_assign(&b_tmp, i, j, v);
		if (game_generate(&b_tmp, i_next, j_next)) {
			*b = b_tmp;
			return 1;
		}
	}
	return 0;
}

int main()
{
	board_t b;
	board_init(&b);
	if (game_generate(&b, 0, 0)) {
		board_print(&b, "Generated board");
		file_write("Generated_board.full", &b, sizeof(b));
	}
}
