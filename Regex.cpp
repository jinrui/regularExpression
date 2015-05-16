#include "Regex.h"
#include "Ast.h"
#include "AstCodeGen.h"
#include<algorithm>
#include <iostream>

void print(Ptr root)
{
    if(root == nullptr) return;
    print(root->left);
    print(root->right);
    std::cout<<root->name;
}

Regex::Regex(std::string str)
{
   AstCodeGen codeGen(str);
    Ptr astNode =codeGen.generateAst();
//    print(astNode);
    AstToNfa *astToNfa = new AstToNfa;
    this->node =    astNode->accept(astToNfa);
    this->node->last->finalStatus = true;
   delete astToNfa;
}

void Regex::setStr(std::string str)
{
    exp = str;
    cur = 0;
    success = false;
}

void Regex::addState(std::vector<StatePtr> &l, StatePtr s)
{
    if(s ==  nullptr || find(l.begin(),l.end(),s) != l.end())
        return;
    if(s->out1 && s->out1->ranges.size() != 0 && s->out1->ranges[0].beg == -1)
    {
        if(s->out1->shouldchecked)
        {
            if(s->out1->current_num < s->out1->min_repeat_num)
            {
                s->out1->current_num++;
                addState(l, s->out1->next);
                return;
            }
            else  if(s->out1->current_num >= s->out1->max_repeat_num)
            {
                if(s->out2 != nullptr && s->out2->ranges[0].beg == -1)
                 {
                    s->out1->current_num = 0;
                 }
                 else l.push_back(s);
            }
            else
            {
                s->out1->current_num++;
                l.push_back(s);
                addState(l, s->out1->next);
            }
        }
        else
        {
            l.push_back(s);
            addState(l, s->out1->next);
        }
    }
    else l.push_back(s);
    if(s->out2 && s->out2->ranges.size()!=0 && s->out2->ranges[0].beg == -1)
       addState(l, s->out2->next);
}

/***********************************汤普森nfa匹配，实质是bfs************************************/
void Regex::addNextStates(EdgePtr out)
{
    std::vector<Range> ran = out->ranges;
    StatePtr n1 = out->next; //empty?!?!
    auto isWord = [](char w){return ((w>='0' && w<='9') || (w>='a' && w<='z') || (w>='A' && w<='Z' )|| w == '_');};
    //这里可以添加对正向反向预查的处理
    //TODO
    //预查是设计一条功能边，这条边指向一个预查nfa
    if(out->functionType != -1)
    {
        if(out->functionType == 0 && out->isPostive)
        {
            success = isAssertMatched(out->out, exp.substr(cur));
            if(success)
                addState(next_states, out->next);
        }
        else if(out->functionType == 0 && !out->isPostive)
        {
            success = !isAssertMatched(out->out, exp.substr(cur));
            if(success)
                addState(next_states, out->next);
        }
        else if(out->functionType == 1 && out->isPostive)
        {
//            assertForward = isAssertMatched(out->out, exp.substr(cur));
        }
        else if(out->functionType == 1 && !out->isPostive)
        {
      //      assertForward = !isAssertMatched(out->out, exp.substr(cur));
        }
        return;
    }
    for(Range r:ran)
    {
        if(!out->inverse && cur < exp.length() && exp[cur]>=r.beg && exp[cur]<= r.end)
        {
            addState(current_states_copy, n1);
            break;
        }
        else if(out->inverse && cur < exp.length() && exp[cur]>=r.beg && exp[cur]<= r.end)
        {
            return;
        }
        else if(r.beg == Boundary)
        {
            if((cur == 0 && isWord(exp[0]) ) || (cur == exp.length()  && isWord(exp[cur - 1])) || (!isWord(exp[cur -1]) && isWord(exp[cur]))
            || (!isWord(exp[cur +1]) && isWord(exp[cur])))
            {
                addState(next_states, n1);
            }
        }
        else if(r.beg == NoBoundary)
        {
            if((cur == 0 && !isWord(exp[0]))  || (cur == exp.length() && !isWord(exp[cur -1])) || (isWord(exp[cur -1]) && !isWord(exp[cur]))
            || (isWord(exp[cur +1]) && !isWord(exp[cur])))
            {
                addState(next_states, n1);
            }
        }
        else if(r.beg == Ahead)
        {
            if(cur == 0)
                addState(next_states, n1);
        }
        else if(r.beg == LittleTail)
        {
            if(cur == exp.length())
                addState(next_states, n1);
        }
        else if(r.beg == Tail)
        {
            if(cur == exp.length() || exp[cur] == '\n' )
                addState(next_states, n1);
        }
        else if(r.beg == Begin)
        {
            if(cur == 0)
                addState(next_states, n1);
        }
        else if(r.beg == End)
        {
            if(cur == exp.length())
                addState(next_states, n1);
        }
    }
    if(out->inverse && cur < exp.length())
    {
        addState(current_states_copy, n1);
    }
}

bool Regex::isAssertMatched(StatePtr start, std::string str)
{
    Regex re;
    re.addState( re.current_states,start);
    re.setStr(str);
    while(!re.isSearched())
    {
        re.step();
        if(re.current_states.empty())
            return false;
    }
    re.reSet();
    return true;
}

void Regex::step()
{
        next_states.clear();
        current_states_copy.clear();
        for(const auto& s:current_states)
        {
              if(s->out1 != nullptr)
            {
               addNextStates(s->out1);
            }
              if(s->out2 != nullptr)
            {
               addNextStates(s->out2);
            }
        }
        while(!next_states.empty())
        {
            StatePtr s = *next_states.rbegin();
            next_states.pop_back();
             if(s->out1 != nullptr)
            {
               addNextStates(s->out1);
            }
              if(s->out2 != nullptr)
            {
               addNextStates(s->out2);
            }
            if(!s->out1 && !s->out2 && s->finalStatus)
                addState(current_states_copy, s);
        }
        current_states = current_states_copy;
       if(!current_states.empty() && !success)  //success为真，代表刚成功进入一个assert，而assert是非占位符，不能cur+1。避免assert在末尾的情况。
            ++cur;
        success = false;    //和上面success对应
}

bool Regex::isMatched()
{
     for(auto& s:current_states)
     {
       //     if(s->finalStatus && cur == exp.length() && (s->out1 == nullptr || s->out1->current_num >= s->out1->max_repeat_num && s->out1->min_repeat_num == s->out1->max_repeat_num || s->out1->current_num >= s->out1->min_repeat_num ))
          //       return true;
             if(s->finalStatus && cur == exp.length() )
             {
                    successSet.insert(cur);
                    return true;
             }
     }
     return false;
}

bool Regex::isSearched()
{
     for(auto& s:current_states)
     {
        //    if(s->finalStatus && (s->out1 == nullptr || s->out1->current_num >= s->out1->max_repeat_num && s->out1->min_repeat_num == s->out1->max_repeat_num || s->out1->current_num >= s->out1->min_repeat_num ))
           //      return true;
            if(s->finalStatus)
             {
                    successSet.insert(cur);
                    return true;
             }
     }
     return false;
}

void Regex::reSet()
{
    cur = 0;
    current_states.clear();
    next_states.clear();
    successSet.clear();
    success = false;
}

bool regex_match(std::string str, Regex re)
{
    StatePtr start = re.node->first;
    re.addState( re.current_states,start);
    re.setStr(str);
    while(!re.isMatched())
    {
        re.step();
        if(re.isMatched()) break;
        if(re.current_states.empty())
            return false;
    }
    re.reSet();
    return true;
}

 bool regex_search(std::string str, Regex re)
 {
    StatePtr start = re.node->first;
    bool result = false;
    for(int i = 0;i < str.length();i++)
    {
        re.addState( re.current_states,start);
        re.setStr(str.substr(i));
        result = re.isSearched();
        while(!result)
        {
            re.step();
            result = re.isSearched();
            if(re.current_states.empty())
                break;
        }
         if(result)
         {
            re.reSet();
            return true;
         }
        re.reSet();
    }
    re.reSet();
    return false;
 }

 bool regex_search(std::string str, std::vector<std::string> &match, Regex re)
 {
    StatePtr start = re.node->first;
    bool result = false;
    for(int i = 0;i < str.length();)
    {
        re.addState( re.current_states,start);
        re.setStr(str.substr(i));
        result = re.isSearched();
        while(true)
        {
            re.step();
            result = re.isSearched();
            if(re.current_states.empty())
                break;
        }
        if(re.successSet.size())
        {
            auto tmp = re.successSet.rbegin();
            match.push_back(str.substr(i, *tmp));
            i += *tmp;
            result = true;
        }
        else ++i;
        re.reSet();
    }
    re.reSet();
    return result;
 }

bool regex_replace(std::string text, Regex re, std::string replace)
{
    StatePtr start = re.node->first;
    re.addState( re.current_states,start);
    bool result = false;
    std::string tmp;
    for(int i = 0;i < text.length();)
    {
        re.setStr(text.substr(i));
        bool matched = true;
        while(!re.isSearched())
        {
            re.step();
            if(re.current_states.empty())
            {
                matched = false;
                tmp.append(1, text[i]);
                ++i;
                break;
            }
        }
        if(matched)
        {
            tmp += replace;
            i += re.cur;
            result = true;
        }
        re.reSet();
    }
    text = tmp;
    return result;
}
