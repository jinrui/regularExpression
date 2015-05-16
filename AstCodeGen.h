#ifndef ASTCODEGEN_H
#define ASTCODEGEN_H
#include <stack>
#include <vector>
#include <map>
#include "Ast.h"
#define MAX_INT 0xffff
/**************************************************************
Generate AST
**************************************************************/
typedef AstNode::Ptr Ptr;
typedef std::vector<Range> List;
struct Errors
{
    int position;
    std::string message;
    Errors(int pos, std::string msg);
};
class AstCodeGen
{
    public:
                    std::string exp;
                    int cur = 0;
                    std::stack<char> sign_stack;
                    std::stack<Ptr> node_stack;
                    std::vector<Errors> errors;
                    std::map <char,int> sign_map;
                    bool should_linked = false;

                    Ptr generateAst();
                    Ptr generateCharNode(std::string name, std::vector<Range>  _charRange, bool inverse = false);
                    Ptr generateLinkedNode(std::string name);
                    Ptr generateRepeatNode(std::string name, int _beg, int _end);
                    Ptr generateParallelNode(std::string name);
                    Ptr generateFunctionNode(std::string name, int functionType, bool isPostive);

                    void init();
                    void parse();
                    void praseBrace(); // 大括号
                    void parseParentheses(); //小括号
                    void parseSquareBrackets(); //中括号
                    void parseCharacter(int cr); //普通字符
                    void parseEscape(); //转义字符
                    void parseParallel(); // 并联字符
                    void parseRepeat(); // 重复字符
                    void parseFunction(int type, bool flag); // 零宽断言，负向零宽断言

                    AstCodeGen(std::string exp);

                    void reset();//执行完一次后，重置
};

#endif // AST_H
