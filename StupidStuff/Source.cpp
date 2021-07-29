#include <iostream>
#include "Match.hpp"
#include "Async.hpp"
#include "Thing.hpp"
#include "PartialApplication.hpp"
#include "SlowPApp.hpp"
#include <chrono> 
#include <type_traits>
#include <cassert>
//#include "PartialApplicationTests.hpp"
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

struct MyParser : public BasicParser
{
    static inline Parser<VarDecl()> decl = Cast<VarDecl>((symbol("var") > identifier) * (symbol("=") > integer < symbol(";")));
};

int main()
{
    auto carrot = "var carrot = 1000;";
    auto res = MyParser::decl(carrot).result;
    
    int av = 1;
    //int bv = 2;
    //int cv = 3;
    //int dv = 4;

    //auto aa = Add<int> << av << Sub<int> << Mul<int> << bv << cv << dv;
    //
    //int answer = Negate<int> << Add<int> << av << bv;
    //std::cout << answer << std::endl;

    //Function<int(int, int, int)> am = Add<int> << Mul<int>;
    //int res = am(1, 2, 3);

    //int ans = Add<int> << 2 << Sub<int> << Mul<int> << 2 << 3 << 1;
    //{
    //    Function test1 = [](Thing& a, Thing& b, int c) -> int { return a.v + b.v + c; };
    //    
    //    Thing t1{ 4 };
    //    Thing t2{ 1 };
    //    Thing t3{ 3 };
    //    Thing t10{ 10 };

    //    auto addto4 = test1(t1);
    //    auto addto1 = test1(t2);
    //    auto addto3 = test1(t3);

    //    auto addto14 = addto4(t10);
    //    auto addto11 = addto1(t10);
    //    auto addto13 = addto3(t10);

    //    auto res1 = addto14(1);
    //    auto res2 = addto11(2);
    //    auto res3 = addto13(3);
    //    auto res4 = addto11(4);
    //    auto res5 = addto14(5);

    //    Thing::refcount;
    //    int a = 1;
    //}

    std::srand(std::time(nullptr));
    Function basics = [](int* a, int& b, int& c, Thing d) -> int 
    { 
        return *a + b + c + d.v; 
    };
    for (int i = 0; i < 1000000; i++)
    {
        int a = std::rand() % 100;
        int b = std::rand() % 100;
        int c = std::rand() % 100;
        Thing d{ std::rand() % 100 };
        auto res1 = basics(&a);
        auto res2 = res1(b);
        auto res3 = res2(c);
        auto res4 = res3(d);
        assert(res4 == a + b + c + d.v);
    }
    Thing::refcount;
    /*reinterpret_cast<long long>(1.0);
    void* test = reinterpret_cast<void*>();
    double a = (double)reinterpret_cast<long long>(test);*/


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
