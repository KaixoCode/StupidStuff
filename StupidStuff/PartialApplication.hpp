#pragma once
#include <functional>
#include <any>

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
        using _SubFunction = std::conditional_t<N == sizeof...(Args), Ret,
            decltype(_SubSeq<Ret, N, Args...>(std::make_index_sequence<sizeof...(Args) - N>{}))>;

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
    Operator(Div, /);
    Operator(Mod, %);
    Operator(LeftShift, <<);
    Operator(RightShift, >>);
    Operator(LessThan, <);
    Operator(LessThanEqual, <=);
    Operator(GreaterThan, >);
    Operator(GreaterThanEqual, >=);
    Operator(Equal, ==);
    Operator(NotEqual, !=);
    Operator(BitwiseAnd, &);
    Operator(BitwiseXor, ^);
    Operator(BitwiseOr, |);
    Operator(And, &&);
    Operator(Or, ||);
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
 

namespace faster {
    // Template list wrapper, empty
    template<typename ...Tys>
    struct TList {
        constexpr static bool last = true;

        using Remainder = void;
        using Type = void;
    };

    // Template list wrapper
    template<typename T, typename ...Tys>
    struct TList<T, Tys...> {
        constexpr static bool last = false;

        using Remainder = TList<Tys...>;
        using Type = T;
    };

    // Check for compatible types, remove reference/const and check if same type.
    template<typename T1, typename T2>
    constexpr bool CompatibleType = (std::is_same_v<std::remove_const_t<std::remove_reference_t<T1>>,
        std::remove_const_t<std::remove_reference_t<T2>>> || std::is_constructible_v<std::remove_const_t<std::remove_reference_t<T1>>,
        std::remove_const_t<std::remove_reference_t<T2>>>);

    // Compare 2 TLists, to see if they contain compatible types up to the one with the least types.
    template<typename L1, typename L2>
    struct CompareTypes : public CompareTypes<
        std::conditional_t<L1::last || L2::last, void, typename L1::Remainder>, 
        std::conditional_t<L1::last || L2::last, void, typename L2::Remainder>> {
        using Parent = CompareTypes<
            std::conditional_t<L1::last || L2::last, void, typename L1::Remainder>,
            std::conditional_t<L1::last || L2::last, void, typename L2::Remainder>>;
        constexpr static bool same = (CompatibleType<L1::Type, L2::Type>
            || std::is_same_v<L1::Type, void> || std::is_same_v<L2::Type, void>
            ) && Parent::same;
    };

    // Base cases for the compare, if a void is encountered.
    template<> struct CompareTypes<void, void> { constexpr static bool same = true; };
    template<typename T> struct CompareTypes<T, void> { constexpr static bool same = true; };
    template<typename T> struct CompareTypes<void, T> { constexpr static bool same = true; };

    // Function binder base class
    template<typename Return>
    struct _Binder {
        struct mem { const void* ptr; size_t size; };
        std::vector<std::function<void(void)>> todelete;
        size_t ref = 0;
        bool finalized = false;

        template<typename Arg>
        inline const void* ConvertToVoidP(Arg&& arg) {
            if constexpr (std::is_pointer_v<Arg>)
                return reinterpret_cast<const void*>(arg);
            else if constexpr (std::is_reference_v<Arg>)
                return reinterpret_cast<const void*>(&arg);
            else
            {
                Arg* _ptr = new Arg(std::move(arg));
                auto _void = reinterpret_cast<const void*>(_ptr);
                todelete.push_back([&, _ptr] { delete _ptr; });
                return _void;
            }
        }

        template<typename Arg>
        inline Arg ConvertFromVoidP(const void* carg) {
            void* _arg = const_cast<void*>(carg);
            if constexpr (std::is_pointer_v<Arg>)
                return reinterpret_cast<Arg>(_arg);
            else if constexpr (std::is_reference_v<Arg>)
                return *reinterpret_cast<std::remove_reference_t<Arg>*>(_arg);
            else {
                return *reinterpret_cast<Arg*>(_arg);
            }
        }

        virtual ~_Binder() {
            for (auto& _t : todelete) _t();
        }

        virtual void Apply(const void* a) = 0;
        virtual Return Finalize() = 0;
        virtual _Binder<Return>* Copy(size_t) = 0;
        virtual void Revert(size_t) = 0;
    };

    // Full binder contains all the type info, which allows the Finalize method to cast 
    // all void pointers back to their original types, to then call the function pointer
    template<typename Return, typename ...Args>
    struct _FullBinder : public _Binder<Return> {
        _FullBinder(Return(*fun)(Args...)) 
            : m_Fun(fun) { m_Args.reserve(sizeof...(Args)); }

        inline void Apply(const void* a) override { m_Args.push_back(a); };

        // Finalize binding and call the function.
        inline Return Finalize() override {
            this->finalized = true;
            return CallFun(std::make_index_sequence<sizeof...(Args)>{});
        }

        // Use pack expansion and an index sequence to cast the arguments back, and then call
        template<std::size_t... Is>
        inline Return CallFun(std::index_sequence<Is...>) {
            return m_Fun(this->ConvertFromVoidP<Args>(m_Args[Is])...);
        }

        // Deep copy the binder
        _Binder<Return>* Copy(size_t size) override {
            _FullBinder<Return, Args...>* _binder = new _FullBinder<Return, Args...>{ m_Fun };
            _binder->m_Args = m_Args;
            _binder->ref = 1;
            _binder->Revert(size);
            return _binder;
        }

        // Revert arguments to match expected amount of still expected arguments
        void Revert(size_t size) {
            while (m_Args.size() > sizeof...(Args) - size)
                m_Args.pop_back();
        }

    private:
        Return(*m_Fun)(Args...);
        std::vector<const void*> m_Args; // already binded arguments
    };

    template<typename T>
    struct Function;

    // Get the Function object with the N last arguments
    template<typename Ret, std::size_t N, typename... T, std::size_t... I>
    Function<Ret(std::tuple_element_t<N + I, std::tuple<T...>>...)> _SubSeq(std::index_sequence<I...>) {};
    template<size_t N, typename Return, typename ... Args>
    using _SubFunction = std::conditional_t < N == sizeof...(Args), Return,
        decltype(_SubSeq<Return, N, Args...>(std::make_index_sequence<sizeof...(Args) - N>{})) > ;

    // Main partial application Function class
    template<typename Return, typename Arg, typename ...Args>
    struct Function<Return(Arg, Args...)> {
        using FunType = Return(*)(Arg, Args...);

        template<typename T>
        Function(T t) 
            : m_Binder(new _FullBinder<Return, Arg, Args...>{ (FunType)t }) { m_Binder->ref++; }

        Function(FunType fun) 
            : m_Binder(new _FullBinder<Return, Arg, Args...>{ fun }) { m_Binder->ref++; }

        Function(_Binder<Return>* m_Binder)
            : m_Binder(m_Binder) { m_Binder->ref++; }

        ~Function() { m_Binder->ref--; if (m_Binder->ref == 0) delete m_Binder; }

        template<typename ...Tys, typename = std::enable_if_t<CompareTypes<TList<Tys...>, TList<Args...>>::same>>
        inline _SubFunction<sizeof...(Tys), Return, Args...> operator()(Arg&& arg, Tys&& ...tys) {
            return Apply<Tys...>(std::forward<Arg>(arg), std::forward<Tys>(tys)...);
        }

        template<typename ...Tys, typename = std::enable_if_t<CompareTypes<TList<Tys...>, TList<Args...>>::same>>
        inline _SubFunction<sizeof...(Tys), Return, Args...> Apply(Arg&& arg, Tys&& ...tys) {
            // If it has been previously called, make a copy of the binder to make the new call unique.
            // Unless the call using this binder has been finalized
            if (m_Binder->finalized) {
                m_Called = false, m_Binder->finalized = false;
                m_Binder->Revert(sizeof...(Args) + 1);
            }
            if (m_Called) {
                m_Binder->ref--;
                _Binder<Return>* _cpy = m_Binder;
                // Copy, give the amount of remaining arguments we expect to deliver.
                m_Binder = m_Binder->Copy(sizeof...(Args) + 1);
                if (_cpy->ref == 0)
                    delete _cpy; // make sure to delete when refcount reaches 0!
            }
            m_Binder->Apply(m_Binder->ConvertToVoidP<Arg>(std::forward<Arg>(arg)));
            (m_Binder->Apply(m_Binder->ConvertToVoidP<Tys>(std::forward<Tys>(tys))), ...);
            m_Called = true;
            if constexpr (sizeof...(Tys) == sizeof...(Args))
                return m_Binder->Finalize();
            else {
                m_Called = true;
                return { m_Binder };
            }
        }

    private:
        bool m_Called = false;
        _Binder<Return>* m_Binder;
    };

    // Base case partial application Function class, no arguments
    template<typename Return>
    struct Function<Return()> {
        Function(Return(*fun)()) 
            : m_Binder(new _FullBinder<Return>{ fun }) {}

        Function(_Binder<Return>* m_Binder) 
            : m_Binder(m_Binder) {}

        ~Function() { delete m_Binder; }

        inline Return operator()() { return Apply(); }
        inline Return Apply() { return m_Binder->Finalize(); }

    private:
        _Binder<Return>* m_Binder;
    };

    // Function constructor deduction guide for function pointers
    template <class Ret, class ...Args>
    Function(Ret(Args...))->Function<Ret(Args...)>;

    // Function constructor deduction guide for lambdas
    template <class _Fx>
    Function(_Fx)->Function<typename std::_Deduce_signature<_Fx>::type>;
}