#include "random.h"

struct _board_status
{
	unsigned long long board;
	float value[8];
	unsigned char search_depth[8];
	unsigned short bagstatus[8];
	unsigned char has_dead_board[8];
};

typedef struct _board_status boardStatus;

unsigned long long* fast_hash_table;
boardStatus* transposition_table;
int index_mask;

void initialize_z_hash(){
	unsigned long long normal_hash_table[16][16];
	unsigned i,j;

	random_init(0);
	for(i = 0;i < 16;i++)
		for(j = 0;j < 16;j++)
			normal_hash_table[i][j] = random_value();
	fast_hash_table = (unsigned long long *) malloc( sizeof(unsigned long long) * (0x40000) ); // 4 row
	for(i = 0;i < 65536;i++) {
		for(j = 0;j < 4;j++) {
			fast_hash_table[i * 4 + j] = normal_hash_table[(i & 0xf000) >> 12][j * 4] ^
										 normal_hash_table[(i & 0x0f00) >> 8][j * 4 + 1] ^
										 normal_hash_table[(i & 0x00f0) >> 4][j * 4 + 2] ^
										 normal_hash_table[(i & 0x000f)][j * 4 + 3];
		}
	}
	transposition_table = (boardStatus*) malloc ( sizeof(boardStatus) * 0x1000000);

	if(transposition_table == NULL){
		printf("\n\n\nDead\n\n\n\n");
	}
	index_mask = 0x1000000 - 1;
}
unsigned long long get_hash_value(unsigned long long board)
{
	return fast_hash_table[((board & 0xffff000000000000) >> 46)] ^
		   fast_hash_table[((board & 0x0000ffff00000000) >> 30) + 1] ^
		   fast_hash_table[((board & 0x00000000ffff0000) >> 14) + 2] ^
		   fast_hash_table[((board & 0x000000000000ffff) << 2) + 3];
}


float z_hash_table_get_value(bitboard board,unsigned search_depth, bool *has_dead_board, bool* find_value, unsigned bag, unsigned hint, unsigned op)
{
	
	unsigned h = (hint-1) | (op<<2);
	unsigned long long hash_value = get_hash_value(board);;
	*find_value = false;
	boardStatus* board_status = transposition_table + (hash_value & index_mask);


	
	if(board_status->board == board && board_status->search_depth[h] >= search_depth && board_status->bagstatus[h] == bag) {
		
		*find_value = true;
		*has_dead_board = board_status->has_dead_board[h];

		return board_status->value[h];
	}

	
	return 0.0;
}
void z_hash_table_set_value(bitboard board,Float value,unsigned search_depth, bool has_dead_board, unsigned bag, unsigned hint, unsigned op)
{
	unsigned h = (hint-1) | (op<<2);
	unsigned long long hash_value = get_hash_value(board);
	boardStatus* board_status = transposition_table + (hash_value & index_mask);
	if(board_status->board != board) {
		board_status->board = board;
		board_status->value[h] = value;
		board_status->has_dead_board[h] = has_dead_board;
		board_status->search_depth[h] = search_depth;
		board_status->bagstatus[h] = bag;
		
	}else if(board_status->search_depth[h] < search_depth) {		
		board_status->value[h] = value;
		board_status->has_dead_board[h] = has_dead_board;
		board_status->search_depth[h] = search_depth;
		board_status->bagstatus[h] = bag;
	}
}

void free_zhashing(){
	free(transposition_table);
}