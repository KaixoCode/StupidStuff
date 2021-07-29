#pragma once
#include <functional>
namespace faster {
    template<size_t N, typename... Ts> using NthTypeOf = typename std::tuple_element<N, std::tuple<Ts...>>::type;

    // Template pack wrapper, empty
    template<typename ...Tys>
    struct TPack {
        enum { isLast = true };

        using Remainder = void;
        using Type = void;
    };

    // Template pack wrapper
    template<typename T, typename ...Tys>
    struct TPack<T, Tys...> {
        enum { isLast = false };

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
    struct _CompatibleTPacks : public _CompatibleTPacks<
        std::conditional_t<L1::isLast || L2::isLast, void, typename L1::Remainder>, 
        std::conditional_t<L1::isLast || L2::isLast, void, typename L2::Remainder>> {
        using Parent = _CompatibleTPacks<
            std::conditional_t<L1::isLast || L2::isLast, void, typename L1::Remainder>,
            std::conditional_t<L1::isLast || L2::isLast, void, typename L2::Remainder>>;
        constexpr static bool same = (CompatibleType<L1::Type, L2::Type>
            || std::is_same_v<L1::Type, void> || std::is_same_v<L2::Type, void>
            ) && Parent::same;
    };

    // Base cases for the compare, if a void is encountered.
    template<> struct _CompatibleTPacks<void, void> { constexpr static bool same = true; };
    template<typename T> struct _CompatibleTPacks<T, void> { constexpr static bool same = true; };
    template<typename T> struct _CompatibleTPacks<void, T> { constexpr static bool same = true; };

    // Destructor class, used to delete any type properly
    struct Destructor { Destructor() { } virtual ~Destructor() { }; };
    template<typename Type>
    struct TypedDestructor : public Destructor {
        TypedDestructor(Type* t)
            : m_Ptr(t) {}

        ~TypedDestructor() override { delete m_Ptr; }
    private:
        Type* m_Ptr;
    };

    using dynamic = void*;

    // Function binder base class
    template<typename Return>
    struct _Binder {
        virtual inline void Apply(dynamic, size_t) = 0;
        virtual inline Return Finalize() = 0;
        virtual inline _Binder<Return>* Copy() = 0;
        virtual inline size_t Size() const = 0;

    protected:
        size_t m_RefCount = 0;
        bool m_Finalized = false;

        template<typename Ty, typename Arg>
        inline dynamic _ConvertToDynamic(Ty&& arg, Destructor** toDelete, size_t index) {
            if constexpr (std::is_reference_v<Ty> && std::is_reference_v<Arg> && std::is_const_v<std::remove_reference_t<Arg>> &&
                !std::is_same_v<std::remove_const_t<std::remove_reference_t<Arg>>, std::remove_const<std::remove_reference_t<Ty>>>) {
                using RealType = std::remove_const_t<std::remove_reference_t<Arg>>;
                RealType* _ptr = new RealType(arg);
                auto _void = reinterpret_cast<dynamic>(_ptr);
                if (toDelete[index] != nullptr)
                    delete toDelete[index];
                toDelete[index] = new TypedDestructor<RealType>{ _ptr };
                return _void;
            } else if constexpr (std::is_pointer_v<Ty> && (std::is_pointer_v<Arg> || std::is_reference_v<Arg>))
                return reinterpret_cast<dynamic>(const_cast<std::remove_const_t<std::remove_pointer_t<Ty>>*>(arg));
            else if constexpr (std::is_reference_v<Ty> && (std::is_pointer_v<Arg> || std::is_reference_v<Arg>))
                return reinterpret_cast<dynamic>(const_cast<std::remove_const_t<std::remove_reference_t<Ty>>*>(&arg));
            else if constexpr (sizeof(Arg) <= sizeof(dynamic) && std::is_trivially_copyable_v<Arg> && std::is_trivially_constructible_v<Arg>) {
                if constexpr (std::is_pointer_v<Ty>) {
                    if constexpr (std::is_same_v<std::remove_pointer_t<Ty>, Arg>) {
                        dynamic _ret = nullptr;
                        std::memcpy(&_ret, &arg, sizeof(Ty));
                        return _ret;
                    } else  {
                        dynamic _ret = nullptr;
                        Arg _arg = static_cast<Arg>(*arg);
                        std::memcpy(&_ret, &_arg, sizeof(Arg));
                        return _ret;
                    }
                } else if constexpr (std::is_reference_v<Ty>) {
                    if constexpr (std::is_same_v<std::remove_reference_t<Ty>, Arg>) {
                        dynamic _ret = nullptr;
                        std::memcpy(&_ret, &arg, sizeof(Ty));
                        return _ret;
                    }
                    else {
                        dynamic _ret = nullptr;
                        Arg _arg = static_cast<Arg>(arg);
                        std::memcpy(&_ret, &_arg, sizeof(Arg));
                        return _ret;
                    }
                } else {
                    if constexpr (std::is_same_v<Ty, Arg>) {
                        dynamic _ret = nullptr;
                        std::memcpy(&_ret, &arg, sizeof(Ty));
                        return _ret;
                    }
                    else {
                        dynamic _ret = nullptr;
                        Arg _arg = static_cast<Arg>(arg);
                        std::memcpy(&_ret, &_arg, sizeof(Arg));
                        return _ret;
                    }
                }
            } else if constexpr (std::is_reference_v<Arg> && std::is_const_v<std::remove_reference_t<Arg>>) {
                using RealType = std::remove_const_t<std::remove_reference_t<Arg>>;
                RealType* _ptr = new RealType(std::forward<Ty>(arg));
                auto _void = reinterpret_cast<dynamic>(_ptr);
                if (toDelete[index] != nullptr)
                    delete toDelete[index];
                toDelete[index] = new TypedDestructor<RealType>{ _ptr };
                return _void;
            } else {
                Arg* _ptr = new Arg(std::forward<Ty>(arg));
                auto _void = reinterpret_cast<dynamic>(_ptr);
                if (toDelete[index] != nullptr)
                    delete toDelete[index];
                toDelete[index] = new TypedDestructor<Arg>{ _ptr };
                return _void;
            }
        }

        template<typename Arg>
        inline Arg _ConvertFromDynamic(dynamic arg) {
            if constexpr (std::is_pointer_v<Arg>)
                return reinterpret_cast<Arg>(arg);
            else if constexpr (std::is_reference_v<Arg>)
                return *reinterpret_cast<std::remove_reference_t<Arg>*>(arg);
            else if constexpr (sizeof(Arg) <= sizeof(dynamic) && std::is_trivially_copyable_v<Arg> && std::is_trivially_constructible_v<Arg>) {
                Arg _ret;
                std::memcpy(&_ret, &arg, sizeof(Arg));
                return _ret;
            }
            else 
                return *reinterpret_cast<Arg*>(arg);
        }

        template<typename T> friend class Function;
    };

    // Full binder contains all the type info, which allows the Finalize method to cast 
    // all void pointers back to their original types, to then call the function pointer
    template<typename Return, typename ...Args>
    struct _FullBinder : public _Binder<Return> {
        _FullBinder(Return(*fun)(Args...)) 
            : m_Fun(fun) {}

        inline void Apply(dynamic a, size_t index) override {
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
            memcpy(&_binder->m_Args[0], &m_Args[0], sizeof(dynamic) * sizeof...(Args));
            _binder->m_RefCount = 1;
            return _binder;
        }

        inline size_t Size() const { return sizeof...(Args); }

    private:
        static inline std::make_index_sequence<sizeof...(Args)> m_IndexSeq{};
        Return(*m_Fun)(Args...);
        dynamic m_Args[sizeof...(Args)]; // already binded arguments

        // Use pack expansion and an index sequence to cast the arguments back, and then call
        template<std::size_t... Is>
        inline Return _CallFun(std::index_sequence<Is...>) {
            return m_Fun(this->_ConvertFromDynamic<Args>(m_Args[Is])...);
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
            : m_Binder(new _FullBinder<Return, Arg, Args...>{ (FunType)t }) { m_Binder->m_RefCount++; _InitDestructors(); }

        Function(FunType fun) 
            : m_Binder(new _FullBinder<Return, Arg, Args...>{ fun }) { m_Binder->m_RefCount++; _InitDestructors(); }

        Function(_Binder<Return>* m_Binder)
            : m_Binder(m_Binder) { m_Binder->m_RefCount++; _InitDestructors(); }

        ~Function() { 
            m_Binder->m_RefCount--;
            if (m_Binder->m_RefCount == 0) 
                delete m_Binder; 
            for (int i = 0; i < sizeof...(Args) + 1; i++)
                if (m_Destructors[i] != nullptr) delete m_Destructors[i];
        }

        template<typename ...Tys, typename = std::enable_if_t<_CompatibleTPacks<TPack<Arg, Args...>, TPack<Tys...>>::same>>
        inline _SubFunction<sizeof...(Tys) - 1, Return, Args...> operator()(Tys&& ...tys) {
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
            _ApplyBinder<0, Tys...>(std::forward<Tys>(tys)..., std::index_sequence_for<Tys...>{});
            m_Called = true;
            if constexpr (sizeof...(Tys) - 1 == sizeof...(Args))
                return m_Binder->Finalize();
            else {
                m_Called = true;
                return { m_Binder };
            }
        }
    private:
        bool m_Called = false;
        _Binder<Return>* m_Binder;
        Destructor* m_Destructors[sizeof...(Args) + 1];

        void _InitDestructors() {
            for (int i = 0; i < sizeof...(Args) + 1; i++) m_Destructors[i] = nullptr;
        }

        // Use pack expansion to call the binder for all given arguments
        template<std::size_t N, typename... Tys, std::size_t... Is>
        inline void _ApplyBinder(Tys&& ... tys, std::index_sequence<Is...>) {
            size_t _index = sizeof...(Args) + 2;
            ((_index--, m_Binder->Apply(m_Binder->_ConvertToDynamic<NthTypeOf<Is, Tys...>, NthTypeOf<Is, Arg, Args...>>(
                std::forward<NthTypeOf<Is, Tys...>>(tys), m_Destructors, _index - 1), _index)), ...);
        }
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