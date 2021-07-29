#include <iostream>
#include "Match.hpp"
#include "Async.hpp"
#include "Thing.hpp"
#include "PartialApplication.hpp"
#include "SlowPApp.hpp"
#include <chrono> 
#include <type_traits>
#include <cassert>

using namespace faster;

template<typename T>
struct ParseResult
{
    std::string_view remainder;
    T result;
    bool success = false;
};

template<typename T, typename ...Args>
using Parser = Function<ParseResult<T>(Args..., std::string_view&)>;

template<typename T, typename R>
Parser<std::tuple<T, R>> operator>(const Parser<T>& a, const Parser<R>& b)
{
    return[a = std::move(a), b = std::move(b)](std::string_view& v)->ParseResult<std::tuple<T, R>>
    {
        auto r1 = a(v);
        auto r2 = b(r1.remainder);
        if (r1.success && r2.success)
            return { r2.remainder, { r1.result, r2.result }, true };
        else return { v };
    };
}

template<typename R, typename ...T>
Parser<std::tuple<T..., R>> operator>(const Parser<std::tuple<T...>>& a, const Parser<R>& b)
{
    return[a = std::move(a), b = std::move(b)](std::string_view& v)->ParseResult<std::tuple<T..., R>>
    {
        auto r1 = a(v);
        auto r2 = b(r1.remainder);
        if (r1.success && r2.success)
            return { r2.remainder, std::tuple_cat(r1.result, std::tuple<R>{r2.result}), true };
        else return { v };
    };
}

Parser<char, char> character{ [](char c, std::string_view& a) -> ParseResult<char> {
    if (a[0] == c) return { a.substr(1), c, true }; else return { a };
} };

Parser<std::string, const char*> identifier{ [](const char* c, std::string_view& a) -> ParseResult<std::string> {
    if (a.rfind(c, 0) == 0)
        return { a.substr(strlen(c)), c, true };
    else
        return { a };
} };

template<typename T>
Parser<std::vector<T>, const Parser<T>&> many{ [](const Parser<T>& p, std::string_view& v) -> ParseResult<std::vector<T>> {
    ParseResult<T> res;
    std::vector<T> all;
    std::string_view rem = v;
    do {
        res = p(rem);
        if (res.success)
            all.push_back(res.result), rem = res.remainder;
    } while (res.success);
    return { rem, all, true };
} };

template<typename T>
Parser<std::vector<T>, Parser<T>> many1{ [](const Parser<T>& p, std::string_view& v) -> ParseResult<std::vector<T>> {
    ParseResult<T> res;
    std::vector<T> all;
    std::string_view rem = v;
    do {
        res = p(rem);
        if (res.success)
            all.push_back(res.result), rem = res.remainder;
    } while (res.success);
    return { rem, all, true };
} };


int MyAdd(const Thing& a, Thing& b, const Thing& c) {
    return a.v + b.v + c.v;
}

#include "PartialApplicationTests.hpp"

int main()
{
    PartialApplicationTests::Run();

    //CompareTypes<TPack<decltype("aa")>, TPack<const char*, int>>::same;

    //std::is_constructible_v<const char*, decltype("aaa")>;

    //auto parseApple = many<std::string>(identifier("apple")) > character('a');

    //std::string_view carrot = "appleappleapple";
    //
    //std::tuple<std::vector<std::string>, char> res13r = parseApple(carrot).result;

    //int av = 1;
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
