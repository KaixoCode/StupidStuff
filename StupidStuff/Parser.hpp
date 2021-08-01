#pragma once
#include <string_view>
#include <variant>
#include <string>
#include "PartialApplication.hpp"

template <typename T>
struct Exactly {
    template <typename U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
    operator U() const;
};

template <typename To, typename From>
To unsafe_variant_cast(From&& from)
{
    return std::visit([](auto&& elem) -> To {
        using U = std::decay_t<decltype(elem)>;
        if constexpr (std::is_constructible_v<To, Exactly<U>>) {
            return To(std::forward<decltype(elem)>(elem));
        }
        else {
            throw std::runtime_error("Bad type");
        }
        }, std::forward<From>(from));
}


struct PtrBase {
    PtrBase() { refcount++; }
    size_t refcount = 0;
};

template<typename T>
struct PtrContainer : public PtrBase
{
    PtrContainer(const T& arg)
        : val(arg) {}

    T val;
};

template<typename T>
struct MyPtr
{
    MyPtr() {}

    MyPtr(const T& arg) 
        : ptr(new PtrContainer<T>{ arg }) {}

    MyPtr(const MyPtr<T>& copy)
        : ptr(copy.ptr) {
        ptr->refcount++;
    }

    MyPtr(MyPtr<T>&& copy)
        : ptr(copy.ptr) {
        copy.ptr = nullptr;
    }

    ~MyPtr() {
        if (ptr == nullptr)
            return;
        ptr->refcount--;
        if (ptr->refcount == 0)
            delete ptr, ptr = nullptr;
    }

    operator T& () { return ptr->val; }
    operator const T& () const { return ptr->val; }
    
    T& operator*() { return ptr->val; }
    const T& operator*() const { return ptr->val; }

    T& Ref() { return ptr->val; }
    const T& Ref() const { return ptr->val; }

    PtrContainer<T>* ptr = nullptr;
};

template<typename ...Ts>
using Tuple = std::tuple<Ts...>;

template<typename ...Ts>
using OneOf = std::variant<MyPtr<Ts>...>;

template<typename T, typename ...Args>
struct IndexOfSame;

template<typename T, typename ...Args>
struct IndexOfSame<T, T, Args...> {
    enum { index = sizeof...(Args) };
};

template<typename T, typename A, typename ...Args>
struct IndexOfSame<T, A, Args...> : public IndexOfSame<T, Args...> {
};

template<typename ...T, typename ...R>
auto visit(const OneOf<T...>& t, R... r)
{
    std::visit([=](auto&& v) mutable {
        using CType = std::remove_reference_t<decltype(v.Ref())>;
        using Type = std::decay_t<decltype(v.Ref())>;
        std::get<sizeof...(T) - IndexOfSame<Type, T...>::index - 1>(std::forward_as_tuple(r...))(static_cast<CType&&>(v.Ref()));

        }, t);
}

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
using _SubParser = std::conditional_t<N == sizeof...(Args), ParseResult<Return>,
    decltype(_SubSeqP<Return, N == sizeof...(Args) ? 0 : N, Args..., void, void>(std::make_index_sequence<sizeof...(Args) - (N == sizeof...(Args) ? 0 : N) - 1>{}))&>;

template<typename Ret, typename ...Args>
struct Parser<Ret(Args...)> : public faster::Function<ParseResult<Ret>(Args..., std::string_view)>
{
    using Parent = typename faster::Function<ParseResult<Ret>(Args..., std::string_view)>;
    using Parent::Function;

    Parser(Parent&& f)
        : Parent(f.m_Binder) {}

    Parser(const Parent& f)
        : Parent(f.m_Binder) {}

    Parser(Parent& f)
        : Parent(f.m_Binder) {}

    //, typename = std::enable_if_t<sizeof...(Tys) == sizeof...(Args) && (CompatibleType<Args, Tys> && ...)>
    template<typename ...Tys, typename = std::enable_if_t<faster::_CompatibleTPacks<faster::TPack<Tys...>, faster::TPack<Args..., std::string_view>>::same>>
    inline _SubParser<sizeof...(Tys), Ret, Args..., void> operator()(Tys&& ...tys) const {
        if constexpr (std::is_same_v<_SubParser<sizeof...(Tys), Ret, Args..., void>, ParseResult<Ret>>)
            return Parent::operator()(std::forward<Tys>(tys)...);
        else
            return *new std::remove_reference_t<_SubParser<sizeof...(Tys), Ret, Args..., void>>{ Parent::operator()(std::forward<Tys>(tys)...) };
    }
};

template <class T, class Tuple, size_t... Is>
T construct_from_tuple(const Tuple& tuple, std::index_sequence<Is...>) {
    return T{ std::get<Is>(tuple)... };
}
template <class T, class Tuple>
T construct_from_tuple(const Tuple& tuple) {
    return construct_from_tuple<T>(tuple,
        std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{}
    );
}

// Combinators:
template<typename T, typename R>
Parser<Tuple<T, R>()>& operator*(Parser<T()>& a, Parser<R()>& b)
{
    Parser<Tuple<T, R>(Parser<T()>&, Parser<R()>&)> fun = [](Parser<T()>& a, Parser<R()>& b, std::string_view v)->ParseResult<Tuple<T, R>>
    {
        auto r1 = a(v);
        if (!r1.success)
            return { v };
        auto r2 = b(r1.remainder);
        if (r2.success)
            return { r2.remainder, Tuple<T, R>{ std::tuple<T, R>{ r1.result, r2.result } }, true };
        else return { v };
    };
    return *new Parser<Tuple<T, R>()>{ fun(a, b) };
}

template<typename R, typename ...T>
Parser<Tuple<T..., R>()>& operator*(Parser<Tuple<T...>()>& a, Parser<R()>& b)
{
    Parser<Tuple<T..., R>(Parser<Tuple<T...>()>&, Parser<R()>&)> fun = [](Parser<Tuple<T...>()>& a, Parser<R()>& b, std::string_view v) -> ParseResult<Tuple<T..., R>>
    {
        auto r1 = a(v);
        if (!r1.success)
            return { v };
        auto r2 = b(r1.remainder);
        if (r2.success)
            return { r2.remainder, Tuple<T..., R>{ std::tuple_cat(r1.result, std::tuple<R>{ r2.result }) }, true };
        else return { v };
    };
    return *new Parser<Tuple<T..., R>()>{ fun(a, b) };
}

template<typename T, typename R>
Parser<OneOf<T, R>()>& operator|(Parser<T()>& a, Parser<R()>& b)
{
    Parser<OneOf<T, R>(Parser<T()>&, Parser<R()>&)> fun = [](Parser<T()>& a, Parser<R()>& b, std::string_view v) -> ParseResult<OneOf<T, R>>
    {
        auto r1 = a(v);
        if (r1.success)
            return { r1.remainder, r1.result, r1.success };
        auto r2 = b(v);
        if (r2.success)
            return { r2.remainder, r2.result, r2.success };
        else return { v };
    };
    return *new Parser<OneOf<T, R>()>{ fun(a, b) };
}

template<typename R, typename ...T>
Parser<OneOf<T..., R>()>& operator|(Parser<OneOf<T...>()>& a, Parser<R()>& b)
{
    Parser<OneOf<T..., R>(Parser<OneOf<T...>()>&, Parser<R()>&)> fun = [](Parser<OneOf<T...>()>& a, Parser<R()>& b, std::string_view v) -> ParseResult<OneOf<T..., R>>
    {
        auto r1 = a(v);
        if (r1.success)
            return { r1.remainder, unsafe_variant_cast<OneOf<T..., R>>(r1.result), true };
        auto r2 = b(v);
        if (r2.success)
            return { r2.remainder, r2.result, true };
        else return { v };
    };
    return *new Parser<OneOf<T..., R>()>{ fun(a, b) };
}

template<typename R>
Parser<R()>& operator|(Parser<R()>& a, Parser<R()>& b)
{
    Parser<R(Parser<R()>&, Parser<R()>&)> fun = [](Parser<R()>& a, Parser<R()>& b, std::string_view v) -> ParseResult<R>
    {
        auto r1 = a(v);
        if (r1.success)
            return r1;
        auto r2 = b(v);
        if (r2.success)
            return r2;
        else return { v };
    };
    return *new Parser<R()>{ fun(a, b) };
}

template<typename T, typename R, typename ...Args>
Parser<R(Args...)>& operator>(Parser<T()>& a, Parser<R(Args...)>& b)
{
    Parser<R(Parser<T()>&, Parser<R(Args...)>&, Args...)> fun = [](Parser<T()>& a, Parser<R(Args...)>& b, Args... args, std::string_view v) -> ParseResult<R>
    {
        auto r1 = a(v);
        if (!r1.success)
            return { v };
        auto r2 = b(args..., r1.remainder);
        if (r2.success)
            return r2;
        else return { v };
    };
    return *new Parser<R(Args...)>{ fun(a, b) };
}

template<typename T, typename R, typename ...Args>
Parser<T(Args...)>& operator<(Parser<T(Args...)>& a, Parser<R()>& b)
{
    Parser<T(Parser<T(Args...)>&, Parser<R()>&, Args...)> fun =
    [](Parser<T(Args...)>& a, Parser<R()>& b, Args...args, std::string_view v) -> ParseResult<T> {
        auto r1 = a(args..., v);
        if (!r1.success)
            return { v };
        auto r2 = b(r1.remainder);
        if (r2.success)
            return { r2.remainder, r1.result, true };
        else return { v };
    };
    return *new Parser<T(Args...)>{ fun(a, b) };
}

template<typename T, typename Arg>
Parser<T()>& Cast(Parser<Arg()>& parser)
{
    Parser<T(Parser<Arg()>&)> fun{ [](Parser<Arg()>& parser, std::string_view v) -> ParseResult<T> {
        auto r1 = parser(v);
        if (r1.success)
            return { r1.remainder, T{ r1.result }, true };
        else
            return { v };
    } };
    return *new Parser<T()>{ fun(parser) };
}

template<typename T, typename Arg>
Parser<T()>& Cast(Parser<Arg()>& parser, T(*conv)(const Arg&))
{
    Parser<T(Parser<Arg()>&, T(*)(const Arg&))> fun{ [](Parser<Arg()>& parser, T(*conv)(const Arg&), std::string_view v) -> ParseResult<T> {
        auto r1 = parser(v);
        if (r1.success)
            return { r1.remainder, conv(r1.result), true };
        else
            return { v };
    } };
    return *new Parser<T()>{ fun(parser, conv) };
}

// Parser base
struct BasicParser
{
    // Complex parsers
    template<typename T, typename Res = std::conditional_t<std::is_same_v<T, char>, std::string, std::vector<T>>>
    static inline Parser<Res(Parser<T()>&)> many1{ [](Parser<T()>& p, std::string_view v) -> ParseResult<Res> {
        ParseResult<T> res;
        Res all;
        std::string_view rem = v;
        res = p(rem);
        if (res.success)
            all.push_back(res.result), rem = res.remainder;
        else
            return { v };
        do {
            res = p(rem);
            if (res.success)
                all.push_back(res.result), rem = res.remainder;
        } while (res.success);
        return { rem, all, true };
    } };

    template<typename T, typename Res = std::conditional_t<std::is_same_v<T, char>, std::string, std::vector<T>>>
    static inline Parser<Res(Parser<T()>&)> many{ [](Parser<T()>& p, std::string_view v) -> ParseResult<Res> {
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

    template<typename T, typename R>
    static inline Parser<std::vector<T>(Parser<T()>&, Parser<R()>&)> sepBy{ [](Parser<T()>& p, Parser<R()>& r, std::string_view v) -> ParseResult<std::vector<T>> {
        ParseResult<T> res;
        std::vector<T> all;
        std::string_view rem = v;
        do {
            res = p(rem);
            if (res.success)
            {
                all.push_back(res.result), rem = res.remainder;
                ParseResult<R> res2 = r(rem);
                if (!res2.success)
                    break;
                rem = res2.remainder;
            }
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
    static inline auto letter = satisfy(isletter);
    static inline auto digit = satisfy(isDigit);
    static inline auto word = many1<char>(letter);
    static inline auto spaces = many<char>(character(' '));
    static inline auto identifier = spaces > word < spaces;
    static inline auto symbol = spaces > string < spaces;
    static inline auto integer = Cast<int>(spaces > many1<char>(digit) < spaces, stoi);
};

