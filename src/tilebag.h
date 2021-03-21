void tilebag_open_episode(indx tile_bag[], unsigned * index){
    for(unsigned i=0;i<TILE_BAG_SIZE;++i){
        tile_bag[i] = (i%3)+1;
    }
    permute_array(tile_bag, TILE_BAG_SIZE);
    //show_tilebag(tile_bag);
    *index = 0;
}
void regen_tilebag(indx tile_bag[], unsigned * index){
    *index = 0;
    //show_tilebag(tile_bag);
    permute_array(tile_bag, TILE_BAG_SIZE);
    //show_tilebag(tile_bag);
}
void show_tilebag(indx tile_bag[]){
    printf("Current Tilebag:");
    for(unsigned i=0;i<TILE_BAG_SIZE;++i){
        printf("%d,",tile_bag[i]);
    }
    printf("\n");
}
