#include <iostream>
#include "Thing.hpp"
#include "PartialApplication.hpp"
#include <chrono> 
//#include "PartialApplicationTests.hpp"
#include "FasterFunction.hpp"
//#include "Parser.hpp"

//#include "FasterFunctionTests.hpp"
template<typename T>
using full_decay = typename std::remove_pointer_t<std::decay_t<T>>;
template<typename Arg>
struct TC
{
    using This = TC<Arg>;
    constexpr static inline bool vala = std::is_array_v<Arg>;                                                                                                                                         /* Arg[N]            */
    constexpr static inline bool valaa = std::is_array_v<std::remove_reference_t<Arg>>;                                                                                                               /* Arg(&&/&)?[N]     */
    constexpr static inline bool lvala = std::is_lvalue_reference_v<Arg> && std::is_array_v<std::remove_reference_t<Arg>>;                                                                            /* Arg&[N]           */
    constexpr static inline bool rvala = std::is_rvalue_reference_v<Arg> && std::is_array_v<std::remove_reference_t<Arg>>;                                                                            /* Arg&&[N]          */
    constexpr static inline bool valar = std::is_reference_v<Arg> && std::is_array_v<std::remove_reference_t<Arg>>;                                                                                   /* Arg&&/&[N]        */
    constexpr static inline bool clvala = std::is_lvalue_reference_v<Arg> && std::is_array_v<std::remove_reference_t<Arg>> && std::is_const_v<std::remove_reference_t<Arg>>;                          /* const Arg&[N]     */
    constexpr static inline bool crvala = std::is_rvalue_reference_v<Arg> && std::is_array_v<std::remove_reference_t<Arg>> && std::is_const_v<std::remove_reference_t<Arg>>;                          /* const Arg&&[N]    */
    
    constexpr static inline bool val = std::is_same_v<Arg, full_decay<Arg>> && !This::valaa;                                                                                                          /* Arg               */
    constexpr static inline bool valr = std::is_reference_v<Arg> && !This::valaa;                                                                                                                     /* Arg&/&&([N]!)     */
    constexpr static inline bool lval = std::is_lvalue_reference_v<Arg> && !This::valaa;                                                                                                              /* Arg&              */
    constexpr static inline bool nclval = std::is_lvalue_reference_v<Arg> && !std::is_const_v<std::remove_reference_t<Arg>> && !This::valaa;                                                          /* !const Arg&       */
    constexpr static inline bool rval = std::is_rvalue_reference_v<Arg> && !This::valaa;                                                                                                              /* Arg&&             */
    constexpr static inline bool clval = std::is_lvalue_reference_v<Arg> && std::is_const_v<std::remove_reference_t<Arg>> && !This::valaa;                                                            /* const Arg&        */
    constexpr static inline bool crval = std::is_rvalue_reference_v<Arg> && std::is_const_v<std::remove_reference_t<Arg>> && !This::valaa;                                                            /* const Arg&&       */
    constexpr static inline bool valp = std::is_pointer_v<std::remove_reference_t<Arg>>;                                                                                                              /* Arg*              */
    constexpr static inline bool valpr = std::is_pointer_v<std::remove_reference_t<Arg>> && std::is_reference_v<Arg>;                                                                                 /* Arg*(&&/&)        */
    constexpr static inline bool valpa = std::is_pointer_v<std::remove_reference_t<Arg>>;                                                                                                             /* Arg*(&&/&)?       */
                                                                                                                                                                                                          
};

template<typename in, typename out>
constexpr bool same = std::is_same_v<full_decay<in>, full_decay<out>>;

template<typename in, typename out>
constexpr bool valid_duo = (same<in, out> && (
   (TC<out>::val   && (TC<in>::valr || TC<in>::val))        // Arg           : Arg&/&& || Arg
|| (TC<out>::nclval && (TC<in>::nclval))                    // Arg&          : const! Arg& 
|| (TC<out>::rval  && (TC<in>::rval || TC<in>::val))        // Arg&&         : Arg&& || Arg
|| (TC<out>::clval && (TC<in>::valr || TC<in>::val))        // const Arg&    : Arg&/&&([N]!) || Arg
|| (TC<out>::valp  && (TC<in>::valp || TC<in>::valaa))      // Arg*          : Arg* || Arg(&/&&)?[N]
|| (TC<out>::vala  && (TC<in>::valpa || TC<in>::valaa))     // Arg[N]        : Arg*(&/&&)? || Arg(&/&&)?[N]
|| (TC<out>::lvala && (TC<in>::lvala))                      // Arg&[N]       : Arg(&)[N]
|| (TC<out>::rvala && (TC<in>::rvala))                      // Arg&&[N]      : Arg&&[N]
|| (TC<out>::clvala && (TC<in>::valaa))                     // const Arg&[N] : Arg(&&/&)?[N]
));

using namespace faster;
std::string Apple(std::string a) { return a; }

void typetest1(int&&) {}
void typetest2(int&) {}
void typetest3(const int&) {}
void typetest4(int) {}
void typetest5(int*&&) {}
void typetest6(int*&) {}
void typetest7(const int*&) {}
void typetest8(int * const) {}
void typetest9(int(&&a)[5]) {}
void typetest10(int(&a)[5]) {}
void typetest11(int(a)[5]) {}
void typetest12(const int(&a)[5]) {}
void typetest13(const int(&&a)[5]) {}
void typetest14(const int(&&a)[5]) {}

template<typename T>
void TestType(T&& t) { typetest5(std::forward<T>(t)); };

int main()
{

    int einf = 1;
    int* ap = &einf;

    const int aefa[1]{};
    void* aefa1 = (void*)aefa;
    const int* const aenfa = (const int* const)aefa1;

    typetest8(aefa);

    TC<const int[4]>::valaa;

    valid_duo<const int[4], int* const>;

    TC<const int(&&)[5]>::rvala;

    using Arg = int;
    bool Type1 = TC<Arg&&>            ::rval;
    bool Type2 = TC<Arg&>             ::val;
    bool Type3 = TC<const Arg&>       ::val;
    bool Type4 = TC<Arg*>             ::val;
    bool Type5 = TC<Arg*&&>           ::val;
    bool Type6 = TC<Arg*&>            ::val;
    bool Type7 = TC<const Arg*&>      ::val;
    bool Type8 = TC<Arg*>             ::val;
    bool Type9 = TC<Arg(&&)[5]>       ::val;
    bool TypeA = TC<Arg(&)[5]>        ::val;
    bool TypeB = TC<const Arg(&)[5]>  ::val;
    bool TypeC = TC<Arg[5]>           ::val;

    {
        int a = 1;
        int& b = a;
        const int& c = a;
        int* d = &a;
        int*& e = d;
        int f[5]{ 1, 2, 3, 4, 5 };
        int(&g)[5] = f;
        //TestType((int(&)[5])f);

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
        typetest12(d), typetest12(g), typetest12((int(&&)[5])f), typetest12(e), typetest12(&f);
    }
    
    
    {
        Function fun = [](int a) { return a; };
        int a = 1;
        fun(a);
    }
   // PartialApplicationTests::Run();

    
    
    // Speed testing for all function types
    auto start = std::chrono::high_resolution_clock::now();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
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
