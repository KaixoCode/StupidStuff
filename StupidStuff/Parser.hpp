#pragma once
#include <string_view>
#include <variant>
#include <string>
#include "PartialApplication.hpp"

using namespace faster;

template<typename T>
struct ParseResult
{
    std::string_view remainder;
    T result;
    bool success = false;
};

template<typename>
struct Parser;

// Get the Function object with the N last arguments
template<typename Ret, std::size_t N, typename... T, std::size_t... I>
Parser<Ret(std::tuple_element_t<N + I, std::tuple<T...>>...)> _SubSeqP(std::index_sequence<I...>) {};
template<size_t N, typename Return, typename ...Args>
using _SubParser = std::conditional_t < N == sizeof...(Args) + 1, Return,
    decltype(_SubSeqP<Return, N, Args...>(std::make_index_sequence<sizeof...(Args) - N>{})) > ;

template<typename Ret, typename Arg, typename ...Args>
struct Parser<Ret(Arg, Args...)> : public Function<ParseResult<Ret>(Arg, Args..., std::string_view)>
{
    using Parent = typename Function<ParseResult<Ret>(Arg, Args..., std::string_view)>;
    using Parent::Function;

    Parser(Parent&& f)
        : Parent(f.m_Binder) {}

    Parser(const Parent& f)
        : Parent(f.m_Binder) {}

    Parser(Parent& f)
        : Parent(f.m_Binder) {}

    //, typename = std::enable_if_t<sizeof...(Tys) == sizeof...(Args) && (CompatibleType<Args, Tys> && ...)>
    template<typename ...Tys, typename = std::enable_if_t<_CompatibleTPacks<TPack<Arg, Args...>, TPack<Tys...>>::same>>
    inline _SubParser<sizeof...(Tys) - 1, Ret, Args...> operator()(Tys&& ...tys) const {
        return { Parent::operator()(std::forward<Tys>(tys)...) } ;
    }
};

template<typename Ret>
struct Parser<Ret()> : public Function<ParseResult<Ret>(std::string_view)>
{
    using Parent = typename Function<ParseResult<Ret>(std::string_view)>;
    using Parent::Function;

    Parser(Parent&& f)
        : Parent(f.m_Binder) {}

    Parser(const Parent& f)
        : Parent(f.m_Binder) {}

    Parser(Parent& f)
        : Parent(f.m_Binder) {}

    //, typename = std::enable_if_t<sizeof...(Tys) == sizeof...(Args) && (CompatibleType<Args, Tys> && ...)>
    inline ParseResult<Ret> operator()(std::string_view tys) const {
        return Parent::operator()(tys);
    }
};

template <class T, class Tuple, size_t... Is>
T construct_from_tuple(Tuple&& tuple, std::index_sequence<Is...>) {
    return T{ std::get<Is>(std::forward<Tuple>(tuple))... };
}
template <class T, class Tuple>
T construct_from_tuple(Tuple&& tuple) {
    return construct_from_tuple<T>(std::forward<Tuple>(tuple),
        std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{}
    );
}

// Combinators:
template<typename T, typename R>
Parser<std::tuple<T, R>()> operator*(Parser<T()> a, Parser<R()> b)
{
    Parser<std::tuple<T, R>(Parser<T()>, Parser<R()>)> fun = [](Parser<T()> a, Parser<R()> b, std::string_view v)->ParseResult<std::tuple<T, R>>
    {
        auto r1 = a(v);
        auto r2 = b(r1.remainder);
        if (r1.success && r2.success)
            return { r2.remainder, { r1.result, r2.result }, true };
        else return { v };
    };
    return fun(a, b);
}

template<typename R, typename ...T>
Parser<std::tuple<T..., R>()> operator*(Parser<std::tuple<T...>()> a, Parser<R()> b)
{
    Parser<std::tuple<T..., R>(Parser<std::tuple<T...>()>, Parser<R()>)> fun = [](Parser<std::tuple<T...>()> a, Parser<R()> b, std::string_view v) -> ParseResult<std::tuple<T..., R>>
    {
        auto r1 = a(v);
        auto r2 = b(r1.remainder);
        if (r1.success && r2.success)
            return { r2.remainder, std::tuple_cat(r1.result, std::tuple<R>{r2.result}), true };
        else return { v };
    };
    return fun(a, b);
}

template<typename T>
Parser<T()> operator|(Parser<T()> a, Parser<T()> b)
{
    Parser<T(Parser<T()>, Parser<T()>)> fun = [](Parser<T()> a, Parser<T()> b, std::string_view v) -> ParseResult<T>
    {
        auto r1 = a(v);
        if (r1.success)
            return r1;
        auto r2 = b(v);
        if (r2.success)
            return r2;
        else return { v };
    };
    return fun(a, b);
}

template<typename T, typename R, typename ...Args>
Parser<R(Args...)> operator>(Parser<T()> a, Parser<R(Args...)> b)
{
    Parser<R(Parser<T()>, Parser<R(Args...)>, Args...)> fun = [](Parser<T()> a, Parser<R(Args...)> b, Args... args, std::string_view v) -> ParseResult<R>
    {
        auto r1 = a(v);
        if (!r1.success)
            return { v };
        if constexpr (sizeof...(Args) == 0) {
            auto r2 = b(r1.remainder);
            if (r2.success)
                return r2;
            else return { v };
        } else {
            auto r2 = b(args...)(r1.remainder);
            if (r2.success)
                return r2;
            else return { v };
        }
    };
    return fun(a, b);
}

template<typename T, typename R, typename ...Args>
Parser<T(Args...)> operator<(Parser<T(Args...)> a, Parser<R()> b)
{
    Parser<T(Parser<T(Args...)>, Parser<R()>, Args...)> fun = 
    [](Parser<T(Args...)> a, Parser<R()> b, Args...args, std::string_view v) -> ParseResult<T> {        
        if constexpr (sizeof...(Args) == 0) {
            auto r1 = a(v);
            if (!r1.success)
                return { v };
            auto r2 = b(r1.remainder);
            if (r2.success)
                return { r2.remainder, r1.result, true };
            else return { v };
        } else {
            auto r1 = a(args...)(v);
            if (!r1.success)
                return { v };
            auto r2 = b(r1.remainder);
            if (r2.success)
                return { r2.remainder, r1.result, true };
            else return { v };
        }
    };
    return fun(a, b);
}

// Type casting
template<typename T, typename ...Args>
Parser<T()> Cast(Parser<std::tuple<Args...>()> parser)
{
    Parser<T(Parser<std::tuple<Args...>()>)> fun =
    [](Parser<std::tuple<Args...>()> parser, std::string_view v) -> ParseResult<T> {
        auto r1 = parser(v);
        if (r1.success)
            return { r1.remainder, construct_from_tuple<T>(r1.result), true };
        else
            return { v };
    };
    return fun(parser);
}

template<typename T, typename Arg>
Parser<T()> Cast(Parser<Arg()> parser)
{
    Parser<T(Parser<Arg()>)> fun{ [](Parser<Arg()> parser, std::string_view v) -> ParseResult<T> {
        auto r1 = parser(v);
        if (r1.success)
            return { r1.remainder, T{ r1.result }, true };
        else
            return { v };
    } };
    return fun(parser);
}

template<typename T, typename Arg>
Parser<T()> Cast(Parser<Arg()> parser, T(*conv)(const Arg&))
{
    Parser<T(Parser<Arg()>, T(*)(const Arg&))> fun{ [](Parser<Arg()> parser, T(*conv)(const Arg&), std::string_view v) -> ParseResult<T> {
        auto r1 = parser(v);
        if (r1.success)
            return { r1.remainder, conv(r1.result), true };
        else
            return { v };
    } };
    return fun(parser, conv);
}

// Parser base
struct BasicParser
{
    // Complex parsers
    template<typename T, typename Res = std::conditional_t<std::is_same_v<T, char>, std::string, std::vector<T>>>
    static inline Parser<Res(Parser<T()>)> many{ [](Parser<T()> p, std::string_view v) -> ParseResult<Res> {
        ParseResult<T> res;
        Res all;
        std::string_view rem = v;
        do {
            res = p(rem);
            if (res.success)
                all.push_back(res.result), rem = res.remainder;
        } while (res.success);
        return { rem, all, true };
    } };

    // Basic Parsers
    static inline Parser<char(bool(*)(char))> satisfy = [](bool(*f)(char), std::string_view v) -> ParseResult<char> {
        if (v.size() > 0 && f(v[0])) return { v.substr(1), v[0], true }; else return { v };
    };

    static inline Parser<char(char)> character = [](char c, std::string_view a) -> ParseResult<char> {
        if (a.size() > 0 && a[0] == c) return { a.substr(1), c, true }; else return { a };
    };

    static inline Parser<std::string(std::string)> string = [](std::string c, std::string_view a) -> ParseResult<std::string> {
        if (a.rfind(c, 0) == 0) return { a.substr(c.size()), c, true }; else return { a };
    };

    static inline bool isletter(char c) { return isalpha(c); }
    static inline bool isDigit(char c) { return isdigit(c); }
    static inline int stoi(const std::string& s) { return std::stoi(s); }
    static inline Parser<char()> letter = satisfy(isletter);
    static inline Parser<char()> digit = satisfy(isDigit);
    static inline Parser<std::string()> word = many<char>(letter);
    static inline Parser<std::string()> spaces = many<char>(character(' '));
    static inline Parser<std::string()> identifier = spaces > word < spaces;
    static inline Parser<std::string(std::string)> symbol = spaces > string < spaces;
    static inline Parser<int()> integer = Cast<int>(spaces > many<char>(digit) < spaces, stoi);
};

