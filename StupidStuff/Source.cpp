#include <iostream>
#include "Match.hpp"
#include "Async.hpp"
#include "Thing.hpp"
#include "PartialApplication.hpp"
#include <chrono> 
#include <type_traits>

using namespace stupid_stuff;


int main()
{
    


    int av = 1;
    int bv = 2;
    int cv = 3;
    int dv = 4;

    auto aa = Add<int> << av << Sub<int> << Mul<int> << bv << cv << dv;

    int answer = Negate<int> << Add<int> << av << bv;
    std::cout << answer << std::endl;

    Function<int(int, int, int)> am = Add<int> << Mul<int>;
    int res = am(1, 2, 3);

    int ans = Add<int> << 2 << Sub<int> << Mul<int> << 2 << 3 << 1;

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
    std::vector<int> things{ 4, 5, 3, 2, 1 };
    for (int i = 0; i < n; i++)
    {
        auto resa = Sum<int>(things);
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    std::cout << duration.count() / n << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++)
    {
        int res = 0;
        for (int i = 0; i < things.size(); i++)
            res += things[i];
    }
    stop = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
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
}
