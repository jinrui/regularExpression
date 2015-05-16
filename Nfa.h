#ifndef NFA_H
#define NFA_H
#include "Range.h"
#include <memory>
struct Edge;
struct State;
struct Nfa;
typedef Edge* EdgePtr;
typedef State* StatePtr;
typedef Nfa* NfaPtr;
static int stateId = 0;
struct State
{
    int id = stateId++;
    EdgePtr out1 = nullptr;
    EdgePtr out2 = nullptr;
    bool finalStatus = false;
    ~State()
    {
        if(out1 != nullptr)
            delete out1;
        if(out2 != nullptr)
            delete out2;
        out1 = nullptr;
        out2 = nullptr;
    }
};
struct Edge
{
    std::vector<Range> ranges;
    StatePtr prev;
    StatePtr next;
    StatePtr out;
    int min_repeat_num;
    int max_repeat_num;
    int current_num = 0;
    int functionType = -1;
    bool isPostive;
    bool inverse;
    bool shouldchecked = false;

    Edge( std::vector<Range> _ranges, StatePtr _prev, StatePtr _next,int _min_repeat_num,int _max_repeat_num,int _current_num, bool _shouldchecked, bool _inverse = false):
    ranges(_ranges),prev(_prev),next(_next),min_repeat_num(_min_repeat_num),max_repeat_num(_max_repeat_num),
    current_num(_current_num), shouldchecked(_shouldchecked), inverse(_inverse){}
    Edge(std::vector<Range> _ranges, StatePtr _prev, StatePtr _next, bool _inverse = false): ranges(_ranges),prev(_prev),next(_next), inverse(_inverse){}
    Edge(int _functionType, bool _isPositive, StatePtr _prev, StatePtr _next, StatePtr _out) : functionType(_functionType), isPostive(_isPositive),prev(_prev),next(_next),out(_out){}

    ~Edge()
    {
        if(prev != nullptr)
            delete prev;
        if(next != nullptr)
            delete next;
        prev = nullptr;
        next = nullptr;
    }
};
struct Nfa
{
    StatePtr first;
    StatePtr last;
};


#endif // Nfa_H
