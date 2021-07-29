#pragma once
#include <functional>
namespace stupid_stuff
{
    template<typename> struct _GetFunctionImpl;
    template<typename> struct _FunctionImpl;

    // Partial application function definition
    template<typename Signature>
    struct Function : public _GetFunctionImpl<Signature>::type {
        using FuncSignature = Signature;

        template<typename Lambda, typename = _GetFunctionImpl<Signature>::IsCallable<Lambda>>
        constexpr Function(Lambda t) : _GetFunctionImpl<Signature>::type(t) {}
        constexpr Function(Signature* t) : _GetFunctionImpl<Signature>::type(t) {}
    };

    // Get a sub sequence from a template pack
    template<typename Ret, std::size_t N, typename... T, std::size_t... I>
    Function<Ret(std::tuple_element_t<N + I, std::tuple<T...>>...)> _SubSeq(std::index_sequence<I...>) {};

    // Check for compatible types, remove reference/const and check if same type.
    template<typename T1, typename T2>
    constexpr bool CompatibleType = (std::is_same_v<std::remove_const_t<std::remove_reference_t<T1>>,
        std::remove_const_t<std::remove_reference_t<T2>>> || std::is_constructible_v<std::remove_const_t<std::remove_reference_t<T1>>,
        std::remove_const_t<std::remove_reference_t<T2>>>);

    // Function implementation for no arguments
    template<typename Ret>
    struct _FunctionImpl<Ret()> {
        template<typename T>
        constexpr _FunctionImpl(T&& fun)
            : fun(std::forward<T>(fun)) {}

        Ret operator()() { return fun(); }

    private:
        std::function<Ret()> fun;
    };

    // Function implementation for 1 or more arguments
    template<typename Ret, typename Arg, typename ...Args>
    struct _FunctionImpl<Ret(Arg, Args...)> {
        // Get the Nth sub function (function type after N partial applications)
        template<size_t N>
        using _SubFunction = std::conditional_t < N == sizeof...(Args), Ret,
            decltype(_SubSeq<Ret, N, Args...>(std::make_index_sequence<sizeof...(Args) - N>{})) > ;

        template<typename T>
        constexpr _FunctionImpl(T&& fun)
            : m_Fun(std::forward<T>(fun)) {}

        // << operator for function composition
        template<typename ...Ts>
        inline Function<Ret(Ts..., Args...)> operator<<(const Function<Arg(Ts...)>& f) const {
            return [&](Ts&&... as, Args&& ...args) { return m_Fun(f(std::forward<Ts>(as)...), std::forward<Args>(args)...); };
        }

        // << operator for partial application
        template<typename A, typename = std::enable_if_t<CompatibleType<Arg, A>>>
        inline _SubFunction<0> operator<<(A&& arg) const {
            return Apply(std::forward<A>(arg));
        }

        // () operator for partial application
        template<typename A, typename ...Tys, typename =
            std::enable_if_t<sizeof...(Tys) <= sizeof...(Args) && CompatibleType<Arg, A>>>
            inline _SubFunction<sizeof...(Tys)> operator()(A&& arg, Tys&& ...tys) const {
            return Apply(std::forward<A>(arg), std::forward<Tys>(tys)...);
        }

        // Apply function, used for partial application
        template<typename A, typename ...Tys, typename =
            std::enable_if_t<sizeof...(Tys) <= sizeof...(Args) && CompatibleType<Arg, A>>>
            inline _SubFunction<sizeof...(Tys)> Apply(A&& arg, Tys&& ...tys) const {
            // If only 1 argument, this is the final application.
            if constexpr (sizeof...(Args) == 0)
                return m_Fun(std::forward<A>(arg));
            else {
                // Capture the current argument, then generate the sub-function and recursively generate the next one with the Tys
                static auto _makeCap = [](A&& arg) -> auto {
                    if constexpr (std::is_same_v<Arg, A>) { struct { Arg v; } _cap{ std::forward<A>(arg) }; return _cap; }
                    else { struct { Arg v; } _cap{ Arg{ std::forward<A>(arg) } }; return _cap; };
                };
                auto _cap = _makeCap(std::forward<A>(arg));
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
    struct _GetFunctionImpl<Ret(Args...)> {
        template<typename T> using IsCallable = std::_Is_invocable_r<Ret, T, Args...>;
        using type = typename _FunctionImpl<Ret(Args...)>;
    };

    // Function call constructor deduction guide for lambdas
    template <class _Fx>
    Function(_Fx)->Function<typename std::_Deduce_signature<_Fx>::type>;

#define Operator(name, op)                                                 \
    template<typename T>                                                   \
    auto _##name(T t1, T t2) -> decltype(t1 op t2) { return t1 op t2; }    \
    template<typename T> Function<decltype(_##name<T>)> name = _##name<T>; \

    Operator(Add, +);
    Operator(Sub, -);
    Operator(Mul, *);
    Operator(Div, / );
    Operator(Mod, %);
    Operator(LeftShift, << );
    Operator(RightShift, >> );
    Operator(LessThan, < );
    Operator(LessThanEqual, <= );
    Operator(GreaterThan, > );
    Operator(GreaterThanEqual, >= );
    Operator(Equal, == );
    Operator(NotEqual, != );
    Operator(BitwiseAnd, &);
    Operator(BitwiseXor, ^);
    Operator(BitwiseOr, | );
    Operator(And, &&);
    Operator(Or, || );
    Operator(Assign, =);
    Operator(AssignAdd, +=);
    Operator(AssignSub, -=);
    Operator(AssignMul, *=);
    Operator(AssignDiv, /=);
    Operator(AssignMod, %=);
    Operator(AssignLeftShift, <<=);
    Operator(AssignRightShift, >>=);
    Operator(AssignBitwiseAnd, &=);
    Operator(AssignBitwiseXor, ^=);
    Operator(AssignBitwiseOr, |=);

    template<typename T>
    auto _Neg(T t1) -> decltype(-t1) { return -t1; }
    template<typename T> Function<decltype(_Neg<T>)> Negate = _Neg<T>;

    template<typename T>
    auto _Inc(T t1) -> decltype(t1++) { return t1++; }
    template<typename T> Function<decltype(_Inc<T>)> Increment = _Inc<T>;

    template<typename T>
    auto _Dec(T t1) -> decltype(t1--) { return t1--; }
    template<typename T> Function<decltype(_Dec<T>)> Decrement = _Dec<T>;

    template<typename T>
    auto _BNt(T t1) -> decltype(~t1) { return ~t1; }
    template<typename T> Function<decltype(_BNt<T>)> BitwiseNot = _BNt<T>;

    template<typename T>
    auto _Not(T t1) -> decltype(!t1) { return !t1; }
    template<typename T> Function<decltype(_Not<T>)> Not = _Not<T>;

    template<typename T>
    auto _Adr(T t1) -> decltype(&t1) { return &t1; }
    template<typename T> Function<decltype(_Adr<T>)> AddressOf = _Adr<T>;

    template<typename T>
    auto _Ind(T t1) -> decltype(*t1) { return *t1; }
    template<typename T> Function<decltype(_Ind<T>)> Indirection = _Ind<T>;

    template<typename T>
    auto _Fold(const Function<T(T, T)>& f, T first, const std::vector<T>& v) {
        for (const auto& i : v) first = f(first, i); return first;
    }
    template<typename T> Function<decltype(_Fold<T>)> Fold = _Fold<T>;

    template<typename T>
    auto _Sum(const std::vector<T>& v) {
        return Fold<T> << Add<T> << 0 << v;
    }
    template<typename T>
    Function<T(const std::vector<T>&)> Sum = _Sum<T>;

}

namespace std {
    // std::function constructor deduction guide to convert Function to std::function
    template<class _Fx>
    function(const _Fx&)->function<typename _Fx::FuncSignature>;
}