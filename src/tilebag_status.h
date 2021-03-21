// For tilebag status

unsigned tilebag_transfer_table[3][293];

unsigned possible_hint_length_table[293];
unsigned possible_hint_num_total_table[293];
unsigned possible_hints_table[293][3];
unsigned possible_hint_nums_table[293][3];



// next tilebag status = tilebag_transfer_table [ hint ][ current status ]
// tilebag status 3bit * 3 = 9bit
// the number of 1,2,3 in tilebag
// eg.
//  3   2   1
// 000 000 000

const unsigned TILEBAG_STATUS_FULL = 292;
const unsigned TILEBAG_STATUS_EMPTY = 0;

void make_possible_hint_tables(unsigned tilebag){

    unsigned possible_hint_length = 0;
    unsigned possible_hint_num_total = 0;

    unsigned temp_tilebag = tilebag;

    for(unsigned nhint = 1 ; nhint <= 3 ; ++nhint){
        unsigned possible_hint_num = temp_tilebag & 7;
        temp_tilebag >>= 3;
        if (possible_hint_num) {
            
            possible_hints_table[tilebag][possible_hint_length] = nhint;
            possible_hint_nums_table[tilebag][possible_hint_length] = possible_hint_num;
            possible_hint_num_total += possible_hint_num;
            possible_hint_length++;
        }
    }
    possible_hint_length_table[ tilebag] = possible_hint_length;
    possible_hint_num_total_table[ tilebag] = possible_hint_num_total;

}
void initialize_tilebag_transfer_table() {
    unsigned t1, t2, t3;
    unsigned curr;

    for (t1 = 0; t1 <= 4; ++t1) {
        for (t2 = 0; t2 <= 4; ++t2) {
            for (t3 = 0; t3 <= 4; ++t3) {
                curr = t1 | (t2 << 3) | (t3 << 6);
                
                if (t1 > 0) {
                    tilebag_transfer_table[0][curr] =
                        (t1 - 1) | (t2 << 3) | (t3 << 6);
                    if (tilebag_transfer_table[0][curr] ==
                        TILEBAG_STATUS_EMPTY) {
                        tilebag_transfer_table[0][curr] = TILEBAG_STATUS_FULL;
                    }

                }
                if (t2 > 0) {
                    tilebag_transfer_table[1][curr] =
                        t1 | ((t2 - 1) << 3) | (t3 << 6);
                    if (tilebag_transfer_table[1][curr] ==
                        TILEBAG_STATUS_EMPTY) {
                        tilebag_transfer_table[1][curr] = TILEBAG_STATUS_FULL;
                    }
                }
                if (t3 > 0) {
                    tilebag_transfer_table[2][curr] =
                        t1 | (t2 << 3) | ((t3 - 1) << 6);
                    if (tilebag_transfer_table[2][curr] ==
                        TILEBAG_STATUS_EMPTY) {
                        
                        tilebag_transfer_table[2][curr] = TILEBAG_STATUS_FULL;
                    }
                }
            }
        }
    }
}

void tilebag_status_show(unsigned ts) {
    printf("remaining 1[%u] 2[%u] 3[%u]\n", ts & 7, (ts >> 3) & 7,
           (ts >> 6) & 7);
}

unsigned tilebag_status_get(unsigned t1, unsigned t2, unsigned t3) {
    return t1 | (t2 << 3) | (t3 << 6);
}

unsigned tilebag_status_transfer(unsigned cur, unsigned next) {
    // if it's bonus then make it same
    return next > 3 ? cur : tilebag_transfer_table[next - 1][cur];
}
void tilebag_status_transfer_ptr(unsigned *cur, unsigned next) {
    if(next > 3) return;
    *cur = tilebag_transfer_table[next - 1][*cur];
}
unsigned tilebag_status_from_tilebag_content(indx tile_bag[], unsigned *index) {
    unsigned counter[3] = {0, 0, 0};
    for (unsigned i = *index; i < TILE_BAG_SIZE; ++i) {
        counter[tile_bag[i] - 1]++;
    }
    return tilebag_status_get(counter[0], counter[1], counter[2]);
}

typedef struct _TBS {
    unsigned t1;
    unsigned t2;
    unsigned t3;
} TBS;

TBS TBS_from_tilebag_content(indx tile_bag[], unsigned *index) {
    TBS tbs;
    unsigned counter[3] = {0,0,0};
    for (unsigned i = *index; i < TILE_BAG_SIZE; ++i) {
        counter[tile_bag[i] - 1]++;

    }
    tbs.t1 = counter[0];
    tbs.t2 = counter[1];
    
    tbs.t3 = counter[2];
    return tbs;
}
void TBS_show(TBS tbs) {
    printf("remaining 1[%u] 2[%u] 3[%u]\n", tbs.t1,tbs.t2,tbs.t3);
}
TBS TBS_transfer(TBS cur, unsigned next) {
    if( *(&(cur.t1)+(next-1)) > 0 ){
        *(&(cur.t1)+(next-1)) = *(&(cur.t1)+(next-1)) -1 ;
        if(cur.t1+cur.t2+cur.t3==0){
            cur.t1 = cur.t2 = cur.t3 = 4;
            return cur;
        }
    }
    return cur;
}