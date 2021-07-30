#include <iostream>
#include "Thing.hpp"
#include "PartialApplication.hpp"
#include <chrono> 
#include "PartialApplicationTests.hpp"
//#include "Parser.hpp"

using namespace faster;
std::string Apple(std::string a) { return a; }

void typetest1(int&&) {}
void typetest2(int&) {}
void typetest3(const int&) {}
void typetest4(int) {}
void typetest5(int*&&) {}
void typetest6(int*&) {}
void typetest7(const int*&) {}
void typetest8(int*) {}
void typetest9(int(&&a)[5]) {}
void typetest10(int(&a)[5]) {}
void typetest11(int[5]) {}

template<typename T>
void TestType(T&& t) { typetest5(std::forward<T>(t)); };

int main()
{
    {
        int a = 1;
        int& b = a;
        const int& c = a;
        int* d = &a;
        int*& e = d;
        int f[5]{ 1, 2, 3, 4, 5 };
        int(&g)[5] = f;
        TestType((int(&)[5])f);

        typetest1(1);
        typetest2(a);
        typetest3(1), typetest3(a), typetest3(b), typetest3(c);
        typetest4(1), typetest4(a), typetest4(b), typetest4(c);
        //typetest5(f), typetest5(g), typetest5(&a), typetest5(f);
        //typetest6(g);
        //typetest7();
        //typetest8();
        //typetest9();
        //typetest10();
        //typetest11();
    }
    
    
    {
        Function fun = [](int a) { return a; };
        int a = 1;
        fun(a);
    }
    PartialApplicationTests::Run();

    
    
    // Speed testing for all function types
    auto start = std::chrono::high_resolution_clock::now();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    auto lambda = [](const Thing& a, Thing& b, const Thing& c, int d, int e, int f) -> int { return a.v + b.v + c.v + d + e + f; };
    std::function func = lambda;
    Function addThings = lambda;

    Thing t1{ 1 };
    Thing t2{ 2 };
    Thing t3{ 3 };

    auto lambda2 = [&](const Thing& a, Thing& b, const Thing& c, int d, int e, int f) -> int { return t1.v + t2.v + t3.v + a.v + b.v + c.v + d + e + f; };
    std::function func2 = lambda2;
    Function addThings2 = lambda2;

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
