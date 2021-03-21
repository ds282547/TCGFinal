#include "string.h"
struct __stat{
    unsigned end_tile_count[15];
    unsigned *total;
    float end_frac[15];
    float win_frac[15];
    char player_name[500];
    char evil_name[500];
} stat;

struct __stat_actions{
    unsigned action_count;
    time64_t start_tp;
    time64_t last_action_tp;
    time64_t end_tp;
    unsigned long long all_op_total;
    unsigned long long evil_op_total;
    unsigned long long player_op_total;
    time64_t all_op_time;
    time64_t evil_op_time;
    time64_t player_op_time;

    char *filepath;
    FILE *fp;
} stat_actions;
struct __stat_action {
    // tile    pos  - type
    // XXXX    XXXX   0 (place) 9 bits    
    //          dir - type
    //           XX - 1 (slide) 3 bits
    unsigned op;  
    unsigned time;
    unsigned rwd;
} stat_action_collection[10000];
void initialize_stat(unsigned *_total){
    memset(stat.end_tile_count,0,sizeof(stat.end_tile_count[0]) * 14);
    stat.total = _total;
    strncpy(stat.player_name,"dummy",20);
    strncpy(stat.evil_name,"random",20);

    stat_actions.all_op_total = 0;
    stat_actions.evil_op_total = 0;
    stat_actions.player_op_total = 0;
    stat_actions.all_op_time = 0;
    stat_actions.evil_op_time = 0;
    stat_actions.player_op_time = 0;
}
void stat_board(indx board[]){
    stat.end_tile_count[ biggest_tile (board) ]++;
}
void stat_board_bb(bitboard *b){
    stat.end_tile_count[ biggest_tile_bb (b) ]++;
}

void setup_stat_file(char *path){
    stat_actions.filepath = path;
    // clear file
    fclose(fopen(path, "w"));
}
void stat_open_episode(char *ag1,char *ag2,int safeN){
    if(ag1 != NULL)
        strncpy(stat.player_name,ag1,safeN);
    if(ag2 != NULL)
        strncpy(stat.evil_name,ag2,safeN);

    stat_actions.last_action_tp = stat_actions.start_tp = millisecond_from_epoch();
    stat_actions.action_count = 0;
}

void stat_action_finished(unsigned type,unsigned pos_or_dir,unsigned tile,unsigned rwd){
    unsigned op = type;
    time64_t curr_tp = millisecond_from_epoch();
    if(type){
        //slide type=1
        op = op| pos_or_dir << 1;
    } else{
        //place type=0
        op = op | pos_or_dir << 1 | tile << 5;
    }

    stat_action_collection[ stat_actions.action_count ].time = curr_tp - stat_actions.last_action_tp;
    if (stat_action_collection[ stat_actions.action_count ].time)
        stat_actions.last_action_tp = curr_tp;
    stat_action_collection[ stat_actions.action_count ].rwd = rwd;
    stat_action_collection[ stat_actions.action_count++ ].op = op;
}
void stat_action_initial_place(indx poses[],indx tile_bag[]){
    for(unsigned i=0; i<INITIAL_PLACE; ++i){
        stat_action_finished(0, poses[i] ,tile_bag[i],(tile_bag[i]==3)*3);
    }
}
void stat_close_episode(){
    stat_actions.end_tp = millisecond_from_epoch();
    stat_actions.all_op_time += stat_actions.end_tp - stat_actions.start_tp;

    unsigned op;
    for(unsigned i=0;i<stat_actions.action_count;++i){
        stat_actions.all_op_total++;
        op = stat_action_collection[i].op;
        if( op & 1 ){
            // record time & op
            stat_actions.player_op_total++;
            stat_actions.player_op_time += stat_action_collection[i].time;
        }else{
            // record time & op
            stat_actions.evil_op_total++;
            stat_actions.evil_op_time += stat_action_collection[i].time;
        }
       
    }
}
void stat_save_to_file(){
    const char dir_map[4] = {'U','R','D','L'};
    unsigned op, i;
    stat_actions.fp = fopen(stat_actions.filepath,"a+");
    fprintf(stat_actions.fp,"%s:%s@%lld|",stat.player_name,stat.evil_name,stat_actions.start_tp);
    for(unsigned i=0;i<stat_actions.action_count;++i){
        op = stat_action_collection[i].op;
        if( op & 1 ){
            // slide
            fprintf(stat_actions.fp, "#%c", dir_map[op >> 1] );
        }else{
            // place
            fprintf(stat_actions.fp, "%01X%01X",op >> 1 & 0xf, op >> 5);
        }
        if(stat_action_collection[i].rwd){
            fprintf(stat_actions.fp, "[%u]",stat_action_collection[i].rwd);
        }
        if(stat_action_collection[i].time){
            fprintf(stat_actions.fp, "(%u)",stat_action_collection[i].time);
        }
        
        
        
    }
    fprintf(stat_actions.fp,"|%s@%lld\n",stat.evil_name,stat_actions.end_tp);
    fclose(stat_actions.fp);
}
void show_stat(){
    float accu = 0;
    unsigned i;


    printf("=====Speed=====\n");
    printf("[all op]\ttotal=%llu\ttime=%llu ms\tspeed=%llu ops\n",stat_actions.all_op_total,stat_actions.all_op_time
    , stat_actions.all_op_total * 1000 / stat_actions.all_op_time);
    printf("[player op]\ttotal=%llu\ttime=%llu ms\tspeed=%llu ops\n",stat_actions.player_op_total,stat_actions.player_op_time
    , stat_actions.player_op_total * 1000 / stat_actions.player_op_time);

    if ( stat_actions.evil_op_time ) {
        printf("[evironment op]\ttotal=%llu\ttime=%llu ms\tspeed=%llu ops\n",stat_actions.evil_op_total,stat_actions.evil_op_time
        , stat_actions.evil_op_total * 1000 / stat_actions.evil_op_time);
    } else {
        printf("[evironmecharNnt op]\ttotal=%llu\ttime=%llu ms\tspeed=inf ops\n",stat_actions.evil_op_total,stat_actions.evil_op_time);
    }


    for(i=14;i>=4;--i){
        stat.end_frac[i] = (float)stat.end_tile_count[i] / (*stat.total) * 100;
        accu += stat.end_tile_count[i];
        stat.win_frac[i] = accu / (*stat.total) * 100;
    }
    printf("================\nTILE     WIN     END\n");
    for(i=4;i<=14;++i){
        if(stat.end_tile_count[i])
            printf("%6d %6.2f%% %6.2f%% \n", indx_mapping[i], stat.win_frac[i], stat.end_frac[i]);
    }
}
