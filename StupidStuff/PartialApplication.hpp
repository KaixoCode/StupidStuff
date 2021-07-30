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

    // Check for compatible types, remove reference/const and check if same type.
    template<typename T1, typename T2>
    constexpr bool CompatibleType = (std::is_same_v<std::remove_const_t<std::remove_reference_t<T1>>,
        std::remove_const_t<std::remove_reference_t<T2>>> || std::is_constructible_v<std::remove_const_t<std::remove_reference_t<T1>>,
        std::remove_const_t<std::remove_reference_t<T2>>>);

    // Check for same types, remove reference/const and check if same type.
    template<typename T1, typename T2>
    constexpr bool SameType = std::is_same_v<T1, T2>;

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

    // Compare 2 TLists, to see if they contain compatible types up to the one with the least types.
    template<typename L1, typename L2>
    struct _SameTPacks : public _SameTPacks<
        std::conditional_t<L1::isLast || L2::isLast, void, typename L1::Remainder>,
        std::conditional_t<L1::isLast || L2::isLast, void, typename L2::Remainder>> {
        using Parent = _SameTPacks<
            std::conditional_t<L1::isLast || L2::isLast, void, typename L1::Remainder>,
            std::conditional_t<L1::isLast || L2::isLast, void, typename L2::Remainder>>;
        constexpr static bool same = (SameType<L1::Type, L2::Type>
            || std::is_same_v<L1::Type, void> || std::is_same_v<L2::Type, void>
            ) && Parent::same;
    };

    // Base cases for the compare, if a void is encountered.
    template<> struct _CompatibleTPacks<void, void> { constexpr static bool same = true; };
    template<typename T> struct _CompatibleTPacks<T, void> { constexpr static bool same = true; };
    template<typename T> struct _CompatibleTPacks<void, T> { constexpr static bool same = true; };

    template<> struct _SameTPacks<void, void> { constexpr static bool same = true; };
    template<typename T> struct _SameTPacks<T, void> { constexpr static bool same = true; };
    template<typename T> struct _SameTPacks<void, T> { constexpr static bool same = true; };

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
        _BinderBase() { refcount++; }
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

        /*
        
        -- if same: 
        Ty&&         std::is_rvalue_reference_v<Ty>
        Arg&&, const Arg&, Arg
                     
        Ty&          std::is_lvalue_reference_v<Ty>
        Arg&, const Arg&, Arg

        Ty
        Arg&&, const Arg&, Arg
 

        Ty*&&        std::is_rvalue_reference_v<Ty> && std::is_pointer_v<std::remove_reference_t<Ty>>
        Arg*&&, const Arg*&, Arg*

        Ty*&         std::is_lvalue_reference_v<Ty> && std::is_pointer_v<std::remove_reference_t<Ty>>
        Arg*&, const Arg*&, Arg*

        Ty*          std::is_pointer_v<Ty>
        Arg*, const Arg*&, Arg*

        Ty&&[N]      std::is_rvalue_reference_v<Ty> && std::is_array_v<std::remove_reference_t<Ty>>
        Ty&[N]       std::is_lvalue_reference_v<Ty> && std::is_array_v<std::remove_reference_t<Ty>>
        Ty[N]        std::is_array_v<Ty>

        Arg&&        std::is_rvalue_reference_v<Arg>
        Ty&&

        Arg&         std::is_lvalue_reference_v<Arg>
        Ty&

        const Arg&   std::is_lvalue_reference_v<Arg> && std::is_const_v<std::remove_reference_t<Arg>>
        Ty&&, Ty&, const Ty&

        Arg
        Ty&&, Ty&, const Ty&

        Arg*&&       std::is_rvalue_reference_v<Arg> && std::is_pointer_v<std::remove_reference_t<Arg>>
        Ty*&&, Ty&&[N]

        Arg*&        std::is_lvalue_reference_v<Arg> && std::is_pointer_v<std::remove_reference_t<Arg>>
        const Arg*&  std::is_lvalue_reference_v<Arg> && std::is_pointer_v<std::remove_reference_t<Arg>> && std::is_const_v<std::remove_reference_t<Arg>>
        Arg*         std::is_pointer_v<Arg>

        Arg&&[N]     std::is_rvalue_reference_v<Arg> && std::is_array_v<std::remove_reference_t<Arg>>
        Arg&[N]      std::is_lvalue_reference_v<Arg> && std::is_array_v<std::remove_reference_t<Arg>>
        Arg[N]       std::is_array_v<Arg>

        */

        // Convert Ty to dynamic (void*), put resulting void* into destructor table at index if it was heap allocated.
        template<typename Ty, typename Arg>
        inline dynamic _ConvertToDynamic(Ty&& arg, Destructor** destructorTable, size_t index) {
            constexpr bool _ar_fptr = std::is_function_v<std::remove_pointer_t<Arg>>;
            constexpr bool _ty_cref = std::is_reference_v<Ty> && std::is_const_v<std::remove_reference_t<Arg>>;
            constexpr bool _ar_cref = std::is_reference_v<Arg> && std::is_const_v<std::remove_reference_t<Arg>>;
            constexpr bool _ty_ref = std::is_reference_v<Ty> && !_ty_cref;
            constexpr bool _ty_r2p = std::is_reference_v<Ty> && std::is_pointer_v<std::remove_reference_t<Ty>> && !_ty_cref;
            constexpr bool _ar_ref = std::is_reference_v<Arg> && !_ar_cref;
            constexpr bool _ty_ptr = std::is_pointer_v<Ty>;
            constexpr bool _ar_ptr = std::is_pointer_v<Arg>;
            constexpr bool _ty_arr = std::is_array_v<std::remove_pointer_t<decltype(&arg)>>;
            constexpr bool _ty_aref = std::is_array_v<std::remove_reference_t<Arg>>;
            constexpr bool _ty_cns = std::is_const_v< std::remove_reference_t<std::remove_pointer_t<Ty>>>;
            constexpr bool _same = std::is_same_v<FullDecay<Arg>, FullDecay<Ty>>;
            constexpr bool _small = sizeof(Arg) <= sizeof(dynamic) && std::is_trivially_copyable_v<Arg> && std::is_trivially_constructible_v<Arg>;
            constexpr bool _constr = !_same && std::is_constructible_v<Arg, Ty>;

            // If ty is a ptr, convert to void*
            if constexpr (_ar_fptr && _same || _ty_r2p && _same)
                return (dynamic)arg;
            else if constexpr (!_constr && _ty_arr)
                return (dynamic)&arg;
            else if constexpr (!_constr && _same && _ty_ptr && (_ar_ref || _ar_ptr || _ar_cref))
                return reinterpret_cast<dynamic>(const_cast<FullDecay<Ty>*>(arg));
            // If ty is a ref, make it a ptr, remove any const and convert to void*
            else if constexpr (!_constr && _ty_cns && _same && (_ty_ref || _ty_cref) && (_ar_ref || _ar_ptr || _ar_cref))
                return reinterpret_cast<dynamic>(const_cast<FullDecay<Ty>*>(&arg));
            else if constexpr (_ty_aref || !_constr && _same && (_ty_ref || _ty_cref) && (_ar_ref || _ar_ptr || _ar_cref))
                return reinterpret_cast<dynamic>(&arg);
            // If ty is neither, but it fits in a void*, copy memory to a void*
            else if constexpr (_small) {
                if constexpr (_same) {
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
                        return _ret;
                    } else {
                        // Not a class: cast (usually primitive types)
                        Arg _arg = (Arg)arg;
                        std::memcpy(&_ret, &_arg, sizeof(Arg));
                        return _ret;
                    }
                }
            // In all other cases, allocate object on heap and save ptr to void*
            } else {
                using RealType = std::remove_const_t<std::remove_reference_t<Arg>>;
                RealType* _ptr;
                if constexpr (!_same) _ptr = new RealType(arg);
                else _ptr = new RealType(std::forward<Ty>(arg));
                auto _void = reinterpret_cast<dynamic>(_ptr);
                if (destructorTable[index] != nullptr)
                {
                    (destructorTable[index])->refcount -= 1;
                    if (destructorTable[index]->refcount == 0)
                        delete destructorTable[index];
                }
                destructorTable[index] = new TypedDestructor<RealType>{ _ptr };
                return _void;
            }
        }

        // Convert void* back to Arg
        template<typename Arg>
        inline Arg _ConvertFromDynamic(dynamic arg) {
            // Small and trivially copyable/constructable.
            constexpr bool _small = sizeof(Arg) <= sizeof(dynamic) && std::is_trivially_copyable_v<Arg> && std::is_trivially_constructible_v<Arg>;
            constexpr bool _ar_fptr = std::is_function_v<Arg>;

            // If function or pointer just reinterpret void*
            if constexpr (std::is_pointer_v<Arg> || _ar_fptr)
                return reinterpret_cast<Arg>(arg);
            // References are converted to pointers when stored, so reinterpret to Arg*, then back to &Arg
            else if constexpr (std::is_reference_v<Arg>)
                return *reinterpret_cast<std::remove_reference_t<Arg>*>(arg);
            // If small, copy memory into an Arg object.
            else if constexpr (_small) {
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
    struct _CallBinder : public _Binder<Return>
    {
        virtual Return Call(Args&&...) = 0;

        ~_CallBinder() {
            for (int i = 0; i < sizeof...(Args); i++) if (m_Destructors[i] != nullptr) {
                m_Destructors[i]->refcount -= 1;
                if (m_Destructors[i]->refcount == 0)
                    delete m_Destructors[i]; // delete items with no references.
            }
        }

        dynamic m_Args[sizeof...(Args) + 1]; // already binded arguments
        Destructor* m_Destructors[sizeof...(Args) + 1]{ (Destructor*)nullInit<Args>::value... };

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
            return _CallFun(m_IndexSeq);
        }

        // Deep copy the binder
        inline _Binder<Return>* Copy() override {
            _FullBinder<T, Return(Args...)>* _binder = new _FullBinder<T, Return(Args...)>{ m_Fun };
            memcpy(&_binder->m_Args[0], &this->m_Args[0], sizeof(dynamic) * sizeof...(Args));
            memcpy(&_binder->m_Destructors[0], &this->m_Destructors[0], sizeof(dynamic) * sizeof...(Args));
            for (int i = 0; i < sizeof...(Args); i++) if (this->m_Destructors[i] != nullptr) this->m_Destructors[i]->refcount++;
            _binder->m_RefCount = 1;
            return _binder;
        }

        virtual inline bool Lambda() { return !std::is_same_v<T, Return(*)(Args...)>; }
        
        virtual Return Call(Args&&...args) override {
            return m_Fun(std::forward<Args>(args)...);
        };

    private:
        static inline std::make_index_sequence<sizeof...(Args)> m_IndexSeq{};
        T m_Fun;

        // Use pack expansion and an index sequence to cast the arguments back, and then call
        template<std::size_t... Is>
        inline Return _CallFun(std::index_sequence<Is...>&) {
            return m_Fun(this->_ConvertFromDynamic<Args>(this->m_Args[sizeof...(Args)-Is-1])...);
        }

        template<typename T> friend class Function;
    };

    template<typename Ret, typename ...Args>
    struct LambdaType
    {
        Ret operator()(Args...) { return Ret{}; };
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
        template<size_t N> static inline std::make_index_sequence<N> m_IndexSeq;
        using FunType = Return(*)(Arg, Args...);

        // Capturing lambda constructor
        template<typename T, typename = typename std::_Deduce_signature<T>::type, 
            typename = std::enable_if_t<sizeof(T) >= 2>>
        Function(const T& t)
            : m_Binder(new _FullBinder<T, typename std::_Deduce_signature<T>::type>{ t }) { m_Binder->m_RefCount++; }

        // Lambda constructor
        template<typename T, typename = std::enable_if_t<sizeof(T) == 1 && std::is_same_v<FunType, typename std::_Deduce_signature<T>::type*>>>
        Function(const T& t) 
            : m_Binder(new _FullBinder<FunType, Return(Arg, Args...)>{ (FunType)t }) { m_Binder->m_RefCount++; }

        // Function pointer constructor
        Function(FunType fun) 
            : m_Binder(new _FullBinder<FunType, Return(Arg, Args...)>{ fun }) { m_Binder->m_RefCount++; }

        // After-call constructor
        Function(_Binder<Return>* m_Binder)
            : m_Binder(m_Binder) { m_Binder->m_RefCount++; }

        // Copy Constructor
        Function(const Function<Return(Arg, Args...)>& f) 
            : m_Binder(f.m_Binder) { m_Binder->m_RefCount++; }

        // Move constructor
        Function(Function<Return(Arg, Args...)>&& f) 
            : m_Binder(f.m_Binder) { m_Binder->m_RefCount++; }

        ~Function() { 
            m_Binder->m_RefCount--;
            if (m_Binder->m_RefCount == 0) 
                delete m_Binder, m_Binder = nullptr; 
        }

        template<typename ...Tys, typename = std::enable_if_t<_CompatibleTPacks<TPack<Arg, Args...>, TPack<Tys...>>::same>>
        inline _SubFunction<sizeof...(Tys) - 1, Return, Args...> operator()(Tys&& ...tys) const {
            // Optimization for direct call with all parameters
            if constexpr (sizeof...(Tys) == sizeof...(Args) + 1 && _CompatibleTPacks<TPack<Tys...>, TPack<Arg, Args...>>::same) {
                if (sizeof...(Args) + 1 == m_Binder->Size())
                    if (!m_Binder->Lambda())
                        return ((_FullBinder<FunType, Return(Arg, Args...)>*)m_Binder)->m_Fun(std::forward<Tys>(tys)...);
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
            _ApplyBinder<0, Tys...>(std::forward<Tys>(tys)..., m_IndexSeq<sizeof...(Tys)>);
            m_Called = true;
            if constexpr (sizeof...(Tys) - 1 == sizeof...(Args))
                return m_Binder->Finalize();
            else
                return { m_Binder };
        }

        // Use pack expansion to call the binder for all given arguments
        template<std::size_t N, typename... Tys, std::size_t... Is>
        inline void _ApplyBinder(Tys&& ... tys, std::index_sequence<Is...>&) const {
            size_t _index = sizeof...(Args) + 1;
            ((_index--, ((_CallBinder<Return, Arg, Args...>*)m_Binder)->m_Args[_index] = m_Binder->_ConvertToDynamic<NthTypeOf<Is, Tys...>, NthTypeOf<Is, Arg, Args...>>(
                std::forward<NthTypeOf<Is, Tys...>>(tys), m_Binder->Destructors(), _index)), ...);
        }

        mutable bool m_Called = false;
        mutable _Binder<Return>* m_Binder;
    };

    // Base case partial application Function class, no arguments
    template<typename Return>
    struct Function<Return()> {
        using FunType = Return(*)();

        // Capturing lambda constructor
        template<typename T, typename = typename std::_Deduce_signature<T>::type,
            typename = std::enable_if_t<sizeof(T) >= 2>>
        Function(const T& t)
            : m_Binder(new _FullBinder<T, typename std::_Deduce_signature<T>::type>{ t }) { m_Binder->m_RefCount++; }

        // Lambda constructor
        template<typename T, typename = std::enable_if_t<std::is_same_v<FunType, typename std::_Deduce_signature<T>::type*>>>
        Function(T t)
            : m_Binder(new _FullBinder<FunType, Return()>{ (FunType)t }) { m_Binder->m_RefCount++; }

        // Function pointer constructor
        Function(FunType fun)
            : m_Binder(new _FullBinder<FunType, Return()>{ fun }) { m_Binder->m_RefCount++;}

        // After-call constructor
        Function(_Binder<Return>* m_Binder) 
            : m_Binder(m_Binder) { m_Binder->m_RefCount++; }

        // Copy Constructor
        Function(const Function<Return()>& f)
            : m_Binder(f.m_Binder) { m_Binder->m_RefCount++; }

        // Move constructor
        Function(Function<Return()>&& f)
            : m_Binder(f.m_Binder) { m_Binder->m_RefCount++; }

        ~Function() {
            m_Binder->m_RefCount--;
            if (m_Binder->m_RefCount == 0)
                delete m_Binder, m_Binder = nullptr;
        }

        inline Return operator()() {
            if (!m_Binder->Lambda())
                return ((_FullBinder<FunType, Return()>*)m_Binder)->m_Fun();
            else
                return ((_CallBinder<Return>*)m_Binder)->Call();
        }

        _Binder<Return>* m_Binder;
    };

    // Function constructor deduction guide for function pointers
    template <class Ret, class ...Args>
    Function(Ret(Args...))->Function<Ret(Args...)>;

    // Function constructor deduction guide for lambdas
    template <class _Fx>
    Function(_Fx)->Function<typename std::_Deduce_signature<_Fx>::type>;
}