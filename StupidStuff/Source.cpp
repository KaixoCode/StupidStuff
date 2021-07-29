#include <iostream>
#include "Match.hpp"
#include "Async.hpp"
#include "Thing.hpp"
#include "PartialApplication.hpp"
#include "SlowPApp.hpp"
#include <chrono> 
#include <type_traits>
#include <cassert>
#include "PartialApplicationTests.hpp"
#include "Parser.hpp"

struct AppleRes
{
    std::string word1;
    std::string spaces;
    std::string word2;
};

struct VarDecl
{
    std::string name;
    int value;
};

struct FunDecl
{
    std::string name;
    std::vector<std::string> args;
};

struct MyParser : public BasicParser
{
    static inline Parser<VarDecl()> deff =
        Cast<VarDecl>((symbol("var") > identifier));

    static inline Parser<VarDecl()> decl =
        Cast<VarDecl>((symbol("var") > identifier) * (symbol("=") > integer < symbol(";")));

    static inline Parser<FunDecl()> func =
        Cast<FunDecl>((symbol("function") > identifier) * (symbol("(") > sepBy<std::string, std::string>(identifier, symbol(",")) < symbol(")")));
};

int main()
{
    PartialApplicationTests::Run();

    auto carrot = "function AppleJuice(carrot, apple)";
    auto res = MyParser::func(carrot).result;
    
    // Speed testing for all function types
    auto lambda = [](const Thing& a, Thing& b, const Thing& c, int d, int e, int f) -> int { return a.v + b.v + c.v + d + e + f; };
    std::function func = lambda;
    Function addThings = lambda;

    Thing t1{ 1 };
    Thing t2{ 2 };
    Thing t3{ 3 };

    double n = 1000000;

    std::cout << "New Func 6 calls" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = addThings(t1);
        auto b = a(t2);
        auto c = b(t3);
        auto d = c(1);
        auto e = d(2);
        auto f = e(3);
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
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
    
    std::cout << "std::function 1 call" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = func(t1, t2, t3, 1, 2, 3);
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
