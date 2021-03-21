#include <omp.h>

#include "stdio.h"
#include "stdlib.h"
// #include "conio.h"
#include "string.h"
#include <sys/time.h>
#include "def.h"
#include "tool.h"

#include "tilebag.h"
#include "board.h"
#include "bitboard.h"
#include "action.h"
#include "tuple.h"
#include "td.h"

char * save_weight_path = NULL;
struct timeval start, end;


int main(int argc, char const *argv[])
{
    // for-loop
    unsigned i, j, k;

    // parse arguments
    char * subarg_delim = ",";
    char * subarg_token;
    char * subarg;

    char * weight_arg = NULL;
    char * load_weight_path = NULL;
    
   
    char * total_arg = NULL;

    

    unsigned total = 1000;


    for(i=1;i<argc;++i){
        if (strstr(argv[i],"--weight")) {
            weight_arg = strchr(argv[i],'=') + 1;
            if ( weight_arg[0] == '\0') {
                weight_arg = NULL;
            }
        } else if (strstr(argv[i],"--total")) {
            total_arg = strchr(argv[i],'=') + 1;
            if ( total_arg[0] != '\0') {
                total = atoi(total_arg);
            }
        } else if (strstr(argv[i],"--M")) {
            printf("Turn on Merge Feature \n");
            use_M_tuple = true;
        }
    }

    seedtime();
    
    // time measure
    
    gettimeofday(&start, NULL);




    // weight arg parse
    if (weight_arg) {
        subarg_token = strtok(weight_arg,subarg_delim);
        
        while (subarg_token != NULL)
        {
            printf ("%s\n",subarg_token);
            
            if (strstr(subarg_token,"load")) {
                load_weight_path = get_arguemt_after_equal( subarg_token );
            } else if (strstr(subarg_token,"save")) {
                save_weight_path = get_arguemt_after_equal( subarg_token );

            } else if (strstr(subarg_token,"loadsave")) {
                load_weight_path = get_arguemt_after_equal( subarg_token );
                save_weight_path = get_arguemt_after_equal( subarg_token );
            }
            subarg_token = strtok(NULL ,subarg_delim);
        }      
    }


    //load-weight

    if (load_weight_path){
        load_weight(load_weight_path);
    } else {
        init_weight();
    }
    

    // initialize all tuple 
    initialize_tuples();

    // initialize bitboard
    initialize_bitboard();

    printf("Start Training...\n");

    unsigned maxscore = 0;
    bitboard max_score_board;
   

    printf("max %p\n", &max_score_board);

    // avg score
    const int part_interval = 100;
    float avg_score[part_interval];
    unsigned avg_part = 0;
    unsigned avg_count = 0;
    avg_score[avg_part] = 0;
    unsigned scorepart = total / part_interval;

    int numthreads = omp_get_max_threads();
    printf("Use Thread %d, max thread %d\n", numthreads, omp_get_max_threads());
    unsigned total_th = total/numthreads;
    unsigned trained_game = 0;
    
    #pragma omp parallel num_threads(numthreads) default(shared) private(i,j)
    {
        printf("Thread %d start ...\n", omp_get_thread_num());
    // test bit-board
    bitboard bb;
    bitboard bb_state[4];
    bitboard bb_state_pp;
    bitboard bb_state_pp_next[4];

    // board
    /*
    indx board[16];
    indx state[4][16];
    indx state_pp[16];
    indx state_pp_next[4][16];
    */
    //indx max_score_board[16];
    // tile-bag

    indx initial_place_poses[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    indx tile_bag[TILE_BAG_SIZE];
    unsigned tile_bag_indx;
   
    // episode
    int op, op_next;
    reward slide_reward, place_reward;
    bool player_dead;
    // training
    Float eval, eval_sp;
    Float max_eval;
    Float max_V;
    Float TD_Target;
    const Float alpha = 0.0025;//0.1 / (TUPLE_COUNT * TUPLE_ISO) / 10;
    reward train_reward;

    unsigned char history_hint[10000];
    int hint_index;
    bitboard history_board[10000];
    int history_index;

    // next tile & bonus tile
    indx next_tile;

    bool gen_bonus_tile;
    unsigned bonus_tile_seed;
    unsigned bonus_tile_count;
    unsigned generated_tile_count;
    float bonus_tile_rate;



    //recorder
    unsigned score = 0;

    

    //record action to file
    /*
    FILE *action_file;
    unsigned action_episode_count = 0;
    const unsigned COND_TO_SAVE_ACTION = 12;
    action actions[20000];
    unsigned action_index;

    action_file = fopen("actions.acn","wb+");
    fwrite(&action_episode_count, sizeof(action_episode_count),1,action_file);
    fclose( action_file);
    */
   unsigned placed_position;

   
    for(j=0;j<total_th;++j) {
        
        //printf("Save File %s\n",save_weight_path);
        

        seedtime();

        bb = 0;

        // start a  new episode
        tilebag_open_episode(tile_bag, &tile_bag_indx);
        //initial_place(initial_bag, board);
        permute_array( initial_place_poses , N );

        bitboard_initial_place(initial_place_poses , tile_bag, &bb);





        tile_bag_indx = 9;
        next_tile = 0;
        bonus_tile_count = 0;
        generated_tile_count = 0;

        history_index = 0;
        hint_index = 0;
        
        //action_index = INITIAL_PLACE; // for initial placement
        eval_sp = 0;
        
        while(true){
            // player turn
            // get next best move

            op = -1;
            
            
            bonus_tile_rate = (Float) (bonus_tile_count+1) / (generated_tile_count+1);
            gen_bonus_tile = (uniform_distribution(1,21) == 1) && (bonus_tile_rate <= bonus_tile_rate_limit) && (biggest_tile_bb(&bb) >= 7);

            if (gen_bonus_tile){
                int a = 1;
            }
            bonus_tile_seed = rand(); // for which bonus tile


            for(i=0; i<4; ++i){
                bb_state[i] = bb;
                slide_reward = bitboard_shift[i](&(bb_state[i]));

                if(slide_reward >= 0){

                    eval = slide_reward + get_v_bb(&(bb_state[i]), 
                    get_next_tile (&(bb_state[i]), tile_bag, tile_bag_indx ,
                    gen_bonus_tile, bonus_tile_seed));

                    if(op == -1){
                        max_eval = eval;
                        eval_sp = eval;
                        op = i;
                    } else if( eval > max_eval ){
                        max_eval = eval;
                        eval_sp = eval;
                        op = i;
                    }
                } 
            }


            if(op == -1) {
                break;
            }
            //actions[action_index++] = op;
            //copy_board(state[op] ,board);
            // copy to history
            //copy_board(board,history_board[ history_index++ ]);

            bb = bb_state[op];
            history_board[history_index++] = bb;
            next_tile = get_next_tile(&bb, tile_bag, tile_bag_indx ,
                gen_bonus_tile, bonus_tile_seed);
            if (gen_bonus_tile)
                bonus_tile_count++;
            else
                tile_bag_indx++;
            
            generated_tile_count++;

            // place action
            //place_reward = place( board, op, tile_bag[ tile_bag_indx ], &placed_position);
            //history_hint[ hint_index++ ] = tile_bag[ tile_bag_indx ];

            place_reward = bitboard_place (&bb, op, next_tile, &placed_position);
            history_hint[ hint_index++ ] = next_tile;
            // copy to history
            //copy_board(board,history_board[ history_index++ ]);
            history_board[history_index++] = bb;
           
            //actions[action_index++] = tile_bag[ tile_bag_indx ] + (placed_position << 2);
            //learing evaluate

            /* INGAME LEARNING
            op_next = -1;

            for(i=0; i<4; ++i){
                copy_board(board, state_pp_next[i]);
                slide_reward = slide[i](state_pp_next[i]);
                if(slide_reward >= 0){
                    eval = slide_reward + get_v(state_pp_next[i], tuples, wtable);
                    if(op_next == -1){
                        max_eval = eval;
                        op_next = i;
                    } else if( eval > max_eval ){
                        max_eval = eval;
                        op_next = i;
                    }
                }
            }
            TD_Target = 0;
            if( op_next >= 0 ) TD_Target = max_eval;
            //printf("%f - %f\n",TD_Target,eval_sp);
            adjust_v( state[op], tuples, wtable, ( TD_Target -  eval_sp) * alpha  );
            */

            //tile-bag
            if(tile_bag_indx == TILE_BAG_SIZE){
                
                regen_tilebag(tile_bag, &tile_bag_indx);
            }
            
        }
       

        // max score
        
        

       


            

        score = cal_board_score_bb(&bb);
        if (score > maxscore){
            maxscore = score;
            max_score_board = bb;
        }
         #pragma omp critical
        {    
            trained_game++;
            
            avg_score[avg_part] += score;
            avg_count++;
            // avg score part
            if (trained_game > avg_part * scorepart){
                avg_score[avg_part] /= avg_count;
                avg_part++;
                avg_score[avg_part] = 0;
                avg_count = 0;
                printf("training %d/%d\n",avg_part,part_interval);

                //check_weight();
            }

            

        }

        // after-episode afterstate learning
        //    N-2    N-1     N
        //    MOVE   PLACE   X
        // discard last placement
        history_index-=2;

        hint_index--;
        
        // train terminal state
        adjust_v_bb( &(history_board[ history_index ]) , (0.0f -  
                                            get_v_bb( &(history_board[ history_index ]) , history_hint[hint_index] )
                                            ) * alpha , history_hint[hint_index]  );
        
        while( history_index ){
            // pop [Place]
            history_index--;
            hint_index--;
            //  get next best move 
            op_next = -1;
            for(i=0; i<4; ++i){
                bb_state[i] = history_board[ history_index ];
                slide_reward = bitboard_shift[i](&(bb_state[i]));
                if(slide_reward >= 0){
                    eval = slide_reward + get_v_bb( &(bb_state[i]), history_hint[hint_index+1] );
                    if(op_next == -1){
                        max_eval = eval;
                        op_next = i;
                    } else if( eval > max_eval ){
                        max_eval = eval;
                        op_next = i;
                    } 
                } 
            }
            // pop [Move]
            history_index--;
            max_eval -= get_v_bb( &(history_board[ history_index ]) , history_hint[hint_index] );
            adjust_v_bb( &(history_board[ history_index ]) , max_eval * alpha , history_hint[hint_index] );
        }
        
    }

    }

    gettimeofday(&end, NULL);


    long secs_used=end.tv_sec - start.tv_sec - 1;
    long mili_used=end.tv_usec + 1e6 - start.tv_usec;
    if ( mili_used >= 1e6 ) ++secs_used, mili_used-= 1e6;

    printf("Max Score:%u\n", maxscore);
    
    print_bitboard_real(max_score_board);
    printf("Used Time: %ld.%ld sec\n",secs_used, mili_used);
    
    


    char avg_file_name[50] = "avg_score";
    if(!use_M_tuple)
        strcat(avg_file_name,"_no_merge");
    strcat(avg_file_name,".csv");
    FILE *avg_file = fopen(avg_file_name,"w+");
    for(i=0;i<part_interval;++i){
        fprintf(avg_file, "%f\n",avg_score[i]);
    }
    fclose(avg_file);
    
    printf("Avg Score every %d episode save to %s\n", total / part_interval, avg_file_name);

    check_weight();

    //save weight
    if (save_weight_path){
        save_weight(save_weight_path);
    }
 
    return 0;
}
