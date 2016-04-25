#include <stdlib.h>

/* from stdlib.h */
float strtof(const char *nptr, char **endptr)
{
    return (float)strtod(nptr, endptr);
}

double atof(const char *nptr)
{
    return strtod(nptr, NULL);
}

int abs(int __n)
{
    return (__n < 0) ? -__n : __n;
}

long labs(long __n)
{
    return (__n < 0L) ? -__n : __n;
}

long long llabs(long long __n)
{
    return (__n < 0LL) ? -__n : __n;
}

int rand(void)
{
    return (int)lrand48();
}

void srand(unsigned int __s)
{
    srand48(__s);
}

long random(void)
{
    return lrand48();
}

void srandom(unsigned int __s)
{
    srand48(__s);
}
