#ifndef RANGE_H
#define RANGE_H
#define Boundary -2             // \b
#define NoBoundary -3       // \B
#define Ahead -4                    // \A
#define LittleTail -5                             // \z
#define Tail          -6             // \Z
#define  Begin -7
#define End -8

struct Range
{
    int beg;
    int end;
    bool operator < (const Range &ran)const
    {
            return beg < ran.beg;
    }
};

#endif
