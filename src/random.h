struct RanCtx 
{ 
	unsigned long long a;
	unsigned long long b;
	unsigned long long c;
	unsigned long long d;
} ranctx_;
unsigned long long rotate_rand_num(unsigned long long x, int k) {
		return (x << k) | (x >> (64 - k));
}
unsigned long long random_value() 
{
    unsigned long long e = ranctx_.a - rotate_rand_num(ranctx_.b, 7);
    ranctx_.a = ranctx_.b ^ rotate_rand_num(ranctx_.c, 13);
    ranctx_.b = ranctx_.c + rotate_rand_num(ranctx_.d, 37);
    ranctx_.c = ranctx_.d + e;
    ranctx_.d = e + ranctx_.a;
    return ranctx_.d;
}
void random_init(unsigned long long seed ) 
{
    unsigned long long i;
    ranctx_.a = 0xf1ea5eed, ranctx_.b = ranctx_.c = ranctx_.d = seed;
    for (unsigned i = 0; i < 20; ++i) {
        random_value();
    }
}
