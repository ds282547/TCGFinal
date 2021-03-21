#include "stdio.h"
#include "stdlib.h"

#include "float.h"
// #include "conio.h"
#include "string.h"
#include "def.h"
#include <sys/time.h>
#include "time_elapsed.h"

#include "tool.h"

#include "tilebag.h"
#include "tilebag_status.h"

#include "board.h"
#include "bitboard.h"
#include "action.h"
#include "tuple_MS.h"
#include "td_MS.h"

#include "expecti.h"

#include "arena.h"

//#include "tile_pb.h"
//#include "bestmove.h"
#include "stat.h"

char * stat_path = NULL;


/*
int counter = 0;
indx testbd[16];

int init_bag[12] = {1,1,1,1,2,2,2,2,3,3,3,3};
int cindex[12] = {0};
int bindex[9] = {0};

bitboard min_board;
int min_hint = -1;

float min_eval = FLT_MAX;


int compose(int n,int kk,int k,int s,int cb[]){
    for(int i=s;i<n;++i){
        cb[k-1] = i;
        if(k>1){
            compose(n,kk,k-1,i+1,cb);
        } else {
            reset_board(testbd);

            for(int j=0;j<9;++j){
                bindex[j] = 0;
            }

            compose2(12,9,9,0,bindex);
            
            
        }
    }
}

int compose2(int n,int kk,int k,int s,int cb[]){
    for(int i=s;i<n;++i){
        cb[k-1] = i;
        if(k>1){
            compose2(n,kk,k-1,i+1,cb);
        } else {
             bitboard mb = 0;

            for(int j=0;j<kk;++j){
                //printf("%d ",cindex[j]);
                bitboard_set(&mb, cindex[j] ,init_bag[bindex[j]]);
            }



            //print_bitboard_real(mb);
            
            for(int h=1;h<=3;++h){
                float temp = get_v_bb(&mb,h);
                //printf("%f\n",temp);
                if(temp < min_eval){
                    min_eval = temp;
                    min_hint = h;
                    min_board = mb;
                }
            }

            //print_board(testbd);

            //printf("\n");
            
            
            counter++;
        }
    }
}

*/

int main(int argc, char const *argv[])
{
    setvbuf(stdout, (char *)NULL, _IONBF, 0);
    // for-loop
    unsigned i, j, k;

    // parse arguments
    char * subarg_delim = ",";
    char * subarg_token;
    char * subarg;

    char * weight_arg = NULL;

    char * load_weight_paths[NETWORK_COUNT];
    unsigned load_weight_path_count = 0;
       

    char * seed_arg = NULL;

    char * depth_arg = NULL;
    
    char * shell_arg = NULL;

    //char * multistage_arg = NULL;
    
    bool save_stat = false;

    bool shell_mode = false;

    unsigned total = 0;


    for(i=1;i<argc;++i){
        if (strstr(argv[i],"--weight")) {
            weight_arg = strchr(argv[i],'=') + 1;
            if ( weight_arg[0] == '\0') {
                weight_arg = NULL;
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
        } else if (strstr(argv[i],"--shell")) {
            shell_mode = true;
            if(strchr(argv[i],'=') != NULL) {
                shell_arg = strchr(argv[i],'=') + 1;
                if ( shell_arg[0] == '\0') {
                    shell_arg = NULL;
                }
            }
        }
    }
    if(!shell_mode){
        printf("This program can only run for shell & arena system!\n");
        return 0;
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
    

    /*
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
    */


    // time measure
    //start_tp();

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
                if (load_weight_path_count >= NETWORK_COUNT){
                    printf("Network number error! Need:%d  Input:%d\n",NETWORK_COUNT,load_weight_path_count);
                    return 0;
                }
                load_weight_paths[load_weight_path_count] = get_arguemt_after_equal( subarg_token );
                load_weight_path_count++;
            } 
            subarg_token = strtok(NULL ,subarg_delim);
        }      
    }


    
    //load-weight

    if (load_weight_path_count){
        printf("Network number = Need:%d  Input:%d\n",NETWORK_COUNT,load_weight_path_count);
        for(i=0;i<load_weight_path_count;++i){
            printf("Load network/weight [%s] PATH=%s\n",network_names[i],load_weight_paths[i]);
            current_network = i;
            load_weight(load_weight_paths[i]);
            check_weight();
        }
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
    // Player Op
    int op;
    // Evil Op
    unsigned evil_pos;
    unsigned evil_tile;
    unsigned evil_hint;

    //reward max_slide_reward,slide_reward, place_reward;
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

    //unsigned ep_step = 1;


    // process arena input
    
    // Find Min Init Board

    /*
    printf("Input Initialize Board:\n");

    
    int cb[9];
    compose(16,9,9,0,cindex);
    printf("Count %d, Min Value %f Min Hint %d",counter,min_eval,min_hint);
    print_bitboard(min_board);
    printf("%llu\n",min_board);

    */

    // arena process
    enum Role{
        Play,
        Evil
    } myRole;


    int initial_place_index;

    //|    1    1    2    2|
    //|    0    0    0    2|
    //|    0    0    2    3|
    //|    3    0    3    0|
    int min_initial_place[10] = {1,1,2,2,2,2,3,3,3,1};
    int min_initial_place_pos[9] = {0,1,2,3,7,10,11,12,14};
    char currentGameID[500] = {0};

    const size_t charNSafe = 500;

    char loginName[500] = {0};
    char dollarLoginName[500] = "$";

    char agent1[50] = {0};
    char agent2[50] = {0};

    
    if (shell_arg == NULL) {
        strncpy(loginName,"0756078",charNSafe);
    } else {
        strncpy(loginName,shell_arg,charNSafe);
    }

    strncat(dollarLoginName,loginName,charNSafe);


    //arena command in
    while(1){
        int i,j;
        char buf[500] = {0};
        char cmd[500] = {0};
        char cmdAtr[500] = {0};
        char gameid[500] = {0};

        

        scanf ("%[^\n]%*c", buf);
        switch(buf[0]){
            case '@':
            //For Command
                //skip space
                i=1;
                while(buf[i]==' ') {
                    if(buf[i]=='\0') break;
                    i++;
                }
                j=i;
                while(buf[j]!=' ' && buf[j]!='\0')
                    j++;
                
                memset(cmd, '\0', sizeof(cmd));
                strncpy(cmd,buf+i,j-i);

                if(strcmp(cmd,"login") == 0){
                    printf("@ login %s|kQRI71EFg8\n",loginName);
                } else if(strcmp(cmd,"exit") == 0){
                    exit(1);
                }
            break;
            case '#':
            // For Comment
                i=1;
                while(buf[i]==' ') {
                    if(buf[i]=='\0') break;
                    i++;
                }
                j=i;
                while(buf[j]!=' ' && buf[j]!='\0')
                    j++;
                
                memset(gameid, '\0', sizeof(gameid));
                strncpy(gameid,buf+i,j-i);
                
                //printf("GameID = %s\n",gameid);
                i=j;while(buf[i]==' ') { if(buf[i]=='\0') break; i++; }
                j=i;while(buf[j]!=' ' && buf[j]!='\0')j++;

                memset(cmd, '\0', sizeof(cmd));
                strncpy(cmd,buf+i,j-i);
                //printf("cmd = %s\n",cmd);

                if(strcmp(cmd,"?") == 0){
                    // Show Game id
                    
                    // Process Action
                    if(myRole == Evil){
                        if(initial_place_index < 9){
                            printf("#%s ",currentGameID);
                            arenaOutputPlace(  min_initial_place_pos[  initial_place_index ],
                                               min_initial_place[ initial_place_index ],
                                               min_initial_place[ initial_place_index+1 ]);


                            

                            bitboard_set(&bb,min_initial_place_pos[  initial_place_index ],min_initial_place[ initial_place_index ]);
                            tilebag_status_transfer_ptr(&tilebag_status , min_initial_place[ initial_place_index ] );

                            stat_action_finished(0, min_initial_place_pos[  initial_place_index ],
                                                    min_initial_place[ initial_place_index ],
                                                   (min_initial_place[ initial_place_index ] == 3) * 3 );

                            initial_place_index++;
                            if(initial_place_index == 9) {
                                generated_tile_count = 10;
                                next_tile = min_initial_place[ initial_place_index ];
                            }
                        } else {
                            // min - max 
                            int min_pos,new_hint;
                            
                            //printf("Last Op:%d, next_tile:%d",op,next_tile);

                            //tilebag_status_show(tilebag_status);
                            //print_bitboard_real(bb);
                            //printf("Last Op=%s", slide_name[op]);


                            //bonus rate for next new hint
                            

                            

                            evil_expecti_next(&bb,op,tilebag_status_transfer(tilebag_status , next_tile ), &next_tile, &min_pos, &new_hint, bonus_tile_count+1, generated_tile_count+1);
                            //printf("MinPos:%d, NewHint:%d",op,next_tile);+
                            //print_bitboard_real(bb);

                            stat_action_finished(0, min_pos, next_tile, score_deltas[next_tile]);

                            tilebag_status_transfer_ptr(&tilebag_status , next_tile );

                            printf("#%s ",currentGameID);
                            arenaOutputPlace(  min_pos,
                                               next_tile,
                                               new_hint);
                            
                            bonus_tile_count += (new_hint == 4) ? 1 : 0;
                            generated_tile_count++;

                            //printf("%d / %d Ok? %d\n",bonus_tile_count,generated_tile_count, ((Float) (bonus_tile_count) / (generated_tile_count)) <= bonus_tile_rate_limit);

                            next_tile = new_hint;            

                        }
                    } else{

                    }







                } else if(strcmp(cmd,"open") == 0){
                    i=j;while(buf[i]==' ') { if(buf[i]=='\0') break; i++; }
                    j=i;while(buf[j]!=':' && buf[j]!='\0')j++;

                    memset(agent1, '\0', sizeof(agent1));
                    strncpy(agent1,buf+i,j-i);

                    //Skip :
                    i=++j;while(buf[i]!=' ' && buf[i]!='\0')i++;

                    memset(agent2, '\0', sizeof(agent2));
                    strncpy(agent2,buf+j,i-j);

                    

                    printf("Check name?%s : %s vs %s\n",dollarLoginName,agent1,agent2);

                    if(strcmp(agent1,dollarLoginName) == 0){
                        myRole = Play;
                    } else if (strcmp(agent2,dollarLoginName) == 0){
                        myRole = Evil;
                    } else {
                        printf("#%s open reject\n",gameid);
                        break;
                    }

                    if(currentGameID[0] == '\0'){
                        strcpy(currentGameID,gameid);
                        printf("#%s open accept\n",gameid);

                        printf("New Game play=%s vs evil=%s Gameid=%s\n",agent1,agent2,gameid);

                        //----- Start A New Game ------ 
                        bb = 0;
                        initial_place_index = 0;
                        tilebag_status = TILEBAG_STATUS_FULL;

                        bonus_tile_count = 0;
                        generated_tile_count = 0;

                        stat_open_episode(agent1,agent2,charNSafe);
                    } else {
                        printf("#%s open reject\n",gameid);
                    }
                
                } else if(strcmp(cmd,"close") == 0){
                    printf("dect End!\n");
                    i=j;while(buf[i]==' ') { if(buf[i]=='\0') break; i++; }
                    j=i;while(buf[j]!=' ' && buf[j]!='\0')j++;


                    memset(cmdAtr, '\0', sizeof(cmdAtr));
                    strncpy(cmdAtr,buf+i,j-i);
                
                    printf("Close Flag = %s\n",cmdAtr);
                    memset(currentGameID, '\0', sizeof(currentGameID));

                    //stat
                    stat_close_episode();
                    if (save_stat){
                        stat_save_to_file();
                    }


                    // max score
                    score = cal_board_score_bb(&bb);
                    
                    if (score > maxscore){
                        maxscore = score;
                    }

                    avg_score += score;

                    total++;
                    // stat
                    stat_board_bb(&bb);
                    // stat close episode if user what to save stat file
                } else {
                    //Action
                    if(myRole == Evil){
                        op = arenaGetOp(cmd[1]);
                        //printf("Slide\n");
                        //print_bitboard_real(bb);
                        reward rw = bitboard_shift[op](&bb);
                        //print_bitboard_real(bb);
                        //printf("Slide End\n");

                        stat_action_finished(1, op, 0, rw);
                    } else if(myRole == Play){
                        sscanf("%1x%1x+%1x",&evil_pos,&evil_tile,&evil_hint);
                        bitboard_set(&bb, evil_pos, evil_tile);
                        stat_action_finished(0,evil_pos,evil_tile,score_deltas[evil_hint]);
                    }
                }

            break;
            // case '%':
            // System Info!

        }

    }

    
    show_stat();
    printf("Max Score:%u at episode #%d\n", maxscore, maxscore_index);

    avg_score/=total;
    printf("Avg Score:%f\n", avg_score);
    
    free_zhashing();


    return 0;

}
