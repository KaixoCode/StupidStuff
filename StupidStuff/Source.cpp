#include <iostream>
#include "Thing.hpp"
#include "PartialApplication.hpp"
#include <chrono> 
#include <iomanip>
#include "PartialApplicationTests.hpp"
#include "FasterFunctionTests.hpp"
#include "FasterFunction.hpp"
#include "Parser.hpp"

using namespace faster;

template<typename T>
void ConcatVector(std::vector<T>& a, std::vector<T>& b)
{
    a.insert(a.end(), b.begin(), b.end());
}

struct MyParser : public BasicParser {

    enum Operator { Add, Sub, Mul, Leq, Geq, Eq, Neq, And, Or, Ass, AMu, AAd, ASu };
    enum UOperator { Not, Incr, Decr, Plus, Neg };

    static void PrintOp(Operator op)
    {
        switch (op) {
        case Add: std::cout << "+ "; break;
        case Sub: std::cout << "- "; break;
        case Mul: std::cout << "* "; break;
        case Leq: std::cout << "<= "; break;
        case Geq: std::cout << ">= "; break;
        case Eq: std::cout << "== "; break;
        case Neq: std::cout << "!= "; break;
        case And: std::cout << "&& "; break;
        case Or: std::cout << "|| "; break;
        case Ass: std::cout << "= "; break;
        case AMu: std::cout << "*= "; break;
        case AAd: std::cout << "+= "; break;
        case ASu: std::cout << "-= "; break;
        }
    }

    static void PrintUOp(UOperator op)
    {
        switch (op) {
        case Not: std::cout << "! "; break;
        case Incr: std::cout << "++ "; break;
        case Decr: std::cout << "-- "; break;
        case Plus: std::cout << "+ "; break;
        case Neg: std::cout << "- "; break;
        }
    }

    struct ExprPart
    {
        enum Type { iden, num, opr, uopr };
        ExprPart(const std::string& ident) : type(iden), ident(ident) {}
        ExprPart(int val) : type(num), val(val) {}
        ExprPart(Operator op) : type(opr), op(op) {}
        ExprPart(UOperator uop) : type(uopr), uop(uop) {}
        ExprPart(const ExprPart& copy) : ident("") {
            type = copy.type;
            switch (type) {
            case iden: ident = std::string{ copy.ident }; break;
            case num: val = copy.val; break;
            case opr: op = copy.op; break;
            case uopr: uop = copy.uop; break;
            }
        }
        ExprPart& operator=(const ExprPart& copy) {
            type = copy.type;
            switch (type) {
            case iden: ident = std::string{ copy.ident }; break;
            case num: val = copy.val; break;
            case opr: op = copy.op; break;
            case uopr: uop = copy.uop; break;
            }
            return *this;
        }
        ~ExprPart() {};
        Type type;
        union {
            std::string ident;
            int val;
            Operator op;
            UOperator uop;
        };
        void Print() {
            switch (type) {
            case iden: std::cout << ident << " "; break;
            case num: std::cout << val << " "; break;
            case opr: PrintOp(op); break;
            case uopr: PrintUOp(uop); break;
            }
        }
    };

    using Expr = std::vector<ExprPart>;

    static Expr ToExpr(const OneOf<std::string, int, Expr>& val) {
        Expr expr;
        visit(val,
            [&](const std::string& v) { expr.push_back(ExprPart{ v }); },
            [&](int v) { expr.push_back(ExprPart{ v });},
            [&](const Expr& v) { expr = v; }
        );
        return expr;
    }

    Parser<Operator()>& op(Operator oper, std::string op) {
        return *new Parser<Operator()>{ [=](std::string_view v) -> ParseResult<Operator> {
            if (v.rfind(op, 0) == 0) return { v.substr(op.size()), oper, true };            
            return { v };
        } };
    };

    Parser<UOperator()>& uop(UOperator oper, std::string op) {
        return *new Parser<UOperator()>{ [=](std::string_view v) -> ParseResult<UOperator> {
            if (v.rfind(op, 0) == 0) return { v.substr(op.size()), oper, true };
            return { v };
        } };
    };

    Parser<Expr(Parser<Expr()>&, Parser<Operator()>&)> exprParser = [](Parser<Expr()>& p, Parser<Operator()>& op, std::string_view v) ->ParseResult<Expr> {
        auto a = p(v); if (!a.success) return { v };
        Expr parts = a.result;
        auto remainder = a.remainder;
        while (true) {
            auto f = op(remainder);  if (!f.success) return { remainder, parts, true };
            auto b = p(f.remainder); if (!b.success) return { remainder, parts, true };
            remainder = b.remainder;
            ConcatVector(parts, b.result);
            parts.push_back(f.result);
        }
        return { remainder, parts, true };
    };

    Parser<Expr(Parser<Expr()>&, Parser<UOperator()>&)> postfixExprParser = [](Parser<Expr()>& p, Parser<UOperator()>& op, std::string_view v) ->ParseResult<Expr> {
        auto a = p(v); if (!a.success) return { v };
        Expr parts = a.result;
        auto remainder = a.remainder;
        while (true) {
            auto f = op(remainder); if (!f.success) return { remainder, parts, true };
            remainder = f.remainder;
            parts.push_back(f.result);
        }
        return { remainder, parts, true };
    };

    Parser<Expr(Parser<Expr()>&, Parser<UOperator()>&)> prefixExprParser = [](Parser<Expr()>& p, Parser<UOperator()>& op, std::string_view v) ->ParseResult<Expr> {
        Expr parts;
        auto remainder = v;
        while (true) {
            auto f = op(remainder); if (!f.success) break;
            remainder = f.remainder;
            parts.push_back(f.result);
        }
        auto a = p(remainder); if (!a.success) return { v };
        ConcatVector(a.result, parts);
        return { a.remainder, a.result, true };
    };

    Parser<Operator()> 
        multiplicativeOperator = op(Mul, "*"),                  
        additiveOperator       = op(Add, "+")  | op(Sub, "-"), 
        relationalOperator     = op(Leq, "<=") | op(Geq, ">="),     
        equalityOperator       = op(Eq, "==")  | op(Neq, "!="),
        andOperator            = op(And, "&&"),
        orOperator             = op(Or, "||"), 
        assignOperator         = op(Ass, "=")  | op(AMu, "*=") | op(AAd, "+=") | op(ASu, "-=");

    Parser<UOperator()> postfixOperator = uop(Incr, "++") | uop(Decr, "--");
    Parser<UOperator()> prefixOperator  = uop(Incr, "++") | uop(Decr, "--") | uop(Plus, "+") | uop(Neg, "-") | uop(Not, "!");

    Parser<Expr()> primaryExpression, expression, postfixExpression, prefixExpression, multiplicativeExpression, 
        additiveExpression, relationalExpression, equalityExpression, andExpression, orExpression, assignExpression;

    MyParser() {
        primaryExpression        = Cast<Expr>(identifier | integer | character('(') > expression < character(')'), ToExpr);
        postfixExpression        = postfixExprParser(primaryExpression, postfixOperator);
        prefixExpression         = prefixExprParser(postfixExpression, prefixOperator);
        multiplicativeExpression = exprParser(prefixExpression, multiplicativeOperator);
        additiveExpression       = exprParser(multiplicativeExpression, additiveOperator);
        relationalExpression     = exprParser(additiveExpression, relationalOperator);
        equalityExpression       = exprParser(relationalExpression, equalityOperator);
        andExpression            = exprParser(equalityExpression, andOperator);
        orExpression             = exprParser(andExpression, orOperator);
        assignExpression         = exprParser(orExpression, assignOperator);
        expression               = assignExpression;
    }
};

struct Object {
    int val = 1;

    void Add(int a, Thing b, Thing& c) {
        val += a + b.v + c.v;
    }


};

int main()
{
    auto start = std::chrono::high_resolution_clock::now();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    std::cout << std::fixed << std::setprecision(10);
    PartialApplicationTests::Run();
    fun::FasterFunctionTests::Run();
    //std::cout << "current binder count: " << _BinderBase::refcount << std::endl;
    //start = std::chrono::high_resolution_clock::now();
    //MyParser parser;
    //std::string_view num = "++a-b*!c*(3---3+5*33223252+(apple-juice*2+-3))";
    //auto resa = parser.expression(num);
    //stop = std::chrono::high_resolution_clock::now();
    //duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    //std::cout << "parsing took: " << duration.count() * 0.000000001 << "s" << std::endl;
    //std::cout << "current binder count: " << _BinderBase::refcount << std::endl;
    //for (auto& i : resa.result)
    //    i.Print();
    //MyParser::Expr& expr = resa.result;

    //auto number = MyParser::factor(num);
    int a = 1;

    
    {
        Object obj;
        Function member = { &Object::Add, obj };
        Thing a{ 1 };
        auto res1 = member(1);
        auto res2 = res1(1);
        res2(a);
        int v = obj.val;
    }








    // Speed testing for all function types
    auto lambda = [](const Thing& a, Thing& b, const Thing& c, int d, int e, int f) -> int { return a.v + b.v + c.v + d + e + f; };
    std::function func = lambda;
    Function addThings = lambda;
    fun::Function fastThings = lambda;

    Thing t1{ 1 };
    Thing t2{ 2 };
    Thing t3{ 3 };

    auto lambda2 = [&](const Thing& a, Thing& b, const Thing& c, int d, int e, int f) -> int { return t1.v + t2.v + t3.v + a.v + b.v + c.v + d + e + f; };
    std::function func2 = lambda2;
    Function addThings2 = lambda2;
    fun::Function fastThings2 = lambda2;

    double n = 1000000;

    std::cout << "New Func capture 6 calls" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = addThings2(t1);
        auto b = a(t2);
        auto c = b(t3);
        auto d = c(1);
        auto e = d(2);
        auto f = e(3);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;

    std::cout << "New Func capture 4 calls" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = addThings2(t1, t2);
        auto b = a(t3);
        auto d = b(1, 2);
        auto e = d(3);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;

    std::cout << "New Func capture 1 call" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = addThings2(t1, t2, t3, 1, 2, 3);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;

    std::cout << "New Func 6 calls" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = addThings(t1);
        auto b = a(t2);
        auto c = b(t3);
        auto d = c(1);
        auto e = d(2);
        auto f = e(3);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;

    std::cout << "New Func 4 calls" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = addThings(t1, t2);
        auto b = a(t3);
        auto d = b(1, 2);
        auto e = d(3);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;

    std::cout << "New Func 1 call" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = addThings(t1, t2, t3, 1, 2, 3);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;
    
    std::cout << "faster Function capture 1 call" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = fastThings2(t1, t2, t3, 1, 2, 3);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;

    std::cout << "faster Function 1 call" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = fastThings(t1, t2, t3, 1, 2, 3);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;

    std::cout << "std::function capture 1 call" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = func2(t1, t2, t3, 1, 2, 3);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;

    std::cout << "std::function 1 call" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = func(t1, t2, t3, 1, 2, 3);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;

    std::cout << "lambda capture 1 call" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = lambda2(t1, t2, t3, 1, 2, 3);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;

    std::cout << "lambda 1 call" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = lambda(t1, t2, t3, 1, 2, 3);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;
}
