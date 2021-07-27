#pragma once
#include <thread>
#include <functional>
#include <future>

template<typename T> struct deduce_type;
template<typename Ret, typename C, typename... Args>
struct deduce_type<Ret(C::*)(Args...) const>
{
    using type = std::function<Ret(Args...)>;
};

template<typename T> struct Async;

template<typename R, typename ...Args>
struct Async<std::function<R(Args...)>> : private std::function<R(Args...)> {
    using Func = std::function<R(Args...)>;

    Async(const Func& t) : Func(t) {}

    std::future<R> operator()(const Args&...args) {
        return std::async([this, &args...]() { return ((Func*)this)->operator()(args...); });
    }
};

class _Async {} _AsyncOp;
#define async _AsyncOp >>
class _Await {} _AwaitOp;
#define await _AwaitOp <<

template<typename Ret, typename...Args>
auto operator>>(_Async, Ret(t)(Args...)) { return Async<std::function<Ret(Args...)>>{ t }; }

template<typename T>
auto operator>>(_Async, const T& t) { return Async<typename deduce_type<decltype(&T::operator())>::type>{ t }; }

template<typename T>
auto operator<<(_Await, T&& t) { return t.get(); }