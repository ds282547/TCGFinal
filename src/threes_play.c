#include "stdio.h"
#include "stdlib.h"
// #include "conio.h"
#include "string.h"
#include "def.h"
#include <sys/time.h>
#include "time_elapsed.h"

#include "tool.h"

#include "tilebag.h"
#include "tilebag_status.h"

#include "board.h"
#include "bitboard.h";
#include "action.h"
#include "tuple.h"
#include "td.h"

#include "expecti.h"


//#include "tile_pb.h"
//#include "bestmove.h"
#include "stat.h"

char * stat_path = NULL;


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

    char * seed_arg = NULL;

    char * depth_arg = NULL;
    char * multistage_arg = NULL;
    
    bool save_stat = false;

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
        } else if (strstr(argv[i],"--save")) {
            stat_path = strchr(argv[i],'=') + 1;
            if ( stat_path[0] == '\0') {
                stat_path = NULL;
            } else {
                save_stat = true;
                printf("Stat Save to %s\n", stat_path);
            }
        } else if (strstr(argv[i],"--evil=seed")) {
            seed_arg = strchr(strchr(argv[i],'=') + 1,'=')+1;
            if ( seed_arg[0] == '\0') {
                seed_arg = NULL;
            }
        } else if (strstr(argv[i],"--depth")) {
            depth_arg = strchr(argv[i],'=') + 1;
            if ( depth_arg[0] != '\0') {
                SEARCH_DEPTH = atoi(depth_arg);
            }
        }   else if (strstr(argv[i],"--MS")) {
            multistage_arg = strchr(argv[i],'=') + 1;
            if ( multistage_arg[0] == '\0') {
                multistage_arg = NULL;
            }
        }
    }
    if (seed_arg){
        if(strcmp(seed_arg,"time")==0){
            printf("Random Seed = time\n");
            seedtime();   
        } else {
            printf("Random Seed = %s\n",seed_arg);
            srand(atoi(seed_arg));
        }
    }
    
    // multi-stage mode to get high score boards
    unsigned multistage_tile;
    unsigned multistage_used_ep = 0;
    bool multistage_mode = false;
    char multistage_output_filename[40]  = "multistage_";
    FILE * multistage_output_file;

    if (multistage_arg){
        multistage_mode = true;
        multistage_tile = atoi(multistage_arg);
        printf("===================\nGet Boards For Multi-Stage\n===================\nTarget Tile #%d[%d] \n",multistage_tile,indx_mapping[multistage_tile]);
        
        char temp[40];
        strcat(multistage_output_filename,itoa(indx_mapping[multistage_tile],temp,10));
        strcat(multistage_output_filename,"_tile_");
        strcat(multistage_output_filename,itoa(total,temp,10));
        strcat(multistage_output_filename,"_boards");
        printf("Target Boards Num = %d\n===================\n", total);
        printf("Save To %s\n",multistage_output_filename);
        multistage_output_file = fopen(multistage_output_filename,"wb+");
        //write board number
        fwrite(&total,sizeof(total),1,multistage_output_file);
        
        fclose(multistage_output_file);
    }


    // time measure
    start_tp();

    // bitboard
    bitboard bb;
    bitboard bb_state[4];
    // board
    //indx board[16];
    //indx state[4][16];

    // tile-bag
    //indx initial_bag[N];

    indx initial_place_poses[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    indx tile_bag[TILE_BAG_SIZE];
    unsigned tile_bag_indx;

    // weight arg parse
    if (weight_arg) {
        subarg_token = strtok(weight_arg,subarg_delim);
        
        while (subarg_token != NULL)
        {
            printf ("%s\n",subarg_token);
            
            if (strstr(subarg_token,"load")) {
                load_weight_path = get_arguemt_after_equal( subarg_token );
            } 
            subarg_token = strtok(NULL ,subarg_delim);
        }      
    }
    
    //load-weight

    if (load_weight_path){
        load_weight(load_weight_path);
    } else {
        printf("You don't have weight file!!!");
        return 0;
    }
    
    // initialize bitboard
    initialize_bitboard();
    // initialize all tuple 
    initialize_tuples();

    // for tilebag status
    initialize_tilebag_transfer_table();

    // for z-hash
    initialize_z_hash();

    // initialize statics
    initialize_stat(&total);
    if (save_stat){
        setup_stat_file(stat_path);
    }
    // initial playing
    float eval,max_eval;


    //tile_pb tp;
    //init_tile_pb(&tp);
   
    // episode
    int op;
    reward max_slide_reward,slide_reward, place_reward;
    bool player_dead;
    
    indx next_tile;

    bool gen_bonus_tile;
    unsigned bonus_tile_seed;
    unsigned bonus_tile_count;
    unsigned generated_tile_count;
    float bonus_tile_rate;

    //recorder
    unsigned score = 0;
    unsigned maxscore = 0;
    unsigned maxscore_index = 0;
    // avg score
    float avg_score = 0;

    unsigned placed_position;

    unsigned tilebag_status = TILEBAG_STATUS_FULL;

    unsigned ep_step = multistage_mode ? 0 : 1;
    for(j=0;j<total;j+=ep_step,multistage_used_ep+=multistage_mode) {
        //printf("\n==== EP %d ==== \n\n",j);
        // reset board and seedtime
        //reset_board(board);
        bb = 0;
        // seedtime();
        // open stat
        stat_open_episode();
        // start a  new episode
        tilebag_open_episode(tile_bag, &tile_bag_indx);
        //initial_place(initial_bag, board);
        permute_array( initial_place_poses , N );

        bitboard_initial_place(initial_place_poses , tile_bag, &bb);

        stat_action_initial_place(initial_place_poses , tile_bag);


        tile_bag_indx = 9;
        next_tile = 0;
        bonus_tile_count = 0;
        generated_tile_count = 0;

        tilebag_status = tilebag_status_from_tilebag_content(tile_bag, &tile_bag_indx);

        //action_index = INITIAL_PLACE; // for initial placement
        while(true){
            // player turn
            // get next best move
            
            op = -1;

            bonus_tile_rate = (Float) (bonus_tile_count+1) / (generated_tile_count+1);
            gen_bonus_tile = (uniform_distribution(1,21) == 1 && bonus_tile_rate <= bonus_tile_rate_limit && biggest_tile_bb(&bb) >= 7);

            bonus_tile_seed = rand(); // for which bonus tile

            /*

            for(i=0; i<4; ++i){
                

                //copy_board(board, state[i]);
                //slide_reward = slide[i](state[i]);

                bb_state[i] = bb;
                slide_reward = bitboard_shift[i](&(bb_state[i]));
                
                if(slide_reward >= 0){
                    //eval = slide_reward + get_v(state[i],  tile_bag[ tile_bag_indx ] );

                    eval = slide_reward + get_v_bb(&(bb_state[i]), 
                    get_next_tile (&(bb_state[i]), tile_bag, tile_bag_indx ,
                    gen_bonus_tile, bonus_tile_seed));


                    if(op == -1){
                        max_eval = eval;
                        max_slide_reward = slide_reward;
                        op = i;
                    } else if( eval > max_eval ){
                        max_eval = eval;
                        op = i;
                        max_slide_reward = slide_reward;
                    }
                }
            } */
            
            next_tile = get_next_tile(&bb, tile_bag, tile_bag_indx ,
                    gen_bonus_tile, bonus_tile_seed);
            
            op = expecti_next(&bb ,&max_slide_reward , tilebag_status_transfer(tilebag_status , next_tile ) , next_tile );

            if(op == -1) {
                break;
            }
        
            stat_action_finished(1, op, 0, max_slide_reward);

            // For MS
            if (multistage_mode && biggest_tile_bb(&bb) >= multistage_tile) {
                multistage_output_file = fopen(multistage_output_filename,"ab+");
                //write board number
                fwrite(&bb,sizeof(bb),1,multistage_output_file);
                fwrite(tile_bag,sizeof(tile_bag[0]),TILE_BAG_SIZE,multistage_output_file);
                fwrite(&tile_bag_indx,sizeof(tile_bag_indx),1,multistage_output_file);
                fclose(multistage_output_file);
                j++;
                //print_bitboard_real(bb);
                break;
            }

            next_tile = get_next_tile(&bb, tile_bag, tile_bag_indx ,
                    gen_bonus_tile, bonus_tile_seed);

            if (!gen_bonus_tile){
                //printf("=====\n");
                //printf("%d = ",test_tilebag_status);
                //tilebag_status_show(test_tilebag_status);
                
                //tilebag_status_show(tilebag_status_from_tilebag_content(tile_bag, &tile_bag_indx));
                tilebag_status_transfer_ptr(&tilebag_status, next_tile);
            }
            
            if (gen_bonus_tile)
                bonus_tile_count++;
            else
                tile_bag_indx++;
            
            generated_tile_count++;
            
            
            //place_reward = place( board, op, tile_bag[ tile_bag_indx ], &placed_position);
            place_reward = bitboard_place (&bb, op, next_tile, &placed_position);


            stat_action_finished(0, placed_position, next_tile, place_reward);
            //tile_pb_next_tile(&tp, tile_bag[ tile_bag_indx ]);

            // cancel here's index
            if(tile_bag_indx == TILE_BAG_SIZE){
                regen_tilebag(tile_bag, &tile_bag_indx);
            }

        }
        stat_close_episode();
        if (save_stat){
            stat_save_to_file();
        }


        // max score
        score = cal_board_score_bb(&bb);
        
        if (score > maxscore){
            maxscore = score;
            maxscore_index = j;
        }

        avg_score += score;
        // stat
        stat_board_bb(&bb);
        // stat close episode if user what to save stat file
        
    }
    end_tp_and_show_time_elapesed();
    
    if (multistage_mode){
        printf("Multistage_output Target/Used Episode: %d/%d",total,multistage_used_ep);
        return;
    }
    show_stat();
    printf("Max Score:%u at episode #%d\n", maxscore, maxscore_index);

    avg_score/=total;
    printf("Avg Score:%f\n", avg_score);

    return 0;
}
