#pragma once
#include <functional>
namespace faster {
    template<size_t N, typename... Ts> using NthTypeOf = typename std::tuple_element<N, std::tuple<Ts...>>::type;

    template<typename T>
    using FullDecay = typename std::remove_pointer_t<std::decay_t<T>>;

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

    template<typename Args>
    struct nullInit
    {
        constexpr static void* value = nullptr;
    };

    template<typename Arg>
    struct TC
    {
        using This = TC<Arg>;
        constexpr static inline bool vala = std::is_array_v<Arg>;                                                                /* Arg[N]            */
        constexpr static inline bool valaa = std::is_array_v<std::remove_reference_t<Arg>>;                                      /* Arg(&&/&)?[N]     */
        constexpr static inline bool lvala = std::is_lvalue_reference_v<Arg> && std::is_array_v<std::remove_reference_t<Arg>>;   /* Arg&[N]           */
        constexpr static inline bool rvala = std::is_rvalue_reference_v<Arg> && std::is_array_v<std::remove_reference_t<Arg>>;   /* Arg&&[N]          */
        constexpr static inline bool valar = std::is_reference_v<Arg> && std::is_array_v<std::remove_reference_t<Arg>>;          /* Arg&&/&[N]        */
        constexpr static inline bool clvala = std::is_lvalue_reference_v<Arg> && std::is_array_v<std::remove_reference_t<Arg>> 
            && std::is_const_v<std::remove_reference_t<Arg>>;                                                                    /* const Arg&[N]     */
        constexpr static inline bool crvala = std::is_rvalue_reference_v<Arg> && std::is_array_v<std::remove_reference_t<Arg>> 
            && std::is_const_v<std::remove_reference_t<Arg>>;                                                                    /* const Arg&&[N]    */

        constexpr static inline bool val = std::is_same_v<Arg, FullDecay<Arg>>;                                                  /* Arg               */
        constexpr static inline bool valra = std::is_reference_v<Arg>;                                                           /* Arg&/&&([N]!)     */
        constexpr static inline bool valr = std::is_reference_v<Arg> && !This::valaa;                                            /* Arg&/&&([N]!)     */
        constexpr static inline bool lval = std::is_lvalue_reference_v<Arg> && !This::valaa;                                     /* Arg&              */
        constexpr static inline bool nclval = std::is_lvalue_reference_v<Arg> && !std::is_const_v<std::remove_reference_t<Arg>> 
            && !This::valaa;                                                                                                     /* !const Arg&       */
        constexpr static inline bool rval = std::is_rvalue_reference_v<Arg> && !This::valaa;                                     /* Arg&&             */
        constexpr static inline bool clval = std::is_lvalue_reference_v<Arg> && std::is_const_v<std::remove_reference_t<Arg>> 
            && !This::valaa;                                                                                                     /* const Arg&        */
        constexpr static inline bool crval = std::is_rvalue_reference_v<Arg> && std::is_const_v<std::remove_reference_t<Arg>> 
            && !This::valaa;                                                                                                     /* const Arg&&       */
        constexpr static inline bool valp = std::is_pointer_v<Arg>;                                                              /* Arg*              */
        constexpr static inline bool valpr = std::is_pointer_v<std::remove_reference_t<Arg>> && std::is_reference_v<Arg>;        /* Arg*(&&/&)        */
        constexpr static inline bool valpa = std::is_pointer_v<std::remove_reference_t<Arg>>;                                    /* Arg*(&&/&)?       */

        constexpr static inline bool fun = std::is_function_v<FullDecay<Arg>>;
        constexpr static inline bool funa = std::is_function_v<FullDecay<Arg>> && !This::valr && !This::valp;
        constexpr static inline bool funp = std::is_function_v<FullDecay<Arg>> && This::valp;
        constexpr static inline bool funr = std::is_function_v<FullDecay<Arg>> && This::valr;
        constexpr static inline bool funlr = std::is_function_v<FullDecay<Arg>> && This::lval;
        constexpr static inline bool funrr = std::is_function_v<FullDecay<Arg>> && This::rval;
    };

    template<typename in, typename out>
    constexpr bool Same = std::is_same_v<FullDecay<in>, FullDecay<out>>;

    template<typename in, typename out>
    constexpr bool ValidDuo = (Same<in, out> && (
        (TC<out>::val && (TC<in>::valr || TC<in>::val))             // Arg           : Arg&/&& || Arg
        || (TC<out>::nclval && (TC<in>::nclval))                    // Arg&          : const! Arg& 
        || (TC<out>::rval && (TC<in>::rval || TC<in>::val))         // Arg&&         : Arg&& || Arg
        || (TC<out>::clval && (TC<in>::valr || TC<in>::val))        // const Arg&    : Arg&/&&([N]!) || Arg
        || (TC<out>::valp && (TC<in>::valp || TC<in>::valaa))       // Arg*          : Arg* || Arg(&/&&)?[N]
        || (TC<out>::vala && (TC<in>::valpa || TC<in>::valaa))      // Arg[N]        : Arg*(&/&&)? || Arg(&/&&)?[N]
        || (TC<out>::lvala && (TC<in>::lvala))                      // Arg&[N]       : Arg(&)[N]
        || (TC<out>::rvala && (TC<in>::rvala))                      // Arg&&[N]      : Arg&&[N]
        || (TC<out>::clvala && (TC<in>::valaa))                     // const Arg&[N] : Arg(&&/&)?[N]
        || (TC<out>::funa && (TC<in>::fun))                         // Arg(Arg)      : Arg(*/&/&&)?(Arg)
        || (TC<out>::funp && (TC<in>::fun))                         // Arg(*)(Arg)   : Arg(*/&/&&)?(Arg)
        || (TC<out>::funlr && (TC<in>::funlr || TC<in>::funa))      // Arg(&)(Arg)   : Arg(&/&&)?(Arg)
        || (TC<out>::funrr && (TC<in>::funlr || TC<in>::funa))      // Arg(&&)(Arg)  : Arg(&/&&)?(Arg)
        )) || (std::is_constructible_v<out, in>);
    
    // Compare 2 TLists, to see if they contain compatible types up to the one with the least types.
    template<typename L1, typename L2>
    struct _CompatibleTPacks : public _CompatibleTPacks<
        std::conditional_t<L1::isLast || L2::isLast, void, typename L1::Remainder>,
        std::conditional_t<L1::isLast || L2::isLast, void, typename L2::Remainder>> {
        using Parent = _CompatibleTPacks<
            std::conditional_t<L1::isLast || L2::isLast, void, typename L1::Remainder>,
            std::conditional_t<L1::isLast || L2::isLast, void, typename L2::Remainder>>;
        constexpr static bool same = (ValidDuo<L1::Type, L2::Type>
            || std::is_same_v<L1::Type, void> || std::is_same_v<L2::Type, void>
            ) && Parent::same;
    };    
    
    // Base cases for the compare, if a void is encountered.
    template<> struct _CompatibleTPacks<void, void> { constexpr static bool same = true; };
    template<typename T> struct _CompatibleTPacks<T, void> { constexpr static bool same = true; };
    template<typename T> struct _CompatibleTPacks<void, T> { constexpr static bool same = true; };

    // Destructor class, used to delete any type properly
    struct Destructor {
        Destructor() { } 
        virtual ~Destructor() { };
        size_t refcount = 1;
    };

    template<typename Type>
    struct TypedDestructor : public Destructor {
        TypedDestructor(Type* t)
            : m_Ptr(t) {}

        ~TypedDestructor() override { delete m_Ptr; }
    private:
        Type* m_Ptr;
    };

    using dynamic = void*;

    struct _BinderBase
    {
        static inline size_t refcount = 0;
        _BinderBase() { refcount++; m_RefCount++; }
        virtual ~_BinderBase() { refcount--; }

        virtual inline size_t Size() const = 0;
        virtual inline bool Lambda() { return false; }

        size_t m_RefCount = 0;
        bool m_Finalized = false;
    };

    // Function binder class
    template<typename Return>
    struct _Binder : public _BinderBase {
        virtual inline Return Finalize() = 0;
        virtual inline _Binder<Return>* Copy() = 0;
        virtual inline Destructor** Destructors() = 0;

    protected:
        // Convert Ty to dynamic (void*), put resulting void* into destructor table at index if it was heap allocated.
        template<typename Ty, typename Arg>
        inline dynamic _ConvertToDynamic(Ty&& arg, size_t index) {
            if constexpr ((TC<Arg>::valra || TC<Arg>::vala) && Same<Ty, Arg>)
                return (dynamic)&arg;
            else if constexpr (TC<Arg>::valpa && Same<Ty, Arg>)
                return (dynamic)arg;
            else if constexpr (sizeof(Arg) <= sizeof(dynamic) && 
                std::is_trivially_copyable_v<Arg> && std::is_trivially_constructible_v<Arg>)
                if constexpr (Same<Ty, Arg>) {
                    // If it is the same and small, just copy memory.
                    dynamic _ret = nullptr;
                    std::memcpy(&_ret, &arg, sizeof(Ty));
                    return _ret;
                } else {
                    // If it isn't same, and it is a class, construct new Arg with Ty as param.
                    dynamic _ret = nullptr;
                    if constexpr (std::is_class_v<Arg>) {
                        Arg _arg = Arg{ arg };
                        std::memcpy(&_ret, &_arg, sizeof(Arg));
                    } else {
                        // Not a class: cast (usually primitive types)
                        Arg _arg = (Arg)arg;
                        std::memcpy(&_ret, &_arg, sizeof(Arg));
                    }
                    return _ret;
                }
            else {
                using RealType = std::remove_const_t<std::remove_reference_t<Arg>>;
                RealType* _ptr;
                if constexpr (!Same<Ty, Arg>) _ptr = new RealType(arg);
                else _ptr = new RealType(std::forward<Ty>(arg));
                auto _void = reinterpret_cast<dynamic>(_ptr);
                auto& _destructor = Destructors()[index];
                if (_destructor != nullptr)
                {
                    (_destructor)->refcount -= 1;
                    if ((_destructor)->refcount == 0)
                        delete (_destructor);
                }
                _destructor = new TypedDestructor<RealType>{ _ptr };
                return _void;
            }
        }

        // Convert void* back to Arg
        template<typename Arg>
        inline Arg _ConvertFromDynamic(dynamic arg) {
            // If function or pointer just reinterpret void*
            if constexpr (std::is_pointer_v<Arg> || std::is_function_v<Arg>)
                return reinterpret_cast<Arg>(arg);
            // References are converted to pointers when stored, so reinterpret to Arg*, then back to &Arg
            else if constexpr (std::is_reference_v<Arg>)
                return *reinterpret_cast<std::remove_reference_t<Arg>*>(arg);
            // If small, copy memory into an Arg object.
            else if constexpr (sizeof(Arg) <= sizeof(dynamic) && 
                std::is_trivially_copyable_v<Arg> && std::is_trivially_constructible_v<Arg>) {
                Arg _ret;
                std::memcpy(&_ret, &arg, sizeof(Arg));
                return _ret;
            }
            // Heap allocated object, just cast to Arg*, then to Arg to copy.
            else 
                return *reinterpret_cast<Arg*>(arg);
        }

        template<typename T> friend class Function;
    };
    
    // Binder without type info about what kind of functor it contains, only return and arg types.
    template<typename Return, typename ...Args>
    struct _CallBinder : public _Binder<Return> {
        ~_CallBinder() {
            for (int i = 0; i < sizeof...(Args); i++) if (m_Destructors[i] != nullptr) {
                m_Destructors[i]->refcount -= 1;
                if (m_Destructors[i]->refcount == 0)
                    delete m_Destructors[i]; // delete items with no references.
            }
        }

        dynamic m_Args[sizeof...(Args) == 0 ? 1 : sizeof...(Args)]{ (Destructor*)nullInit<Args>::value... }; // already binded arguments
        Destructor* m_Destructors[sizeof...(Args) == 0 ? 1 : sizeof...(Args)]{ (Destructor*)nullInit<Args>::value... };

        inline size_t Size() const { return sizeof...(Args); }
        inline Destructor** Destructors() override { return m_Destructors; };
    };

    // Full binder contains all the type info, which allows the Finalize method to cast 
    // all void pointers back to their original types, to then call the lambda/function pointer
    template<typename, typename> struct _FullBinder;
    template<typename T, typename Return, typename ...Args>
    struct _FullBinder<T, Return(Args...)> : public _CallBinder<Return, Args...> {
        _FullBinder(const T& fun)
            : m_Fun(fun) {}

        // Finalize binding and call the function.
        inline Return Finalize() override {
            this->m_Finalized = true;
            dynamic* _ptr = this->m_Args;
            return m_Fun(this->_ConvertFromDynamic<Args>(*_ptr++)...);
        }

        // Deep copy the binder
        inline _Binder<Return>* Copy() override {
            _FullBinder<T, Return(Args...)>* _binder = new _FullBinder<T, Return(Args...)>{ m_Fun };
            memcpy(&_binder->m_Args[0], &this->m_Args[0], sizeof(dynamic) * (sizeof...(Args) == 0 ? 1 : sizeof...(Args)) * 2);
            for (int i = 0; i < sizeof...(Args); i++) if (this->m_Destructors[i] != nullptr) this->m_Destructors[i]->refcount++;
            return _binder;
        }

        virtual inline bool Lambda() { return !std::is_same_v<T, Return(*)(Args...)>; }

    private:
        T m_Fun;

        template<typename T> friend class Function;
    };

    // Member binder is the same as a Full binder, but stores a member function pointer together with
    // the reference to the object it should call it on.
    template<typename, typename> struct _MemberBinder;
    template<typename T, typename Return, typename ...Args>
    struct _MemberBinder<T, Return(Args...)> : public _CallBinder<Return, Args...> {
        _MemberBinder(Return(T::*function)(Args...), T& obj)
            : m_Function(function), m_Obj(obj) {}

        // Finalize binding and call the function.
        inline Return Finalize() override {
            this->m_Finalized = true;
            dynamic* _ptr = this->m_Args;
            return (m_Obj.*m_Function)(this->_ConvertFromDynamic<Args>(*_ptr++)...);
        }

        // Deep copy the binder
        inline _Binder<Return>* Copy() override {
            _MemberBinder<T, Return(Args...)>* _binder = new _MemberBinder<T, Return(Args...)>{ m_Function, m_Obj };
            memcpy(&_binder->m_Args[0], &this->m_Args[0], sizeof(dynamic) * (sizeof...(Args) == 0 ? 1 : sizeof...(Args)) * 2);
            for (int i = 0; i < sizeof...(Args); i++) if (this->m_Destructors[i] != nullptr) this->m_Destructors[i]->refcount++;
            return _binder;
        }

        virtual inline bool Lambda() { return true; }

    private:
        T& m_Obj;
        Return(T::* m_Function)(Args...);
        template<typename T> friend class Function;
    };

    template<typename T>
    struct Function;

    // Get the Function object with the N last arguments
    template<typename Ret, std::size_t N, typename... T, std::size_t... I>
    Function<Ret(std::tuple_element_t<N + I, std::tuple<T...>>...)> _SubSeq(std::index_sequence<I...>) {};
    template<size_t N, typename Return, typename ...Args>
    using _SubFunction = std::conditional_t < N == sizeof...(Args), Return,
        decltype(_SubSeq<Return, N, Args...>(std::make_index_sequence<sizeof...(Args) - N>{})) > ;

    // Main partial application Function class
    template<typename Return, typename ...Args>
    struct Function<Return(Args...)> {
        template<size_t N> static inline std::make_index_sequence<N> m_IndexSeq;
        using FunType = Return(*)(Args...);

        Function() {};

        // Member function constructor
        template<typename T>
        Function(Return(T::* a)(Args...), T& t)
            : m_Binder(new _MemberBinder<T, Return(Args...)>{ a, t }) {}

        // Capturing lambda constructor
        template<typename T, typename = typename std::_Deduce_signature<T>::type, 
            typename = std::enable_if_t<sizeof(T) >= 2>>
        Function(const T& t)
            : m_Binder(new _FullBinder<T, typename std::_Deduce_signature<T>::type>{ t }) { }

        // Lambda constructor
        template<typename T, typename = std::enable_if_t<sizeof(T) == 1 && 
            std::is_same_v<FunType, typename std::_Deduce_signature<T>::type*>>>
        Function(const T& t) 
            : m_Binder(new _FullBinder<FunType, Return(Args...)>{ (FunType)t }) { }

        // Function pointer constructor
        Function(FunType fun) 
            : m_Binder(new _FullBinder<FunType, Return(Args...)>{ fun }) { }
        
        // Capturing lambda constructor
        template<typename T, typename = typename std::_Deduce_signature<T>::type,
            typename = std::enable_if_t<sizeof(T) >= 2>>
        auto operator =(const T& t) {
            Clean();
            m_Binder = new _FullBinder<T, typename std::_Deduce_signature<T>::type>{ t };
            return *this;
        }

        // Lambda constructor
        template<typename T, typename = std::enable_if_t<sizeof(T) == 1 && 
            std::is_same_v<FunType, typename std::_Deduce_signature<T>::type*>>>
        auto operator =(const T& t) {
            Clean();
            m_Binder = new _FullBinder<FunType, Return(Args...)>{ (FunType)t };
            return *this;
        }

        // Function pointer constructor
        auto operator=(FunType fun) {
            Clean();
            m_Binder = new _FullBinder<FunType, Return(Args...)>{ fun };
            return *this;
        }

        // After-call constructor
        Function(_Binder<Return>* m_Binder)
            : m_Binder(m_Binder) { m_Binder->m_RefCount++; }

        // Copy Constructor
        Function(const Function<Return(Args...)>& f) 
            : m_Binder(f.m_Binder) { m_Binder->m_RefCount++; }

        // Move constructor
        Function(Function<Return(Args...)>&& f) 
            : m_Binder(f.m_Binder) { m_Binder->m_RefCount++; }

        // Copy Constructor
        auto operator=(const Function<Return(Args...)>& f) {
            m_Binder = f.m_Binder;
            m_Binder->m_RefCount++;
            return *this;
        }

        // Move constructor
        auto operator=(Function<Return(Args...)>&& f) {
            m_Binder = f.m_Binder;
            m_Binder->m_RefCount++;
        }

        ~Function() { 
            Clean();
        }

        template<typename ...Tys, typename = std::enable_if_t<_CompatibleTPacks<TPack<Tys...>, TPack<Args...>>::same>>
        inline _SubFunction<sizeof...(Tys), Return, Args...> operator()(Tys&& ...tys) const {
            if constexpr (sizeof...(Tys) == sizeof...(Args)) 
                // Optimization for direct call with all parameters
                if (sizeof...(Args) == m_Binder->Size() && !m_Binder->Lambda())
                    return ((_FullBinder<FunType, Return(Tys...)>*)m_Binder)->m_Fun(static_cast<Tys&&>(tys)...);                    
                // If not all parameters, quickly apply without overhead of _ApplyBinder call and finalize.
                else {
                    size_t Is = 0;
                    ((((_CallBinder<Return, Args...>*)m_Binder)->m_Args[sizeof...(Args) - Is - 1] =
                        m_Binder->_ConvertToDynamic<Tys, Args>(
                            static_cast<Tys&&>(tys), sizeof...(Args) - Is - 1), Is++), ...);
                    return m_Binder->Finalize();
                }

            // If it has been previously called, make a copy of the binder to make the new call unique.
            // Unless the call using this binder has been finalized
            if (m_Binder->m_Finalized)
                m_Called = false, m_Binder->m_Finalized = false;
            else if (m_Called) {
                m_Binder->m_RefCount--;
                _Binder<Return>* _cpy = m_Binder;
                m_Binder = m_Binder->Copy();
                if (_cpy->m_RefCount == 0)
                    delete _cpy; // make sure to delete when refcount reaches 0!
            }
            _ApplyBinder<0, Tys...>(static_cast<Tys&&>(tys)..., m_IndexSeq<sizeof...(Tys)>);
            m_Called = true;
            if constexpr (sizeof...(Tys) == sizeof...(Args))
                return m_Binder->Finalize();
            else
                return { m_Binder };
        }

        // Use pack expansion to call the binder for all given arguments
        template<std::size_t N, typename... Tys, std::size_t... Is>
        inline void _ApplyBinder(Tys&& ... tys, std::index_sequence<Is...>&) const {
            ((((_CallBinder<Return, Args...>*)m_Binder)->m_Args[sizeof...(Args) - Is - 1] =
                m_Binder->_ConvertToDynamic<NthTypeOf<Is, Tys...>, NthTypeOf<Is, Args...>>(
                    static_cast<NthTypeOf<Is, Tys...>&&>(tys), sizeof...(Args) - Is - 1)), ...);
        }

        void Clean() {
            if (!m_Binder)
                return;
            m_Binder->m_RefCount--;
            if (m_Binder->m_RefCount == 0)
                delete m_Binder, m_Binder = nullptr;
        }

        mutable bool m_Called = false;
        mutable _Binder<Return>* m_Binder = nullptr;
    };

    // Function constructor deduction guide for function pointers
    template <class Ret, class ...Args>
    Function(Ret(Args...))->Function<Ret(Args...)>;

    // Function constructor deduction guide for member functions
    template <class Ret, class T, class ...Args>
    Function(Ret(T::* a)(Args...), T&)->Function<Ret(Args...)>;

    // Function constructor deduction guide for lambdas
    template <class _Fx>
    Function(_Fx)->Function<typename std::_Deduce_signature<_Fx>::type>;
}