#pragma once
#include <iomanip>

#define match(...) Match2{ __VA_ARGS__ }
#define maybe(...) __VA_ARGS__, [&]

template<typename Arg, typename ...Args>
struct MyTuple : public MyTuple<Args...>
{
    enum { isLast = 0 };

    MyTuple(const Arg& a, const Args& ...rest)
        : MyTuple<Args...>(rest...),
        arg(a)
    {}

    MyTuple<Args...>& Parent() { return *this; }
    size_t Size() { return sizeof...(Args) + 1; }

    Arg& arg;
};

template<typename Arg>
struct MyTuple<Arg>
{
    enum { isLast = 1 };

    MyTuple(const Arg& a)
        : arg(a)
    {}

    Arg& arg;
};

template<typename ...Args>
struct Match2
{
    MyTuple<const Args&...> tuple;

    Match2(const Args& ...a)
        : tuple(a...)
    {}

    template<typename ...Tys>
    inline void operator()(const Tys& ...tys)
    {
        Equal(tys...);
    }

    template<typename F, typename ...Rest>
    inline void Equal(const Args&...args, const F& f, const Rest& ...rest)
    {
        if (EqualOne(tuple, args...))
            f();

        else if constexpr (sizeof...(Rest) > 0)
            Equal(rest...);
    }

    template<typename Arg, typename ...Args>
    inline bool EqualOne(MyTuple<const Arg&, const Args&...>& tuple, const Arg& arg, const Args&...args)
    {
        if constexpr (MyTuple<const Arg&, const Args&...>::isLast)
        {
            if (arg == tuple.arg) return true;
            else return false;
        }
        else
        {
            if (arg == tuple.arg) return EqualOne(tuple.Parent(), args...);
            else return false;
        }
    }
};