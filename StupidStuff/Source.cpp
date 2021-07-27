#include <iostream>
#include "Match.hpp"
#include "Async.hpp"
#include "Thing.hpp"
#include "PartialApplication.hpp"
#include <chrono> 
#include <type_traits>

int Add(Thing& a, Thing& b, Thing& c)
{ 
    return a.v + b.v + c.v; 
}

int main()
{
    Function add = [](int a, int b) { return a + b; };
    Function mul = [](int a, int b) { return a * b; };
    Function sub = [](int a, int b) { return a - b; };

    Function ttt = [](const Function<int(int)>& a, int b) -> int { return a((int)b); };

    Function aae = [](int a) { return a; };

    Function negate = [](int a) { return -a; };

    std::function testf = negate;
    int av = 1;
    int bv = 2;
    int cv = 3;

    int answer = negate << add(av) << bv;
    std::cout << answer << std::endl;
    // (int -> int -> int) -> (int -> int -> int) => ((int -> int -> int) -> int -> int)

    Function<int(int, int, int)> am = add << mul;
    int res = am(1, 2, 3);

    int ans = add << 2 << sub << mul << 2 << 3 << 1;

    std::function func = [](const Thing& a, Thing& b, const Thing& c) -> int { return a.v + b.v + c.v; };
    Function addThings = func;
    
    Thing t1{ 1 };
    Thing t2{ 2 };
    Thing t3{ 3 };

    double n = 100000;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = addThings(t1);
        auto b = a(t2);
        auto c = b(t3);
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = addThings(t1, t2);
        auto b = a(t3);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = addThings(t1, t2, t3);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = func(t1, t2, t3);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        auto a = Add(t1, t2, t3);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;
}
