void initial_place(indx initial_bag[], indx board[]){
    for(unsigned i=0; i<INITIAL_PLACE; ++i){
        board[ initial_bag[i] ] = (i%3)+1;
        //actions[i] = ((i%3)+1) + ( initial_bag[i] << 2);
    }
}

reward slide_left(indx board[]) {
    unsigned moved = false;
    unsigned pos;
    reward score_delta = 0;
    for (int r = 0; r < N; r+=4) {
        for (int c = 0; c < 3; c++) {
            pos = r+c;
            if( (board[pos] == 1 && board[pos+1] == 2) || (board[pos] == 2 && board[pos+1] == 1)){ // merge 1 & 2 to 3
                score_delta += 3;
                board[pos] = 3; board[pos+1] = 0;
                moved = true;
            } else if ( board[pos] > 2 && board[pos] < 14 && board[pos] == board[pos+1]) { // merge 3*2^i to 3*2^(i+1)
                score_delta += score_deltas[ board[pos] ];
                ++board[pos]; board[pos+1] = 0;
                moved = true;
                
            } else if ( board[pos] == 0 && board[pos+1] > 0) { // move to hole
                board[pos] = board[pos+1]; board[pos+1] = 0;
                moved = true;
            }
        }
    }
    return moved ? score_delta : -1;
}
reward slide_right(indx board[]) {
    unsigned moved = false;
    unsigned pos;
    reward score_delta = 0;
    for (int r = 0; r < N; r+=4) {
        for (int c = 3; c > 0; c--) {
            pos = r+c;
            if( (board[pos] == 1 && board[pos-1] == 2) || (board[pos] == 2 && board[pos-1] == 1)){ // merge 1 & 2 to 3
                score_delta += 3;
                board[pos] = 3; board[pos-1] = 0;
                moved = true;
            } else if ( board[pos] > 2 && board[pos] < 14 && board[pos] == board[pos-1]) { // merge 3*2^i to 3*2^(i+1)
                score_delta += score_deltas[ board[pos] ];
                ++board[pos]; board[pos-1] = 0;
                moved = true;
            } else if ( board[pos] == 0 && board[pos-1] > 0) { // move to hole
                board[pos] = board[pos-1]; board[pos-1] = 0;
                moved = true;
            }
        }
    }
    return moved ? score_delta : -1;
}
reward slide_up(indx board[]) {
    unsigned moved = false;
    unsigned pos;
    reward score_delta = 0;
    for (int r = 0; r < 12; r+=4) {
        for (int c = 0; c < 4; c++) {
            pos = r+c;
            if( (board[pos] == 1 && board[pos+4] == 2) || (board[pos] == 2 && board[pos+4] == 1)){ // merge 1 & 2 to 3
                score_delta += 3;
                board[pos] = 3; board[pos+4]  = 0;
                moved = true;
            } else if ( board[pos] > 2 && board[pos] < 14 && board[pos] == board[pos+4]) { // merge 3*2^i to 3*2^(i+1)
                score_delta += score_deltas[ board[pos]];
                ++board[pos]; board[pos+4] = 0;
                moved = true;
            } else if ( board[pos] == 0 && board[pos+4] > 0) { // move to hole
                board[pos] = board[pos+4]; board[pos+4] = 0;
                moved = true;
            }
        }
    }
    return moved ? score_delta : -1;
}
reward slide_down(indx board[]) {
    unsigned moved = false;
    unsigned pos;
    reward score_delta = 0;
    for (int r = 12; r > 0; r-=4) {
        for (int c = 0; c < 4; c++) {
            pos = r+c;
            if( (board[pos] == 1 && board[pos-4] == 2) || (board[pos] == 2 && board[pos-4] == 1)){ // merge 1 & 2 to 3
                score_delta += 3;
                board[pos] = 3; board[pos-4] = 0;
                moved = true;
            } else if ( board[pos] > 2 && board[pos] < 14 && board[pos] == board[pos-4]) { // merge 3*2^i to 3*2^(i+1)
                score_delta += score_deltas[ board[pos] ];
                ++board[pos]; board[pos-4] = 0;
                moved = true;
            } else if ( board[pos] == 0 && board[pos-4] > 0) { // move to hole
                board[pos] = board[pos-4]; board[pos-4] = 0;
                moved = true;
            }
        }
    }
    return moved ? score_delta : -1;
}
reward (*slide[4])(indx board[16]) = {slide_up, slide_right, slide_down, slide_left};
char *slide_name[4] = {"up","right","down","left"};

reward place(indx board[16], int slide_op, indx tile, unsigned *placed_pos){
    indx ind[4];
    unsigned n = 0 , pos;
    switch (slide_op){
        case 0: // U
            for (int p = 12; p < N; ++p) 
                if (board[p] == 0) ind[n++] = p;
            break;
        case 1: // R
            for (int p = 0; p < N; p+=4) 
                if (board[p] == 0) ind[n++] = p;
            break;
        case 2: // D
            for (int p = 0; p < 4; ++p) 
                if (board[p] == 0) ind[n++] = p;
            break;
        case 3: // L

            for (int p = 3; p < N; p+=4) 
                if (board[p] == 0) ind[n++] = p;
            break;
    };
    
    if(n == 0)
        return -1;

    pos =  ind [ uniform_distribution(0,n-1) ];
    board[ pos ] = tile;
    *placed_pos = pos;
    return ( tile == 3 ) * 3;
}

indx get_next_tile(bitboard *b,indx tile_bag[], unsigned tb_index,bool gen_bt,unsigned seed){
    indx largeset = biggest_tile_bb(b);
    if (gen_bt){
        // bonus tile
        return uniform_distribution_r(seed,4,largeset-3);
    } else {
        return tile_bag[tb_index];
    } 
}
