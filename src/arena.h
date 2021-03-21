
void arenaOutputPlace(int pos,int tile,int hint){
    printf("%X%X+%d\n",pos,tile,hint);
}
const char arenaSlideChar[4] = {'U','R','D','L'};
void arenaOutputSlide(int op){
    printf("#%c\n",arenaSlideChar[op]);
}

int arenaGetOp(char c){
    switch(c){
        case 'U':
            return 0;
        case 'R':
            return 1;
        case 'D':
            return 2;
        case 'L':
            return 3;
    }
}