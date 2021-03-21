const unsigned indx_mapping[15] = {0, 1, 2, 3, 6, 12, 24, 48, 96, 192, 384, 768, 1536, 3072, 6144};
const unsigned score_deltas[15] = {0,0,0,3,9,27,81,243,729,2187,6561,19683,59049,177147,531441};

void reset_board(int board[]){
    memset(board, 0, 64);
}

void print_board(indx arr[]){
    printf("+--------------------+\n");
    for(unsigned i=0;i<4;++i) {
        printf("|");
        for(unsigned j=0;j<4;++j)
            printf("%5d", arr[ cell(i,j) ]);
        printf("|\n");
    }
    printf("+--------------------+\n");
}
void print_board_real(indx arr[]){
    printf("+--------------------+\n");
    for(unsigned i=0;i<4;++i) {
        printf("|");
        for(unsigned j=0;j<4;++j)
            printf("%5d", indx_mapping[ arr[ cell(i,j) ] ]);
        printf("|\n");
    }
    printf("+--------------------+\n");
}
void copy_board(indx bsource[N], indx bdest[N]){
    memcpy(bdest, bsource, 64);
}
unsigned cal_board_score(indx board[N]){
    unsigned i, score = 0;
    for(i=0;i<N;++i)
        score += score_deltas[ board[i] ];
    return score;
}

unsigned cal_board_score_bb(bitboard *b){
    unsigned i, score = 0;
    bitboard temp = *b;
    for(i=0;i<N;++i) {
        score += score_deltas[ temp & 0xf ];
        temp >>= 4;
    }
    return score;
}

unsigned biggest_tile(indx board[N]){
    unsigned i, big_tile = 0;
    for(i=0;i<N;++i)
        big_tile = board[i] > big_tile ? board[i] : big_tile;
    return big_tile;
}

unsigned get_real_tile(indx index){
    return indx_mapping[index];
}
unsigned convert_real_to_indx(unsigned realnum){
    if (realnum < 4){
        return realnum;
    } else {
        unsigned lg2=0;
        realnum/=3;
        while(realnum>>=1) ++lg2; // eg 4 = 100  shift 3 times
        return lg2+3;
    }
}

unsigned long long convert_board_to_bit(indx board[]){
	unsigned long long r = 0;
	unsigned i;
	for(i=0;i<16;++i){
		r = r || ((unsigned long long)board[i]) << (i << 2);
	}
	return r;
}

int get_all_possible_board(indx board[],indx possible_boards[45][16], float boards_possibility[],unsigned slide_op, unsigned has_hint){
    indx ind[4];
    bool slots_count = 0;
    unsigned pb_count = 0;
    unsigned p,i,j;

    switch(slide_op){
        case 0:
        // U
            for (p = 12; p < N; ++p) 
                if (board[p] == 0) ind[ slots_count++ ] = p;
            break;
        case 1:
        // R
            for (p = 0; p < N; p+=4) 
                if (board[p] == 0) ind[ slots_count++ ] = p;
            break;
        case 2:
        // D
            for (p = 0; p < 4; ++p) 
                if (board[p] == 0) ind[ slots_count++ ] = p;
            break;
        case 3:
        // L
            for (p = 3; p < N; p+=4) 
                if (board[p] == 0) ind[ slots_count++ ] = p;
        break;
    }

    if (has_hint){
        for(i=0;i<slots_count;i++){
            copy_board(board, possible_boards[pb_count]);
            possible_boards[pb_count][ ind[i] ] = has_hint;
            boards_possibility[pb_count] = 1.0f / slots_count;
            pb_count++;
        }
    } else {
        for(i=0;i<slots_count;i++){
            for(j=1;j<=3;j++){
                copy_board(board, possible_boards[pb_count]);
                possible_boards[pb_count][ ind[i] ] = j;
                boards_possibility[pb_count] = 1.0f / 3.0f / slots_count;
                pb_count++;
            }
        }
    }
    return pb_count;
}