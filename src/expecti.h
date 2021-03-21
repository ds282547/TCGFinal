#include "zobrist_hash.h"

#include "omp.h"

int depth = 0;

/*

Float expecti_place(bitboard b,unsigned last_op,unsigned tilebag_after_hint,unsigned hint, bool *has_dead_board);
Float expecti_slide(bitboard b, unsigned tilebag_after_hint,unsigned hint, bool *has_dead_board){



    bitboard bb_state[4];
    reward slide_reward;
    Float eval ,max_eval = 0,max_eval_dead = 0;


    bool next_has_dead_board;
    bool first_op_has_dead = true;
    bool first = true;

    for(unsigned i=0; i<4; ++i){
        //copy_board(board, state[i]);
        //slide_reward = slide[i](state[i]);

        bb_state[i] = b;
        slide_reward = bitboard_shift[i](&(bb_state[i]));
        if(slide_reward >= 0){

            next_has_dead_board = false;
            //eval = slide_reward + get_v(state[i],  tile_bag[ tile_bag_emptyslotsx ] );

            eval = slide_reward + expecti_place(bb_state[i],i,tilebag_after_hint,hint,&next_has_dead_board);

            if(next_has_dead_board) {
                
                if(first_op_has_dead || eval > max_eval_dead){
                    max_eval_dead = eval;
                    first_op_has_dead = false;
                } 
                continue;
            }
            if(first || eval > max_eval){
                max_eval = eval;
                first = false;
            } 
            
        }
    }
    // all child has dead board
    if (first){ 
        max_eval = max_eval_dead;
        *has_dead_board = true;
    } 
    return max_eval;
}
Float expecti_place(bitboard b,unsigned last_op,unsigned tilebag_after_hint,unsigned hint, bool *has_dead_board){
    depth--;
    bool found = false;
    
    Float eval = z_hash_table_get_value(b,depth,has_dead_board,&found);
    if (found){
        depth++;
        return eval;
    }
    if (depth == 0 || hint > 3) {
        eval = get_v_bb(&b, hint);
        z_hash_table_set_value(b,eval,depth,*has_dead_board);
        depth++;
        return eval;
    }
    indx emptyslots[4];
    unsigned emptyslots_length = 0;
    eval = 0;
    
    bitboard temp;
    temp = b;
    switch (last_op){
        case 0: // U
            temp >>= 48;
            for (int p = 12; p < N; ++p,temp>>=4) {
                if (!(temp & 0xf)) emptyslots[emptyslots_length++] = p;
            }
            break;
        case 1: // R
            for (int p = 0; p < N; p+=4,temp>>=16) 
                if (!(temp & 0xf)) emptyslots[emptyslots_length++] = p;
            break;
        case 2: // D
            for (int p = 0; p < 4; ++p,temp>>=4) 
                if (!(temp & 0xf)) emptyslots[emptyslots_length++] = p;
            break;
        case 3: // L
            temp >>= 12;
            for (int p = 3; p < N; p+=4,temp>>=16) 
                if (!(temp & 0xf)) emptyslots[emptyslots_length++] = p;
            break;
    };



    if (depth == 1){
        unsigned temp_tilebag = tilebag_after_hint;
        unsigned possible_hint;

        for(unsigned nhint = 1 ; nhint <= 3 ; ++nhint){
            if (temp_tilebag & 7) {
                possible_hint = nhint;
                break;
            }
            temp_tilebag >>= 3;
        }

        for(unsigned i=0;i<emptyslots_length;++i){
            // need to get the current tilebag content
            temp = b;
            bitboard_set(&temp, emptyslots[i], hint);

            bool next_has_dead_board = false;
            eval += expecti_slide(temp,tilebag_status_transfer(tilebag_after_hint, possible_hint ), possible_hint , &next_has_dead_board); 

            *has_dead_board = *has_dead_board || next_has_dead_board;

        }
        
        eval /= emptyslots_length;
    } else {

        unsigned temp_tilebag = tilebag_after_hint;
        unsigned possible_hint_length;
        unsigned possible_hint_num_total;

        unsigned possible_hints[3];
        unsigned possible_hint_nums[3];

        possible_hint_length = 0;
        possible_hint_num_total = 0;



        for(unsigned nhint = 1 ; nhint <= 3 ; ++nhint){
            unsigned possible_hint_num = temp_tilebag & 7;
            temp_tilebag >>= 3;
            if (possible_hint_num) {
                
                possible_hints[possible_hint_length] = nhint;
                possible_hint_nums[possible_hint_length] = possible_hint_num;
                possible_hint_num_total += possible_hint_num;
                possible_hint_length++;
            }
        }

        //For average
        possible_hint_num_total *= emptyslots_length;
        
        // normal tile
            
        for(unsigned i=0;i<emptyslots_length;++i){
            // need to get the current tilebag content
            temp = b;
            bitboard_set(&temp, emptyslots[i], hint);

            for(unsigned ph = 0 ; ph < possible_hint_length ; ++ph){
                ////printf("<%d,%d> ", possible_hints[ph] ,possible_hint_nums[ph]);
                // slide 1 * factor 1 + slide 2 * factor 2 + ...
                ////printf("PLACE %d with new hint %d\n", hint , possible_hints[ph]  );
                //print_bitboard_real(b);
                bool next_has_dead_board = false;
                eval += expecti_slide(temp,tilebag_status_transfer(tilebag_after_hint, possible_hints[ph] ), possible_hints[ph] , &next_has_dead_board) * possible_hint_nums[ph]; 
                
                *has_dead_board = *has_dead_board || next_has_dead_board;
                
            }

        }
        
        eval /= possible_hint_num_total;

    }


    / * if (hint <= 3) {
    } else {
        // bonus tile
        // 4 5  6  7  8  9   10  11  12 13 14
        // 6 12 24 48 96 192 384 768 
        unsigned possible_bonus_hints[8];
        unsigned possible_bonus_hint_length = biggest_tile_bb(&b) - 6;

        for(unsigned i=0;i<possible_bonus_hint_length;++i){
            possible_bonus_hints[i] = i+4;
        }

        possible_hint_num_total *= possible_bonus_hint_length;

        for(unsigned bt=0;bt<possible_bonus_hint_length;++bt){
            for(unsigned i=0;i<emptyslots_length;++i){
                // need to get the current tilebag content
                bitboard_set(&b, emptyslots[i], possible_bonus_hints[bt]);

                for(unsigned ph = 0 ; ph < possible_hint_length ; ++ph){
                    ////printf("PLACE %d with new hint %d\n", possible_bonus_hints[bt] , possible_hints[ph]  );
                    // print_bitboard_real(b);
                    // slide 1 * factor 1 + slide 2 * factor 2 + ...
                    eval += expecti_slide(b,depth-1,tilebag_status_transfer(tilebag_after_hint, possible_hints[ph] ), possible_hints[ph] ) * possible_hint_nums[ph];
                }
                bitboard_set(&b, emptyslots[i], 0);
            }
           
        }
        eval /= possible_hint_num_total;

    }
    * /

    z_hash_table_set_value(b,eval,depth,*has_dead_board);
    depth++;
    return eval;
    
}

int expecti_next(bitboard *b, reward* max_slide_reward, unsigned tilebag_after_hint,unsigned hint){
    int op = -1, op_dead = 0;
    //print_bitboard_real(*b);
    bitboard bb_state[4];
    reward slide_reward, slide_reward_dead;

    Float eval,max_eval = 0,max_eval_dead = 0;



    bool next_has_dead_board;
    bool first_op_has_dead = true;
    bool first = true;
    for(int i=0; i<4; ++i){
        //copy_board(board, state[i]);
        //slide_reward = slide[i](state[i]);

        bb_state[i] = *b;
        slide_reward = bitboard_shift[i](&(bb_state[i]));
        
        ////printf("s<%d,%d> ",i,slide_reward);
        if(slide_reward >= 0){
            //eval = slide_reward + get_v(state[i],  tile_bag[ tile_bag_emptyslotsx ] );
            // get_v_bb(&(bb_state[i]), hint)
            ////printf("First LEVEL OP %s\n",slide_name[i]);
            next_has_dead_board = false;
            depth = SEARCH_DEPTH;
            eval = slide_reward + expecti_place(bb_state[i], i, tilebag_after_hint, hint, &next_has_dead_board);
            // //printf(" NHD %d",next_has_dead_board);
            if(next_has_dead_board) {
                
                if(first_op_has_dead || eval > max_eval_dead){
                    max_eval_dead = eval;
                    slide_reward_dead = slide_reward;
                    op_dead = i;
                    first_op_has_dead = false;
                 
                }
            } else if(first || eval > max_eval){
                max_eval = eval;
                *max_slide_reward = slide_reward;
                op = i;
                first = false;
            } 
            
        }
    }

    
    // all child has dead board
    
    if(first){
        if(!first_op_has_dead){
            op = op_dead;
            *b = bb_state[op];
            *max_slide_reward = slide_reward_dead;
        }
    } else {
        *b = bb_state[op];
    }
    return op;
}
*/

Float evil_expecti_place(bitboard b,unsigned last_op,unsigned tilebag_after_hint,unsigned hint, bool *has_dead_board, unsigned bonusTile, unsigned totalTile, Float Alpha,Float Beta);


Float evil_expecti_slide(bitboard b, unsigned tilebag_after_hint,unsigned hint, bool *has_dead_board, unsigned bonusTile, unsigned totalTile, Float Alpha,Float Beta){

    depth--;
    
    bool found;

    Float eval = z_hash_table_get_value(b,depth,has_dead_board,&found,tilebag_after_hint,hint,1);

    if (found){
	depth++;
        return eval;
    }

    
    bitboard bb_state[4];
    reward slide_reward;
    Float max_eval = 0,max_eval_dead = 0;


    bool next_has_dead_board;
    bool first_op_has_dead = true;
    bool first = true;
    bool isLeaf = (depth <= 0);
    Float M = Alpha;

    
    for(unsigned i=0; i<4; ++i){
        //copy_board(board, state[i]);
        //slide_reward = slide[i](state[i]);

        bb_state[i] = b;
        slide_reward = bitboard_shift[i](&(bb_state[i]));
        if(slide_reward >= 0){

            next_has_dead_board = false;
            //eval = slide_reward + get_v(state[i],  tile_bag[ tile_bag_emptyslotsx ] );
	    eval = slide_reward + ( isLeaf ? get_v_bb(&(bb_state[i]), hint) :
			    evil_expecti_place(bb_state[i],i,tilebag_after_hint,hint,&next_has_dead_board,bonusTile,totalTile, M, Beta) );


            if(next_has_dead_board) {
                if(first_op_has_dead || eval > max_eval_dead){
                    max_eval_dead = eval;
                    first_op_has_dead = false;
                } 

                if (eval > M) M = eval;
                if (M >= Beta) {
                    //printf("Max Node Cut!\n");
                    break;
                }

                continue;
            }
            if(first || eval > max_eval){
                max_eval = eval;
                first = false;
            } 
            
            if (eval > M) M = eval;
            if (M >= Beta) {
                //printf("Max Node Cut!\n");
                break;
            }
        }
    }
    // all child has dead board
    if (first){ 
        max_eval = max_eval_dead;
        *has_dead_board = true;
    }
    z_hash_table_set_value(b,max_eval,depth,*has_dead_board,tilebag_after_hint,hint,1);
    depth++;
    return max_eval;
}
Float evil_expecti_place(bitboard b,unsigned last_op,unsigned tilebag_after_hint,unsigned hint, bool *has_dead_board, unsigned bonusTile, unsigned totalTile, Float Alpha,Float Beta){

    
    ////printf("Level 2\n");
    //depth--;

    bool found;

    Float eval = z_hash_table_get_value(b,depth,has_dead_board,&found,tilebag_after_hint,hint,0);
    if (found){
        //depth++;
        return eval;
    }


    indx emptyslots[4];
    unsigned emptyslots_length = 0;
    //eval = 0;
    
    ////printf("Level 2 Temp\n");

    bitboard temp;
    temp = b;

    //print_bitboard_real(b);
    
    switch (last_op){
        case 0: // U
            temp >>= 48;
            for (int p = 12; p < N; ++p,temp>>=4) {
                if (!(temp & 0xf)) emptyslots[emptyslots_length++] = p;
            }
            break;
        case 1: // R
            for (int p = 0; p < N; p+=4,temp>>=16) 
                if (!(temp & 0xf)) emptyslots[emptyslots_length++] = p;
            break;
        case 2: // D
            for (int p = 0; p < 4; ++p,temp>>=4) 
                if (!(temp & 0xf)) emptyslots[emptyslots_length++] = p;
            break;
        case 3: // L
            temp >>= 12;
            for (int p = 3; p < N; p+=4,temp>>=16) 
                if (!(temp & 0xf)) emptyslots[emptyslots_length++] = p;
            break;
    };
    ////printf("Level 2 Case Ok\n");
/*
    if(emptyslots_length == 0) {
        //printf("\nDEAD!\n");
    }
*/

    unsigned temp_tilebag = tilebag_after_hint;
    unsigned possible_hint;
    Float min_eval = FLT_MAX,min_eval_dead = FLT_MAX;


    bool next_has_dead_board;
    bool first_op_has_dead = true;
    bool first = true;

    next_has_dead_board = false;

    bool currentHintBouns = false;

    int largest = biggest_tile_bb(&b);
    Float bonus_tile_rate = (Float) (bonusTile) / (totalTile);

    bool gen_bonus_tile = bonus_tile_rate <= bonus_tile_rate_limit && largest >= 7;

    //bool isLeaf = (depth == 0);


    ////printf("Level 2 IF");

    Float M = Beta;

    if(hint == 4){
        // Run over All Bonus Tile
        currentHintBouns = true;

        unsigned bonus=4;
        //for(unsigned bonus=4;bonus <= largest-3;bonus++){
            for(unsigned nhint = 1 ; nhint <= 3 ; ++nhint){
                if (temp_tilebag & 7) {
                    possible_hint = nhint;
                    for(unsigned i=0;i<emptyslots_length;++i){
                        // need to get the current tilebag contentde
                        
                        temp = b;
                        bitboard_set(&temp, emptyslots[i], bonus);

                        eval = evil_expecti_slide(temp,tilebag_status_transfer(tilebag_after_hint, possible_hint ), possible_hint , &next_has_dead_board,bonusTile,totalTile,Alpha,M) ;
                                

                        if(next_has_dead_board) {
                            if(first_op_has_dead || eval < min_eval_dead) {
                                min_eval_dead = eval;
                                first_op_has_dead = false;
                            }
                        } else if(first || eval < min_eval){
                            min_eval = eval;
                            first = false;
                        }


                        if (eval < M) M = eval;
                        if (M <= Alpha) {
                            //printf("Min Node Cut\n");
                            goto GG;
                        }
                        
                    }
            
                }
                temp_tilebag >>= 3;
            }

            // bonus tile ?
            if(gen_bonus_tile) {

                for(unsigned i=0;i<emptyslots_length;++i){
                    // need to get the current tilebag content
                    
                    temp = b;
                    bitboard_set(&temp, emptyslots[i], bonus);

                    eval = evil_expecti_slide(temp,tilebag_after_hint, 4 , &next_has_dead_board,bonusTile+1,totalTile,Alpha,M); 
            

                    if(next_has_dead_board) {
                        if(first_op_has_dead || eval < min_eval_dead) {
                            min_eval_dead = eval;
                            first_op_has_dead = false;
                        }
                    } else if(first || eval < min_eval){
                        min_eval = eval;
                        first = false;
                    }


                    if (eval < M) M = eval;
                    if (M <= Alpha) {
                        //printf("Min Node Cut\n");
                        goto GG;
                    }
                }
            }
        //}
    } else {
        // Tile 1,2,3
        for(unsigned nhint = 1 ; nhint <= 3 ; ++nhint){
            if (temp_tilebag & 7) {
                possible_hint = nhint;
                for(unsigned i=0;i<emptyslots_length;++i){
                    // need to get the current tilebag content
                    temp = b;

                    bitboard_set(&temp, emptyslots[i], hint);

                    eval = evil_expecti_slide(temp,tilebag_status_transfer(tilebag_after_hint, possible_hint ), possible_hint , &next_has_dead_board,bonusTile,totalTile,Alpha,M) ;
                            
                    
                    if(next_has_dead_board) {
                        if(first_op_has_dead || eval < min_eval_dead) {
                            min_eval_dead = eval;
                            first_op_has_dead = false;
                        }
                    } else if(first || eval < min_eval){
                        min_eval = eval;
                        first = false;
                    }

                    if (eval < M) M = eval;
                    if (M <= Alpha) {
                        //printf("Min Node Cut\n");
                        goto GG;
                    }

                }
        
            }
            temp_tilebag >>= 3;
        }

        // bonus tile ?

        if(gen_bonus_tile) {

            for(unsigned i=0;i<emptyslots_length;++i){
                // need to get the current tilebag content
                
                temp = b;
                bitboard_set(&temp, emptyslots[i], hint);

                eval = evil_expecti_slide(temp,tilebag_after_hint, 4 , &next_has_dead_board,bonusTile+1,totalTile,Alpha,M);
                
                if(next_has_dead_board) {
                    if(first_op_has_dead || eval < min_eval_dead) {
                        min_eval_dead = eval;
                        first_op_has_dead = false;
                    }
                } else if(first || eval < min_eval){
                    min_eval = eval;
                    first = false;
                }

                if (eval < M) M = eval;
                if (M <= Alpha) {
                    //printf("Min Node Cut\n");
                    goto GG;
                }
            }
        }
    }

    GG:

    if (first){ 
        min_eval = min_eval_dead;
        *has_dead_board = true;
    } 

    z_hash_table_set_value(b,min_eval,depth,*has_dead_board,tilebag_after_hint,hint,0);

    //depth++;
    return min_eval;
    
}

void evil_expecti_next(bitboard *b,unsigned last_op,unsigned tilebag_after_hint,unsigned *hint, unsigned *min_pos, unsigned *new_hint, unsigned bonusTile, unsigned totalTile){
    //print_bitboard_real(*b);
    indx emptyslots[4];
    unsigned emptyslots_length = 0;
    //eval = 0;

    Float Alpha = FLT_MIN;
    Float Beta = FLT_MAX;

    
    bitboard temp;
    temp = *b;
    switch (last_op){
        case 0: // U
            temp >>= 48;
            for (int p = 12; p < N; ++p,temp>>=4) {
                if (!(temp & 0xf)) emptyslots[emptyslots_length++] = p;
            }
            break;
        case 1: // R
            for (int p = 0; p < N; p+=4,temp>>=16) 
                if (!(temp & 0xf)) emptyslots[emptyslots_length++] = p;
            break;
        case 2: // D
            for (int p = 0; p < 4; ++p,temp>>=4) 
                if (!(temp & 0xf)) emptyslots[emptyslots_length++] = p;
            break;
        case 3: // L
            temp >>= 12;
            for (int p = 3; p < N; p+=4,temp>>=16) 
                if (!(temp & 0xf)) emptyslots[emptyslots_length++] = p;
            break;
    };

/*
    if(emptyslots_length == 0) {
        //printf("\nDEAD!\n");
    }
*/
    

    unsigned temp_tilebag = tilebag_after_hint;
    unsigned possible_hint;
    Float eval,min_eval = FLT_MAX,min_eval_dead = FLT_MAX;

    int min_pos_dead, min_tile_dead,hint_dead,new_hint_dead;

    bitboard min_board = 0,min_board_dead = 0;

    bool next_has_dead_board;
    bool first_op_has_dead = true;
    bool first = true;

    next_has_dead_board = false;

    bool currentHintBouns = false;

    int largest = biggest_tile_bb(b);
    Float bonus_tile_rate = (Float) (bonusTile) / (totalTile);
    bool gen_bonus_tile = bonus_tile_rate <= bonus_tile_rate_limit && largest >= 7;

    Float M = Beta;

    if(*hint == 4){

        // Run over All Bonus Tile
        currentHintBouns = true;

        
        unsigned bonus=4;
        //for(unsigned bonus=4;bonus <= largest-3;bonus++){
            //printf("Bonus = %d   -- ",bonus);
            for(unsigned nhint = 1 ; nhint <= 3 ; ++nhint){
                if (temp_tilebag & 7) {
                    possible_hint = nhint;
                    for(unsigned i=0;i<emptyslots_length;++i){
                        // need to get the current tilebag content
                        
                        temp = *b;

                        bitboard_set(&temp, emptyslots[i], bonus);

                        depth = SEARCH_DEPTH;
                        eval = evil_expecti_slide(temp,tilebag_status_transfer(tilebag_after_hint, possible_hint ), possible_hint , &next_has_dead_board,bonusTile,totalTile+1, Alpha, M);

                        


                        if(next_has_dead_board) {
                            if(first_op_has_dead || eval < min_eval_dead) {

                                min_eval_dead = eval;
                                min_pos_dead = emptyslots[i];
                                new_hint_dead = possible_hint;
                                min_board_dead = temp;
                                hint_dead = bonus;
                                first_op_has_dead = false;
                            }
                        } else if(first || eval < min_eval){
                            min_eval = eval;
                            *min_pos = emptyslots[i];
                            *new_hint = possible_hint;
                            *hint = bonus;
                            min_board = temp;
                            first = false;
                        }

                        if (eval < M) M = eval;
                        if (M <= Alpha) {
                            //printf("Root Min Node Cut\n");
                            goto GG;
                        }
                        
                    }
            
                }
                temp_tilebag >>= 3;
            }

            // bonus tile ?
            if(gen_bonus_tile) {

                for(unsigned i=0;i<emptyslots_length;++i){
                    // need to get the current tilebag content
                    
                    temp = *b;
                    bitboard_set(&temp, emptyslots[i], bonus);

                    depth = SEARCH_DEPTH;
                    eval = evil_expecti_slide(temp,tilebag_after_hint, 4 , &next_has_dead_board,bonusTile+1,totalTile+1, Alpha, M);

                    

                    if(next_has_dead_board) {
                        if(first_op_has_dead || eval < min_eval_dead) {

                            min_eval_dead = eval;
                            min_pos_dead = emptyslots[i];
                            new_hint_dead = 4;
                            min_board_dead = temp;
                            hint_dead = bonus;
                            first_op_has_dead = false;
                        }
                    } else if(first || eval < min_eval){
                        min_eval = eval;
                        *min_pos = emptyslots[i];
                        *new_hint = 4;
                        *hint = bonus;
                        min_board = temp;
                        first = false;
                    }


                        if (eval < M) M = eval;
                        if (M <= Alpha) {
                            //printf("Root Min Node Cut\n");
                            goto GG;
                        }
                }
            }
        //}

        //printf("\n\n\n Choosen Bonus %d \n\n\n",*hint);
    } else {

        // Tile 1,2,3

        for(unsigned nhint = 1 ; nhint <= 3 ; ++nhint){
            if (temp_tilebag & 7) {
                possible_hint = nhint;
                for(unsigned i=0;i<emptyslots_length;++i){
                    // need to get the current tilebag content
                    temp = *b;

                    bitboard_set(&temp, emptyslots[i], *hint);
                    depth = SEARCH_DEPTH;
                    eval = evil_expecti_slide(temp,tilebag_status_transfer(tilebag_after_hint, possible_hint ), possible_hint , &next_has_dead_board,bonusTile,totalTile+1, Alpha, M);

                    
                    if(next_has_dead_board) {
                        if(first_op_has_dead || eval < min_eval_dead) {

                            min_eval_dead = eval;
                            min_pos_dead = emptyslots[i];
                            new_hint_dead = possible_hint;
                            min_board_dead = temp;
                            first_op_has_dead = false;
                        }
                    } else if(first || eval < min_eval){
                        min_eval = eval;
                        *min_pos = emptyslots[i];
                        *new_hint = possible_hint;
                        min_board = temp;
                        first = false;
                    }

                        if (eval < M) M = eval;
                        if (M <= Alpha) {
                            //printf("Root Min Node Cut\n");
                            goto GG;
                        }
                }
        
            }
            temp_tilebag >>= 3;
        }

        // bonus tile ?

        if(gen_bonus_tile) {

            for(unsigned i=0;i<emptyslots_length;++i){
                // need to get the current tilebag content
                
                temp = *b;
                bitboard_set(&temp, emptyslots[i], *hint);

                depth = SEARCH_DEPTH;
                eval = evil_expecti_slide(temp,tilebag_after_hint, 4 , &next_has_dead_board,bonusTile+1,totalTile ,Alpha, M);

                
                
                if(next_has_dead_board) {
                    if(first_op_has_dead || eval < min_eval_dead) {
                        min_eval_dead = eval;
                        min_pos_dead = emptyslots[i];
                        min_board_dead = temp;
                        new_hint_dead = 4;
                        first_op_has_dead = false;
                    }
                } else if(first || eval < min_eval){
                    min_eval = eval;
                    *min_pos = emptyslots[i];
                    *new_hint = 4;
                    min_board = temp;
                    first = false;
                }

                if (eval < M) M = eval;
                if (M <= Alpha) {
                    //printf("Root Min Node Cut\n");
                    goto GG;
                }
            }
        }
    }

    GG:

    if(first){
        if(!first_op_has_dead){
            
            *min_pos = min_pos_dead;
            *new_hint = new_hint_dead;
            *b = min_board_dead;
            if(currentHintBouns){
                *hint = hint_dead;
            }
        }
    } else {
        *b = min_board;
    }

}


int play_expecti_next(bitboard *b, reward* max_slide_reward, unsigned tilebag_after_hint,unsigned hint, unsigned bonusTile, unsigned totalTile){
    int op = -1, op_dead = 0;
    //print_bitboard_real(*b);
    bitboard bb_state[4];
    reward slide_reward, slide_reward_dead;

    Float eval,max_eval = 0,max_eval_dead = 0;

    Float Alpha = FLT_MIN;
    Float Beta = FLT_MAX;

    Float M = Alpha;




    bool next_has_dead_board;
    bool first_op_has_dead = true;
    bool first = true;
    for(int i=0; i<4; ++i){
        //copy_board(board, state[i]);
        //slide_reward = slide[i](state[i]);

        bb_state[i] = *b;
        slide_reward = bitboard_shift[i](&(bb_state[i]));
        
        ////printf("s<%d,%d> ",i,slide_reward);
        if(slide_reward >= 0){
            //eval = slide_reward + get_v(state[i],  tile_bag[ tile_bag_emptyslotsx ] );
            // get_v_bb(&(bb_state[i]), hint)
            ////printf("First LEVEL OP %s\n",slide_name[i]);
            next_has_dead_board = false;
            depth = SEARCH_DEPTH_P;
            eval = slide_reward + evil_expecti_place(bb_state[i], i, tilebag_after_hint, hint, &next_has_dead_board, bonusTile, totalTile, M, Beta);
            // //printf(" NHD %d",next_has_dead_board);
            if(next_has_dead_board) {
                
                if(first_op_has_dead || eval > max_eval_dead){
                    max_eval_dead = eval;
                    slide_reward_dead = slide_reward;
                    op_dead = i;
                    first_op_has_dead = false;
                 
                }
            } else if(first || eval > max_eval){
                max_eval = eval;
                *max_slide_reward = slide_reward;
                op = i;
                first = false;
            } 
            
            if (eval > M) M = eval;
            if (M >= Beta) {
                //printf("Max Node Cut!\n");
                break;
            }

        }
    }

    
    // all child has dead board
    
    if(first){
        if(!first_op_has_dead){
            op = op_dead;
            *b = bb_state[op];
            *max_slide_reward = slide_reward_dead;
        }
    } else {
        *b = bb_state[op];
    }

    return op;
}

