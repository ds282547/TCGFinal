

void print_bitboard(bitboard b);
void initialize_bitboard(){
    // mask table
    bitboard m = 15;
    unsigned i,j;
    for(i=0;i<16;++i){
        bitboard_mask[i] = ~m;
        m <<= 4;
    }
    // shift left table
    unsigned cur,adj,temp,ans,score_delta;
    const unsigned row_mask[4] = {0xfff0,0xff0f,0xf0ff,0x0fff};
    const unsigned row_shift[4] = {0,4,8,12};
    for(i=0;i<0xffff;++i){
        ans = i;
        score_delta = 0;
        for(j=0;j<3;++j){
            temp = ans >> row_shift[j];
            cur = temp&0xf;
            adj = (temp&0xf0) >> 4;
            if((cur == 1 && adj == 2) || (cur == 2 && adj == 1)){ // merge 1 & 2 to 3
                score_delta += 3;
                ans = ans & row_mask[j] | (3 << row_shift[j]); 
                ans = ans & row_mask[j+1];
            } else if ( cur > 2 && cur < 14 && cur == adj) { // merge 3*2^i to 3*2^(i+1)
                score_delta += score_deltas[ cur ];
                ans = ans + (1 << row_shift[j]);
                ans = ans & row_mask[j+1];
            } else if ( cur == 0 && adj > 0) { // move to hole
                ans = (ans & row_mask[j]) | ((ans >> 4) & ~row_mask[j]);
                ans = ans & row_mask[j+1];
            }
        }
        bitboard_shift_score_table[i] = score_delta;
        bitboard_shift_table[i] = ans;
    }

}
void print_bitboard(bitboard b){
    printf("+--------------------+\n");
    for(unsigned i=0;i<4;++i) {
        printf("|");
        for(unsigned j=0;j<4;++j){
            printf("%5d", b & 0xf );
            b >>= 4;
        }
        printf("|\n");
    }
    printf("+--------------------+\n");
}

void print_bitboard_real(bitboard b){
    printf("+--------------------+\n");
    for(unsigned i=0;i<4;++i) {
        printf("|");
        for(unsigned j=0;j<4;++j){
            printf("%5d", indx_mapping[b & 0xf] );
            b >>= 4;
        }
        printf("|\n");
    }
    printf("+--------------------+\n");
}

void bitboard_set(bitboard* b,unsigned pos,bitboard tile){
    *b = (*b & bitboard_mask[pos]) | (tile << (pos << 2));
}
unsigned bitboard_get(bitboard* b,unsigned pos){
    return ((*b) >> (pos << 2)) & 0xf;
}
void bitboard_rotate_ccw(bitboard* b){
    bitboard res = 0;
    res |=  (*b & 0x000F000F000F000F)* 0x0001001001001000 & 0xFFFF000000000000;
    res |= ((*b & 0x00F000F000F000F0)* 0x0000100100100100 & 0xFFFF000000000000) >> 16;
    res |= ((*b & 0x0F000F000F000F00)* 0x0000010010010010 & 0xFFFF000000000000) >> 32;
    res |= ((*b & 0xF000F000F000F000)* 0x0000001001001001 & 0xFFFF000000000000) >> 48;
    *b = res;
}
void bitboard_rotate_cw(bitboard* b){
    bitboard res = 0;
    res |= ((*b & 0x000F000F000F000F)* 0x1000010000100001 & 0xFFFF000000000000) >> 48;
    res |= (((*b>>=4) & 0x000F000F000F000F)* 0x1000010000100001 & 0xFFFF000000000000) >> 32;
    res |= (((*b>>=4) & 0x000F000F000F000F)* 0x1000010000100001 & 0xFFFF000000000000) >> 16;
    res |= (((*b>>=4) & 0x000F000F000F000F)* 0x1000010000100001 & 0xFFFF000000000000);
    *b = res;
}
void bitboard_flip(bitboard* b){
    bitboard res = 0;
    res |= (*b & 0x000F000F000F000F) << 12;
    res |= (*b & 0x00F000F000F000F0) << 4;
    res |= (*b & 0x0F000F000F000F00) >> 4;
    res |= (*b & 0xF000F000F000F000) >> 12;
    *b = res;
}
reward bitboard_shift_left(bitboard *b){
    bitboard old = *b;
    unsigned score;
    score = bitboard_shift_score_table [ *b & 0x000000000000FFFF] +
            bitboard_shift_score_table [(*b & 0x00000000FFFF0000)>>16] +
            bitboard_shift_score_table [(*b & 0x0000FFFF00000000)>>32] +
            bitboard_shift_score_table [(*b & 0xFFFF000000000000)>>48];
    *b = bitboard_shift_table [ *b & 0x000000000000FFFF]          | (*b & (0xFFFFFFFFFFFF0000));
    *b = (bitboard)bitboard_shift_table [(*b & 0x00000000FFFF0000)>>16]<<16 | (*b & (0xFFFFFFFF0000FFFF));
    *b = (bitboard)bitboard_shift_table [(*b & 0x0000FFFF00000000)>>32]<<32 | (*b & (0xFFFF0000FFFFFFFF));
    *b = (bitboard)bitboard_shift_table [(*b & 0xFFFF000000000000)>>48]<<48 | (*b & (0x0000FFFFFFFFFFFF));
    //printf("Row1 %u = %u\n",*b & 0x000000000000FFF, bitboard_shift_score_table [ *b & 0x000000000000FFFF]);
    if (old == *b)
        return -1;
    else
        return score;
}
reward bitboard_shift_right(bitboard *b){
    reward score;
    bitboard temp = *b;
    bitboard_flip(&temp);
    score = bitboard_shift_left(&temp);
    bitboard_flip(&temp);
    *b = temp;
    return score;
}
reward bitboard_shift_up(bitboard *b){
    reward score;
    bitboard temp = *b;
    bitboard_rotate_ccw(&temp);
    score = bitboard_shift_left(&temp);
    bitboard_rotate_cw(&temp);
    *b = temp;
    return score;
}
reward bitboard_shift_down(bitboard *b){
    reward score;
    bitboard temp = *b;
    bitboard_rotate_cw(&temp);
    score = bitboard_shift_left(&temp);
    bitboard_rotate_ccw(&temp);
    *b = temp;
    return score;
}
bitboard bitboard_from_board(indx board[]){
    unsigned i;
    bitboard r = 0;
    for(i=0;i<16;++i){
        r = r | (bitboard)board[i] << (i<<2);
    }
    return r;
}
reward (*bitboard_shift[4])(bitboard *b) = {bitboard_shift_up,
 bitboard_shift_right, bitboard_shift_down, bitboard_shift_left};

 void bitboard_initial_place(indx poses[],indx tile_bag[], bitboard* b){
    for(unsigned i=0; i<INITIAL_PLACE; ++i){
        bitboard_set(b, poses[i], tile_bag[i]);
        //actions[i] = ((i%3)+1) + ( initial_bag[i] << 2);
    }
 }

reward bitboard_place(bitboard * b, int slide_op, indx tile, unsigned *placed_pos){
    indx ind[4];
    unsigned n = 0 , pos;
    switch (slide_op){
        case 0: // U
            for (int p = 12; p < N; ++p) 
                if (bitboard_get(b,p) == 0) ind[n++] = p;
            break;
        case 1: // R
            for (int p = 0; p < N; p+=4) 
                if (bitboard_get(b,p) == 0) ind[n++] = p;
            break;
        case 2: // D
            for (int p = 0; p < 4; ++p) 
                if (bitboard_get(b,p) == 0) ind[n++] = p;
            break;
        case 3: // L

            for (int p = 3; p < N; p+=4) 
                if (bitboard_get(b,p) == 0) ind[n++] = p;
            break;
    };
    
    if(n == 0)
        return -1;

    pos =  ind [ uniform_distribution(0,n-1) ];
    bitboard_set(b, pos, tile);
    *placed_pos = pos;
    return ( tile == 3 ) * 3;
}
unsigned biggest_tile_bb(bitboard *b){
    
    unsigned i, big_tile = 0;
    bitboard temp = *b;
    for(i=0;i<N;++i) {
        big_tile = (temp&0xf) > big_tile ? (temp&0xf) : big_tile;
        temp >>= 4;
    }
    return big_tile;
}

