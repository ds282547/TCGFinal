int uniform_distribution(int rangeLow, int rangeHigh) {
    double myRand = rand()/(1.0 + RAND_MAX); 

    int range = rangeHigh - rangeLow + 1;
    int myRand_scaled = (myRand * range) + rangeLow;
    return myRand_scaled;
}
int uniform_distribution_r(int r,int rangeLow, int rangeHigh) {
    double myRand = r/(1.0 + RAND_MAX); 

    int range = rangeHigh - rangeLow + 1;
    int myRand_scaled = (myRand * range) + rangeLow;
    return myRand_scaled;
}

void swap(indx* a,indx* b){
    if (a == b) return;
    *a ^= *b ^= *a ^= *b;
}
void permute_array(indx arr[], const unsigned n){
    for(unsigned i=0;i<n;++i) {
        swap( arr+i, arr + uniform_distribution(0,n-1));
    }
}

void print_array(indx arr[], const unsigned n){
    for(unsigned i=0;i<n;++i)
        printf("%3d", arr[i]);
        printf("\n");
}

void seedtime(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned int millisecondsSinceEpoch =
    (unsigned int)(tv.tv_sec) * 1000 +
    (unsigned int)(tv.tv_usec) / 1000;
    srand( (unsigned int) millisecondsSinceEpoch );
}

char * get_arguemt_after_equal (const char * arg) {
    char * temp = strchr(arg,'=') + 1;
    if ( temp[0] == '\0') return NULL;
    char * result = (char*)malloc(strlen(temp)+1);
    strcpy(result,temp);
    return temp;
}

#ifdef __linux__
#include <math.h>
char *itoa(unsigned n,char *buf,int radix)
{
    int len = 0;
    unsigned temp = n;
    while(n){
        n/=10;
        len++;
    }
    snprintf(buf, len+1, "%u", temp);

    return   buf;
}

#endif
