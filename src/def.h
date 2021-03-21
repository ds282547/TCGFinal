#define cell(r,c) (r<<2)+c
#define false 0
#define true 1


#define TUPLE_COUNT 4
#define TUPLE_ISO 8
#define TUPLE_N 6
#define TUPLE_LH_N 5

#define WEIGHT_TABLE_COUNT 6u
#define WEIGHT_LT 1024
#define WEIGHT_SLOT 16777216u


const unsigned N = 16;
const unsigned TILE_BAG_SIZE = 12;
const unsigned INITIAL_PLACE = 9;

typedef unsigned indx;
typedef int reward;

typedef float Float;
typedef Float weight;
typedef unsigned bool;

typedef unsigned action;

typedef unsigned long long uint64_t;
typedef long long time64_t;

bool use_M_tuple = false;

// bitboard

//#define bitboard unsigned long long
typedef unsigned long long bitboard;
bitboard bitboard_mask[16];
unsigned bitboard_shift_table[0xffff];
unsigned bitboard_shift_score_table[0xffff];

void bitboard_set(bitboard* b,unsigned pos,bitboard tile);
unsigned bitboard_get(bitboard* b,unsigned pos);

// bonus_tile rate

const float bonus_tile_rate_limit = 1.0/21;

unsigned SEARCH_DEPTH = 2;
unsigned SEARCH_DEPTH_P = 2;
