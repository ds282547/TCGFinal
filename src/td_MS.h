// Hint Encoder
// Origin: 1 2 3 BonusTile
// Result: 0 1 2 3
int hintEncoder(int hint){

    return hint>3 ? 3 : hint - 1;
}

// bitboard version
Float get_v_bb(bitboard *b, int hintTile){
    unsigned net = biggest_tile_bb(b) >= 12 ? (biggest_tile_bb(b) >= 13 ? 2 : 1) : 0;
    #define wtable Networks[net].wtable
    #define wtableLH Networks[net].wtableLH
    #define wtableLT Networks[net].wtableLT

    if(hintTile == 0)
        printf("\n\n\nError Hint Origin %d\n\n\n",hintTile);

    hintTile = hintEncoder(hintTile);

    if(!(hintTile>=0 && hintTile <=3))
        printf("\n\n\nError Hint%d\n\n\n",hintTile);

    unsigned i,j,k,l;
    // tuples 486
    Float v = 0; 
    for(i=0;i<TUPLE_COUNT;++i){
        for(j=0;j<TUPLE_ISO;++j){
            l = 0;
            for(k=0;k<TUPLE_N;++k)
                l += bitboard_get(b, tuples[i][j][k] ) << (k << 2);
            v += wtable[i][l];
        }
    }
    
    // merge feature
/*    
    if (use_M_tuple)
        v += wtableM[ get_M_index( board ) ];
*/        
    // L-hint feature
    for(j=0;j<TUPLE_ISO;++j){
        l = 0;
        for(k=0;k<TUPLE_LH_N;++k)
            l += bitboard_get(b, tuplesLH[j][k] ) << (k << 2);
        l += hintTile << 10;
        v += wtableLH[l];
    }
    
    // large-tile feature
    v += wtableLT[ get_LT_index_bb( b )  ];
    return v;
}

void adjust_v_bb(bitboard * b, Float delta, int hintTile){
    #define wtable Networks[current_network].wtable
    #define wtableLH Networks[current_network].wtableLH
    #define wtableLT Networks[current_network].wtableLT

    hintTile = hintEncoder(hintTile);

    
    if(!(hintTile>=0 && hintTile <=3))
        printf("\n\n\nError Hint%d\n\n\n",hintTile);

    unsigned i,j,k,l;
    // tuples 486
    for(i=0;i<TUPLE_COUNT;++i){
        for(j=0;j<TUPLE_ISO;++j){
            l = 0;
            for(k=0;k<TUPLE_N;++k)
                l += bitboard_get(b, tuples[i][j][k] ) << (k << 2);
            //printf("Adjust [%u][%u] = %f + %f\n",i,l,wtable[i][l],delta);
            wtable[i][l] += delta;
        }
    }

/*
    if (use_M_tuple)
        wtableM[ get_M_index( board ) ] += delta;
*/    
    // L-hint feature
    for(j=0;j<TUPLE_ISO;++j){
        l = 0;
        for(k=0;k<TUPLE_LH_N;++k)
            l += bitboard_get(b, tuplesLH[j][k] ) << (k << 2);
        l += hintTile << 10;
        wtableLH[l] += delta;
    }
    // large-tile feature
    wtableLT[ get_LT_index_bb( b )  ] += delta;
}
