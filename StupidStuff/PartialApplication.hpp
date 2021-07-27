#pragma once
#include <functional>

template<typename _Tx> struct GetFunctionImpl;
template<typename Ret> struct FunctionImpl;

// Partial application function definition
template<typename T>
struct Function : public GetFunctionImpl<T>::type {
    using FuncSignature = T;

    template<typename F, typename = GetFunctionImpl<T>::IsCallable<F>>
    Function(F t) : GetFunctionImpl<T>::type(t) {}
    Function(T* t) : GetFunctionImpl<T>::type(t) {}
};

// Get a sub sequence from a template pack
template<typename Ret, std::size_t N, typename... T, std::size_t... I>
Function<Ret(std::tuple_element_t<N + I, std::tuple<T...>>...)> sub(std::index_sequence<I...>) {};

// Check for compatible types, remove reference/const and check if same type.
template<typename T1, typename T2>
constexpr bool CompatibleType = (std::is_same_v<std::remove_const_t<std::remove_reference_t<T1>>, std::remove_const_t<std::remove_reference_t<T2>>>);

// Function implementation for no arguments
template<typename Ret> 
struct FunctionImpl<Ret()> {
    template<typename T> 
    FunctionImpl(T&& fun) 
        : fun(std::forward<T>(fun)) {}

    Ret operator()() { return fun(); }

private: 
    std::function<Ret()> fun;
};

// Function implementation for 1 or more arguments
template<typename Ret, typename Arg, typename ...Args> 
struct FunctionImpl<Ret(Arg, Args...)> {
    // Get the Nth sub function (function type after N partial applications)
    template<size_t N>
    using SubFunc = std::conditional_t<N == sizeof...(Args), Ret,
        decltype(sub<Ret, N, Args...>(std::make_index_sequence<sizeof...(Args) - N>{}))>;

    template<typename T> 
    FunctionImpl(T&& fun) 
        : m_Fun(std::forward<T>(fun)) {}

    // << operator for function composition
    template<typename ...Ts> 
    inline Function<Ret(Ts..., Args...)> operator<<(const Function<Arg(Ts...)>& f) const {
        return [&](Ts&&... as, Args&& ...args) { return m_Fun(f(std::forward<Ts>(as)...), std::forward<Args>(args)...); };
    }

    // << operator for partial application
    template<typename A, typename = std::enable_if_t<CompatibleType<Arg, A>>> 
    inline SubFunc<0> operator<<(A&& arg) const {
        return Apply(std::forward<A>(arg));
    }

    // () operator for partial application
    template<typename A, typename ...Tys, typename =
        std::enable_if_t<sizeof...(Tys) <= sizeof...(Args) && CompatibleType<Arg, A>>>
    inline SubFunc<sizeof...(Tys)> operator()(A&& arg, Tys&& ...tys) const {
        return Apply<Tys...>(std::forward<A>(arg), std::forward<Tys>(tys)...);
    }

    // Apply function, used for partial application
    template<typename A, typename ...Tys, typename = 
        std::enable_if_t<sizeof...(Tys) <= sizeof...(Args) && CompatibleType<Arg, A>>>
    inline SubFunc<sizeof...(Tys)> Apply(A&& arg, Tys&& ...tys) const {
        // If only 1 argument, this is the final application.
        if constexpr (sizeof...(Args) == 0)
            return m_Fun(std::forward<Arg>(arg));
        else {
            // Capture the current argument, then generate the sub-function and recursively generate the next one with the Tys
            struct { Arg v; } _cap{ std::forward<Arg>(arg) };
            auto _ret = [fun = this->m_Fun, cap = std::move(_cap)](Args&&...args) { return fun(cap.v, std::forward<Args>(args)...); };
            if constexpr (sizeof...(Tys) == 0) return _ret; // If no Tys exist, return the current version
            else return Function<Ret(Args...)>{ _ret }(std::forward<Tys>(tys)...); // recurse with Tys
        }
    }

private: // Stored function
    std::function<Ret(Arg, Args...)> m_Fun;
};

// Get the function implementation given the function type
template <typename Ret, typename... Args>
struct GetFunctionImpl<Ret(Args...)> {
    template<typename T> using IsCallable = std::_Is_invocable_r<Ret, T, Args...>;
    using type = typename FunctionImpl<Ret(Args...)>;
};

// Function call constructor deduction guide for lambdas
template <class _Fx>
Function(_Fx)->Function<typename std::_Deduce_signature<_Fx>::type>;

namespace std {
    // std::function constructor deduction guide to convert Function to std::function
    template<class _Fx>
    function(const _Fx&)->function<typename _Fx::FuncSignature>;
}