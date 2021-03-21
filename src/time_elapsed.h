struct __te {
    struct timeval start, end;
} time_elapsed;

time64_t millisecond_from_epoch () {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (time64_t)(tv.tv_sec) * 1000 + (time64_t)(tv.tv_usec) / 1000;
}
void start_tp(){
    gettimeofday(&(time_elapsed.start), NULL);
}
void end_tp_and_show_time_elapesed(){
    gettimeofday(&(time_elapsed.end), NULL);
    long secs_used=time_elapsed.end.tv_sec - time_elapsed.start.tv_sec - 1;
    long mili_used=time_elapsed.end.tv_usec + 1e6 - time_elapsed.start.tv_usec;
    if ( mili_used >= 1e6 ) ++secs_used, mili_used-= 1e6;
    printf("Used Time: %ld.%ld sec\n",secs_used, mili_used);
}