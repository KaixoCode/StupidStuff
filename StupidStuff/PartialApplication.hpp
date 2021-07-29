#pragma once
#include <functional>
namespace faster {
    // Template pack wrapper, empty
    template<typename ...Tys>
    struct TPack {
        constexpr static bool isLast = true;

        using Remainder = void;
        using Type = void;
    };

    // Template pack wrapper
    template<typename T, typename ...Tys>
    struct TPack<T, Tys...> {
        constexpr static bool isLast = false;

        using Remainder = TPack<Tys...>;
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
        std::conditional_t<L1::isLast || L2::isLast, void, typename L1::Remainder>, 
        std::conditional_t<L1::isLast || L2::isLast, void, typename L2::Remainder>> {
        using Parent = CompareTypes<
            std::conditional_t<L1::isLast || L2::isLast, void, typename L1::Remainder>,
            std::conditional_t<L1::isLast || L2::isLast, void, typename L2::Remainder>>;
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
        virtual inline void Apply(void*, size_t) = 0;
        virtual inline Return Finalize() = 0;
        virtual inline _Binder<Return>* Copy() = 0;
        virtual inline size_t Size() const = 0;

    protected:
        std::vector<std::function<void(void)>> m_ToDelete;
        size_t m_RefCount = 0;
        bool m_Finalized = false;

        template<typename Arg>
        inline void* _ConvertToVoidP(Arg&& arg) {
            if constexpr (std::is_pointer_v<Arg>)
                return reinterpret_cast<void*>(arg);
            else if constexpr (std::is_reference_v<Arg>)
                if constexpr (std::is_const_v<std::remove_reference_t<Arg>>)
                    return reinterpret_cast<void*>(const_cast<std::remove_const_t<std::remove_reference_t<Arg>>*>(&arg));
                else
                    return reinterpret_cast<void*>(&arg);
            else {
                Arg* _ptr = new Arg(std::move(arg));
                auto _void = reinterpret_cast<void*>(_ptr);
                m_ToDelete.push_back([&, _ptr] { delete _ptr; });
                return _void;
            }
        }

        template<typename Arg>
        inline Arg _ConvertFromVoidP(void* arg) {
            if constexpr (std::is_pointer_v<Arg>)
                return reinterpret_cast<Arg>(arg);
            else if constexpr (std::is_reference_v<Arg>)
                return *reinterpret_cast<std::remove_reference_t<Arg>*>(arg);
            else
                return *reinterpret_cast<Arg*>(arg);
        }

        virtual ~_Binder() {
            for (auto& _t : m_ToDelete) _t();
        }

        template<typename T> friend class Function;
    };

    // Full binder contains all the type info, which allows the Finalize method to cast 
    // all void pointers back to their original types, to then call the function pointer
    template<typename Return, typename ...Args>
    struct _FullBinder : public _Binder<Return> {
        _FullBinder(Return(*fun)(Args...)) 
            : m_Fun(fun) {}

        inline void Apply(void* a, size_t index) override {
            m_Args[sizeof...(Args) - index] = a;
        };

        // Finalize binding and call the function.
        inline Return Finalize() override {
            this->m_Finalized = true;
            return _CallFun(m_IndexSeq);
        }

        // Deep copy the binder
        inline _Binder<Return>* Copy() override {
            _FullBinder<Return, Args...>* _binder = new _FullBinder<Return, Args...>{ m_Fun };
            memcpy(&_binder->m_Args[0], &m_Args[0], sizeof(void*) * sizeof...(Args));
            _binder->m_RefCount = 1;
            return _binder;
        }

        inline size_t Size() const { return sizeof...(Args); }

    private:
        static inline std::make_index_sequence<sizeof...(Args)> m_IndexSeq{};
        Return(*m_Fun)(Args...);
        void* m_Args[sizeof...(Args)]; // already binded arguments

        // Use pack expansion and an index sequence to cast the arguments back, and then call
        template<std::size_t... Is>
        inline Return _CallFun(std::index_sequence<Is...>) {
            return m_Fun(this->_ConvertFromVoidP<Args>(m_Args[Is])...);
        }

        template<typename T> friend class Function;
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
            : m_Binder(new _FullBinder<Return, Arg, Args...>{ (FunType)t }) { m_Binder->m_RefCount++; }

        Function(FunType fun) 
            : m_Binder(new _FullBinder<Return, Arg, Args...>{ fun }) { m_Binder->m_RefCount++; }

        Function(_Binder<Return>* m_Binder)
            : m_Binder(m_Binder) { m_Binder->m_RefCount++; }

        ~Function() { m_Binder->m_RefCount--; if (m_Binder->m_RefCount == 0) delete m_Binder; }

        template<typename ...Tys, typename = std::enable_if_t<CompareTypes<TPack<Arg, Args...>, TPack<Tys...>>::same>>
        inline _SubFunction<sizeof...(Tys) - 1, Return, Args...> operator()(Tys&& ...tys) const {
            // Edge case when calling with all parameters
            if constexpr (sizeof...(Tys) - 1 == sizeof...(Args))
                if (m_Binder->Size() == sizeof...(Tys))
                    return ((_FullBinder<Return, Tys...>*)m_Binder)->m_Fun(std::forward<Tys>(tys)...);
            // If it has been previously called, make a copy of the binder to make the new call unique.
            // Unless the call using this binder has been finalized
            if (m_Binder->m_Finalized)
                m_Called = false, m_Binder->m_Finalized = false;
            else if (m_Called) {
                m_Binder->m_RefCount--;
                _Binder<Return>* _cpy = m_Binder;
                // Copy, give the amount of remaining arguments we expect to deliver.
                m_Binder = m_Binder->Copy();
                if (_cpy->m_RefCount == 0)
                    delete _cpy; // make sure to delete when refcount reaches 0!
            }
            size_t _index = sizeof...(Args) + 1;
            (m_Binder->Apply(m_Binder->_ConvertToVoidP<Tys>(std::forward<Tys>(tys)), _index--), ...);
            m_Called = true;
            if constexpr (sizeof...(Tys) - 1 == sizeof...(Args))
                return m_Binder->Finalize();
            else {
                m_Called = true;
                return { m_Binder };
            }
        }

    private:
        mutable bool m_Called = false;
        mutable _Binder<Return>* m_Binder;
    };

    // Base case partial application Function class, no arguments
    template<typename Return>
    struct Function<Return()> {
        Function(Return(*fun)()) 
            : m_Binder(new _FullBinder<Return>{ fun }) {}

        Function(_Binder<Return>* m_Binder) 
            : m_Binder(m_Binder) {}

        ~Function() { delete m_Binder; }

        inline Return operator()() { return ((_FullBinder<Return>*)m_Binder)->m_Fun(); }

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