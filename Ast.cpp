#include "Ast.h"
/**************************************************************
AST node
**************************************************************/
AstNode::AstNode(std::string _name, Ptr _left, Ptr _right) : name(_name), left(_left), right(_right)
{
}

AstNode::~AstNode()
{
}

AstCharacter::AstCharacter(std::string _name, Ptr _left, Ptr _right, std::vector<Range>  _charRange, bool _inverse) :
AstNode(_name, _left, _right), charRange(_charRange),inverse(_inverse)
{
}

AstLinked::AstLinked(std::string _name, Ptr _left, Ptr _right) :
AstNode(_name, _left, _right)
{
}

AstRepeat::AstRepeat(std::string _name, Ptr _left, Ptr _right, int _beg, int _end) :
AstNode(_name, _left, _right), beg(_beg), end(_end)
{
}

AstParallel::AstParallel(std::string _name, Ptr _left, Ptr _right) :
AstNode(_name, _left, _right)
{
}

AstFunction::AstFunction(std::string _name, Ptr _left, Ptr _right, int _functionType, bool _isPostive) :
AstNode(_name, _left, _right), functionType(_functionType), isPositive(_isPostive)
{
}

NfaPtr AstCharacter::accept(AstVisitor *visitor)
{
    return visitor->visit(this);
}

NfaPtr AstLinked::accept(AstVisitor *visitor)
{
    return visitor->visit(this);
}

NfaPtr AstRepeat::accept(AstVisitor *visitor)
{
    return visitor->visit(this);
}

NfaPtr AstParallel::accept(AstVisitor *visitor)
{
    return visitor->visit(this);
}

NfaPtr AstFunction::accept(AstVisitor *visitor)
{
    return visitor->visit(this);
}

NfaPtr AstCharacter::generateNfa()
{
    NfaPtr nfa(new Nfa());
    StatePtr s1(new State());
    StatePtr s2(new State());
    EdgePtr edge(new Edge(this->charRange, s1, s2, inverse));
    s1->out1 = edge;
    nfa->first = s1;
    nfa->last = s2;
    return nfa;
}

//过期了，因为edge中是weakptr，不能保证接下来的state不被析构
NfaPtr AstLinked::generateNfa()
{
    AstToNfa *ast_to_nfa = new AstToNfa;
    NfaPtr nfa_left = left->accept(ast_to_nfa);
    NfaPtr nfa_right = right->accept(ast_to_nfa);
    List l;
    l.push_back(CLOSURE);
    EdgePtr edge(new Edge(l, nfa_left->last, nfa_right->first));
    nfa_left->last->out2 = edge;
    delete ast_to_nfa;
    return new Nfa{nfa_left->first, nfa_right->last};
}

//防止连续的重复，比如(123+)+
NfaPtr AstRepeat::generateNfa()
{
    AstToNfa *ast_to_nfa = new AstToNfa;
    NfaPtr nfa_left = left->accept(ast_to_nfa);
    StatePtr s1 = new State();
    StatePtr s2 = new State();
    List l;
    l.push_back(CLOSURE);
    EdgePtr edge1 = new Edge(l, s1, nfa_left->first);
    s1->out1 = edge1;
    EdgePtr edge2 = new Edge(l, nfa_left->last, s2);
    nfa_left->last->out2 = edge2;
    if(this->beg == 0)
    {
        EdgePtr edge1 = new Edge(l, s1, s2);
        s1->out2 = edge1;
    }
    if(this->end == 0)
    {
        s1->out1 = s1->out2 ;
        s1->out2 = nullptr;
    }
    else
    {
        EdgePtr edge = new Edge(l, s2, s1, beg-1, end - 1, 0 ,true, false);
        s2->out1 = edge;
    }
    StatePtr s3 = new State();
    StatePtr s4 = new State();
    EdgePtr edge3 = new Edge(l, s3, s1);
    s3->out1 = edge3;
    EdgePtr edge4= new Edge(l, s2, s4);
    s2->out2 = edge4;
    return new Nfa{s3, s4};
}

NfaPtr AstParallel::generateNfa()
{
    AstToNfa *ast_to_nfa = new AstToNfa;
    NfaPtr nfa_left = left->accept(ast_to_nfa);
    NfaPtr nfa_right = right->accept(ast_to_nfa);
    StatePtr s1(new State());
    StatePtr s2(new State());
    List l;
    l.push_back(CLOSURE);
    EdgePtr edge1(new Edge(l, s1, nfa_left->first));
    EdgePtr edge2(new Edge(l, s1, nfa_right->first));
    EdgePtr edge3(new Edge(l, nfa_left->last, s2));
    EdgePtr edge4(new Edge(l, nfa_right->last, s2));
    s1->out1 = edge1;
    s1->out2 = edge2;
    nfa_left->last->out2 = edge3;
    nfa_right->last->out2 = edge4;
    delete ast_to_nfa;
    return new Nfa{s1, s2};
}

NfaPtr AstFunction::generateNfa()
{
    AstToNfa *ast_to_nfa = new AstToNfa;
    NfaPtr nfa_left = left->accept(ast_to_nfa);
    StatePtr s1(new State());
    StatePtr s2(new State());
    EdgePtr edge1(new Edge(functionType, isPositive, s1, s2, nfa_left->first));
    s1->out1 = edge1;
    nfa_left->last->finalStatus = true;
    delete ast_to_nfa;
    return new Nfa{s1, s2};
}
/**************************************************************
AST visitor
**************************************************************/

NfaPtr AstToNfa::visit(AstCharacter * node)
{
    return node->generateNfa();
}
NfaPtr AstToNfa::visit(AstLinked * node)
{
    return node->generateNfa();
}
NfaPtr AstToNfa::visit(AstRepeat * node)
{
    return node->generateNfa();
}
NfaPtr AstToNfa::visit(AstParallel * node)
{
    return node->generateNfa();
}
NfaPtr AstToNfa::visit(AstFunction * node)
{
    return node->generateNfa();
}
