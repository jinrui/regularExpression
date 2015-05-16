#include "AstCodeGen.h"
#include <sstream>
#include <set>
#include <vector>

Errors::Errors(int pos, std::string msg) : position(pos), message(msg)
{

}
 AstCodeGen :: AstCodeGen(std::string _exp) : exp(_exp)
 {
    init();
 }

Ptr AstCodeGen :: generateAst()
{
    parse();
    return node_stack.top();
}

Ptr AstCodeGen::generateCharNode(std::string name, std::vector<Range>  _charRange, bool inverse)
{
    return  Ptr(new AstCharacter(name, Ptr(), Ptr(), _charRange, inverse));
}

Ptr AstCodeGen::generateLinkedNode(std::string name)
{
    Ptr right = node_stack.top();
    node_stack.pop();
    Ptr left = node_stack.top();
    node_stack.pop();
    return Ptr(new AstLinked(name, left, right));
}

Ptr AstCodeGen::generateRepeatNode(std::string name, int beg, int end)
{
    Ptr left = node_stack.top();
    node_stack.pop();
    return Ptr(new AstRepeat(name, left, Ptr(), beg, end));
}

Ptr AstCodeGen::generateParallelNode(std::string name)
{
    Ptr right = node_stack.top();
    node_stack.pop();
    Ptr left = node_stack.top();
    node_stack.pop();
    return Ptr(new AstParallel(name, left, right));
}

Ptr AstCodeGen::generateFunctionNode(std::string name, int functionType, bool isPostive)
{
    Ptr left = node_stack.top();
    node_stack.pop();
    return Ptr(new AstFunction(name, left, nullptr, functionType, isPostive));
}

void AstCodeGen::init()
{
    sign_map['|'] = 1;
    sign_map['+'] = 3;
    sign_map['*'] = 3;
    sign_map['?'] = 3;
    sign_map['&'] = 2;
}

/**************************翻译正则串**************************************************************/
 void AstCodeGen::parse()
{
    while(cur < exp.length())
    {
        bool should_end = false;
        switch(exp[cur])
        {
            case '|':
                parseParallel();
                break;
            case '?':
            case '*':
            case '+':
                parseRepeat();
                break;
            case '(':
                parseParentheses();
                break;
            case ')':
                should_end = true;
                break;
            case '[':
                parseSquareBrackets();
                break;
            case '{':
                praseBrace();
                break;
            case '\\':
                parseEscape();
                break;
            case '^':
                parseCharacter(Begin);
                break;
            case '$':
                parseCharacter(End);
                break;
            default:
                parseCharacter(exp[cur]);
        }
        if(should_end) break;
    }
    while(!sign_stack.empty())
    {
        char sign = sign_stack.top();
        sign_stack.pop();
        if(sign == '(') break;
        Ptr tree;
        if(sign == '|')
        {
            tree = generateParallelNode(std::string(1, sign));
        }
        else
        {
            tree = generateLinkedNode(std::string(1, sign));
        }
        node_stack.push(tree);
    }
 }

/**************************处理大括号字符**************************************************************/
void AstCodeGen::praseBrace()
{
    if(cur == 0)
    {
        errors.push_back(Errors(cur, "{不能出现在首位"));
        return;
    }
    int s = 0,e = 0;
    int pos = cur;
    ++cur;
    std::string sign;
    if(exp[cur] <'0' || exp[cur] >'9')
    {
        errors.push_back(Errors(cur, "{}内只能为数字或者,"));
        return;
    }
    while(exp[cur] >= '0' && exp[cur] <= '9' )
        s = s*10 + exp[cur++] - '0';
    if(exp[cur] == ',')
    {
        ++cur;
        if(exp[cur] >= '0' && exp[cur] <= '9')
            while(exp[cur] >= '0' && exp[cur] <= '9' )
                e = e*10 + exp[cur++] - '0';
        else if(exp[cur] == '}')
            e = MAX_INT;
    }
    else if(exp[cur] == '}')
        e = s;
     if(exp[cur] != '}')
    {
        errors.push_back(Errors(cur, "{}内只能为数字或者,"));
        return;
    }
    sign.append(exp, pos, cur - pos + 1);
    node_stack.push(generateRepeatNode(sign, s, e));
    ++cur;
    should_linked = true;
}

/**************************处理小括号字符**************************************************************/
void AstCodeGen::parseParentheses() //小括号
{
    if(cur == exp.length() - 1)
    {
        errors.push_back(Errors(cur, "(不能出现在末尾"));
        return;
    }
    sign_stack.push('(');
    ++cur;
    if(exp[cur] == '?')
    {
        ++cur;
        switch(exp[cur])
        {
            case '=':
                parseFunction(0, true);
                break;
            case '!':
                parseFunction(0, false);
                break;
            case '<':
                ++cur;
                if(exp[cur] == '=')
                    parseFunction(1, true);
                else if(exp[cur] == '!')
                    parseFunction(1, false);
                else {//命名捕获，TODO};
                break;
            case '#':
                //注释，TODO
                break;
            case ':':
                //不捕获，TODO
                break;
        }
        }
        return;
    }
    bool tmp = should_linked;
    should_linked = false;
    parse();
    if(cur == exp.length())
    {
        errors.push_back(Errors(cur, "没有)"));
        return;
    }
    ++cur;
    if(tmp)
        sign_stack.push('&');
    should_linked = true;
}
/**************************处理中括号字符**************************************************************/
void AstCodeGen::parseSquareBrackets() //中括号
{
    bool inverse = false;
    List checkpoint;
    std::string sign;
    int pos = cur;
    ++cur;
    if(exp[cur] == '^')
    {
        inverse = true;
        ++cur;
    }
    while(exp[cur] != ']')
    {
        if(exp[cur + 1] != '-')
        {
            checkpoint.push_back(Range{exp[cur], exp[cur]});
            ++cur;
        }
        else
        {
            if(exp[cur + 2] == ']')
            {
                checkpoint.push_back(Range{exp[cur], exp[cur]});
                ++cur;
            }
            else
            {
                if(exp[cur + 2] < exp[cur])
                {
                    errors.push_back(Errors(cur, "[]中-前面字符不能大于后面字符"));
                    return;
                }
                checkpoint.push_back(Range{exp[cur], exp[cur+2]});
                cur += 3;
            }
        }
    }
    sign.append(exp, pos, cur - pos + 1);
    ++cur;
    node_stack.push(generateCharNode(sign, checkpoint, inverse));
    if(should_linked)
        sign_stack.push('&');
    should_linked = true;
}

/**************************处理普通字符**************************************************************/
void AstCodeGen::parseCharacter(int cr) //普通字符
{
    if(should_linked)
        sign_stack.push('&');
    List v;
    if(cr == '.')
        v.push_back(Range{0,0xffff});
    else v.push_back(Range{cr,cr});
    node_stack.push(generateCharNode(std::string(1, exp[cur]), v));
    ++cur;
    should_linked = true;
}

/**************************处理转义字符**************************************************************/
void AstCodeGen::parseEscape() //转义字符
{
    std::string sign;
    List v;
    sign.append(1, exp[cur]);
    ++cur;
    sign.append(1, exp[cur]);
    switch(exp[cur])
    {
        case 'd':
            v.push_back( Range{'0','9'});
            break;
        case 'D':
            v.push_back(Range{0,'0'-1});
            v.push_back(Range{'9'+1,127});
            break;
        case 'w':
            v.push_back(Range{'a','z'});
            v.push_back(Range{'A','Z'});
            v.push_back(Range{'0','9'});
            break;
        case 'W':
            v.push_back(Range{0,'0'-1});
            v.push_back(Range{'9'+1,'A'-1});
            v.push_back(Range{'Z'+1,'_'-1});
            v.push_back(Range{'_'+1,'a'-1});
            v.push_back(Range{'z'+1,127});
            break;
        case 's':
            v.push_back(Range{'\t','\r'});
            break;
        case 'f':
            v.push_back(Range{'\f','\f'});
            break;
        case 't':
            v.push_back(Range{'\t','\t'});
            break;
        case 'n':
            v.push_back(Range{'\n','\n'});
            break;
        case 'v':
            v.push_back(Range{'\v','\v'});
            break;
        case 'S':
            v.push_back(Range{0,8});
            v.push_back(Range{14,127});
            break;
        case 'b':
            v.push_back(Range{Boundary,Boundary});
            break;
        case 'B':
            v.push_back(Range{NoBoundary,NoBoundary});
            break;
        case 'A':
            v.push_back(Range{Ahead,Ahead});
            break;
        case 'z':
            v.push_back(Range{LittleTail,LittleTail});
            break;
        case 'Z':
            v.push_back(Range{Tail,Tail});
            break;
        default:
            v.push_back(Range{exp[cur], exp[cur]});
            break;
    }
    node_stack.push(generateCharNode(sign, v));
    ++cur;
    if(should_linked)
        sign_stack.push('&');
    should_linked = true;
}

/**************************处理并联字符**************************************************************/
void AstCodeGen::parseParallel()
{
    if(cur == 0 || cur == exp.length() - 1)
    {
        errors.push_back(Errors(cur, "|不能出现在开始和结尾"));
        return;
    }
    ++cur;
     if(exp[cur] == ')'  || exp[cur] == ']' || exp[cur] == '}' || exp[cur] == '|' || exp[cur] == '*' || exp[cur] == '+' || exp[cur] == '?')
     {
        errors.push_back(Errors(cur, "字符错误"));
        return;
     }
    if(sign_stack.empty())
    {
        sign_stack.push('|');
        should_linked = false;
        return;
    }
    char sign = sign_stack.top();
    while(sign_map['|'] <= sign_map[sign] &&sign != '('&& !sign_stack.empty())
    {
        Ptr tree;
        if(sign == '|')
        {
            tree = generateParallelNode(std::string(1, sign));
        }
        else
        {
            tree = generateLinkedNode(std::string(1, sign));
        }
        node_stack.push(tree);
        sign_stack.pop();
        if(sign_stack.empty()) break;
        sign = sign_stack.top();
    }
    sign_stack.push('|');
    should_linked = false;
}

/**************************处理重复字符**************************************************************/
void AstCodeGen::parseRepeat()
{
    if(cur == 0)
    {
        errors.push_back(Errors(cur, "+/*/? 不能出现在开始和结尾"));
        return;
    }
    std::string sign = std::string(1, exp[cur]);
    ++cur;
    Ptr tree;
    if(sign == "+" )
        tree = generateRepeatNode(sign, 1, MAX_INT);
    else if(sign == "*" )
        tree = generateRepeatNode(sign, 0, MAX_INT);
    else if(sign == "?" )
        tree = generateRepeatNode(sign, 0, 1);
    node_stack.push(tree);
    should_linked = true;
}

/**************************处理零宽断言**************************************************************/
void AstCodeGen::parseFunction(int type, bool flag)
{
    ++cur;
     bool tmp = should_linked;
    should_linked = false;
    parse();
    if(cur == exp.length())
    {
        errors.push_back(Errors(cur, "没有)"));
        return;
    }
    ++cur;
    if(tmp)
        sign_stack.push('&');
    should_linked = true;
    Ptr tree = generateFunctionNode("<?= or ! or <= or <! >", type, flag);
    node_stack.push(tree);
}
