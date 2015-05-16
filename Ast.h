#ifndef AST_H
#define AST_H
#include <memory>
#include <vector>
#include "Range.h"
#include "Nfa.h"
#define CLOSURE Range{-1,-1}

typedef std::vector<Range> List;

class AstVisitor;
/**************************************************************
AST node
**************************************************************/
class AstNode : public std::enable_shared_from_this<AstNode>
{
    public:
                    typedef  std::shared_ptr<AstNode>    Ptr;
                    std::string name;
                    Ptr left;
                    Ptr right;

                    virtual NfaPtr accept(AstVisitor *visitor) {return NfaPtr();};
                    virtual NfaPtr generateNfa(){return NfaPtr();};

                    AstNode(std::string _name, Ptr _left, Ptr _right);
                    virtual ~AstNode();
};

class AstCharacter : public AstNode
{
    public:
                    std::vector<Range>  charRange;
                    bool inverse = false;

                     NfaPtr accept(AstVisitor *visitor) override;
                     NfaPtr generateNfa()  override;

                    AstCharacter(std::string _name, Ptr _left, Ptr _right, std::vector<Range>  _charRange, bool _inverse = false);
};

class AstLinked : public AstNode
{
    public:
                     NfaPtr accept(AstVisitor *visitor) override;
                     NfaPtr generateNfa()  override;

                    AstLinked(std::string _name, Ptr _left, Ptr _right);
};

class AstRepeat : public AstNode
{
    public:
                    int beg;
                    int end;

                     NfaPtr accept(AstVisitor *visitor) override;
                     NfaPtr generateNfa()  override;

                    AstRepeat(std::string _name, Ptr _left, Ptr _right, int _beg, int _end);
};

class AstParallel : public AstNode
{
    public:
                    NfaPtr accept(AstVisitor *visitor) override;
                    NfaPtr generateNfa()  override;

                    AstParallel(std::string _name, Ptr _left, Ptr _right);
};

class AstFunction : public AstNode
{
    public:
                    int functionType;
                    bool isPositive;

                    NfaPtr accept(AstVisitor *visitor) override;
                    NfaPtr generateNfa()  override;

                    AstFunction(std::string _name, Ptr _left, Ptr _right, int _functionType, bool _isPostive);
};

/**************************************************************
AST visitor
**************************************************************/
class AstVisitor
{
    public:
                    virtual NfaPtr  visit(AstCharacter * node) = 0;
                    virtual NfaPtr  visit(AstLinked * node) = 0;
                    virtual NfaPtr  visit(AstRepeat * node) = 0;
                    virtual NfaPtr  visit(AstParallel * node) = 0;
                    virtual NfaPtr  visit(AstFunction * node) = 0;
                    virtual ~AstVisitor(){};
};

class AstToNfa : public AstVisitor
{
     public:
                    NfaPtr  visit(AstCharacter * node) override;
                    NfaPtr  visit(AstLinked * node) override;
                    NfaPtr  visit(AstRepeat * node) override;
                    NfaPtr  visit(AstParallel * node) override;
                    NfaPtr  visit(AstFunction * node) override;
};
#endif // AST_H
