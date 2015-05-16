#ifndef REGEX_H
#define REGEX_H
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <set>
#include "Nfa.h"

/******************匹配字符串，由于c++和正则表达式都支持转义字符，要注意对文本的匹配，比如匹配文本\,需要\\\\才行，
这里需要一个trick**************************************************************************************
****************************************************************************************************/

class Regex
{
    public:
                    std::string exp;
                    int cur = 0;
                    std::vector<StatePtr> current_states;
                    std::vector<StatePtr> current_states_copy;
                    std::vector<StatePtr> next_states;

                    std::set<int> successSet;
                    NfaPtr node;
                    int asserBeg = -1;   //断言开始
                    int asserEnd = -1;   //断言结束
                    bool success = false;

                    void addState(std::vector<StatePtr> &l, StatePtr s);
                    void addNextStates(EdgePtr out);
                    void step();
                    void setStr(std::string str);
                    bool shouldEnd();
                    bool isMatched();
                    bool isSearched();
                    void reSet();
                    bool isAssertMatched(StatePtr start, std::string str);


                    Regex() = default;
                    Regex(std::string exp);
};
bool regex_match(std::string str, Regex re);
bool regex_search(std::string str, Regex re);
bool regex_search(std::string str, std::vector<std::string> &match, Regex re);
bool regex_replace(std::string text, Regex re, std::string replace);
#endif // REGEX_H
