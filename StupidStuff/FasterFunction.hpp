#pragma once
#include <functional>
namespace fun {
    struct _FunctionStorageBase {
        // Used for testing
        static inline size_t refcount = 0;
        _FunctionStorageBase() { refcount++; m_RefCount++; }
        virtual ~_FunctionStorageBase() { refcount--; }

        inline _FunctionStorageBase* Clone() { m_RefCount++; return this; }
        virtual inline bool Lambda() const { return false; }

        size_t m_RefCount = 0;
    };

    // Binder without type info about what kind of functor it contains, only return and arg types.
    template<typename Return, typename ...Args>
    struct _FunctionStorageCaller : public _FunctionStorageBase {
        virtual Return Call(Args&&...) = 0;
    };

    // Full binder contains all the type info, which allows the Finalize method to cast 
    // all void pointers back to their original types, to then call the lambda/function pointer
    template<typename, typename> struct _TypedFunctionStorage;
    template<typename T, typename Return, typename ...Args>
    struct _TypedFunctionStorage<T, Return(Args...)> : public _FunctionStorageCaller<Return, Args...> {
        _TypedFunctionStorage(const T& fun)
            : function(fun) {}

        virtual Return Call(Args&&...args) override { return function(static_cast<Args&&>(args)...); };
        T function;
    };

    template<typename, typename> struct _MemberFunctionStorage;
    template<typename T, typename Return, typename ...Args>
    struct _MemberFunctionStorage<T, Return(Args...)> : public _FunctionStorageCaller<Return, Args...> {
        _MemberFunctionStorage(Return(T::*function)(Args...), T& obj)
            : obj(obj), function(function) {}

        virtual Return Call(Args&&...args) override { return (obj.*function)(static_cast<Args&&>(args)...); };
        T& obj;
        Return(T::*function)(Args...);
    };

    template<typename T>
    struct Function;

    // Main partial application Function class
    template<typename Return, typename ...Args>
    struct Function<Return(Args...)> {
        using FunType = Return(*)(Args...);

        Function() {}

        template<typename T>
        Function(Return(T::*a)(Args...), T& t) 
            : m_Storage(new _MemberFunctionStorage<T, Return(Args...)>{ a, t }), m_Type(false) {}
        
        // Capturing lambda constructor
        template<typename T, typename = typename std::_Deduce_signature<T>::type,
            typename = std::enable_if_t<sizeof(T) >= 2>>
        Function(const T& t)
            : m_Storage(new _TypedFunctionStorage<T, typename std::_Deduce_signature<T>::type>{ t }), m_Type(false) {}

        // Lambda constructor
        template<typename T, typename = std::enable_if_t<sizeof(T) == 1 && std::is_same_v<FunType, typename std::_Deduce_signature<T>::type*>>>
        Function(const T& t)
            : m_Functor((FunType)t) {}

        // Function pointer constructor
        Function(FunType fun)
            : m_Functor(fun) {}

        // Copy Constructor
        Function(const Function<Return(Args...)>& f) {
            m_Type = f.m_Type;
            if (f.m_Type)
                m_Functor = f.m_Functor;
            else
                m_Storage = f.m_Storage, m_Storage->m_RefCount++;
        }

        // Move constructor
        Function(Function<Return(Args...)>&& f) {
            m_Type = f.m_Type;
            if (f.m_Type)
                m_Functor = f.m_Functor, f.m_Functor = nullptr;
            else
                m_Storage = f.m_Storage, f.m_Storage = nullptr;
        }

        ~Function() {
            if (m_Type || m_Storage == nullptr) return;
            m_Storage->m_RefCount--;
            if (m_Storage->m_RefCount == 0)
                delete m_Storage, m_Storage = nullptr;
        }

        inline Return operator()(Args ...args) const {
           return m_Type ? m_Functor(static_cast<Args&&>(args)...) : m_Storage->Call(static_cast<Args&&>(args)...);
        }

    private:
        bool m_Type = true;
        union {
            Return(*m_Functor)(Args...);
            _FunctionStorageCaller<Return, Args...>* m_Storage = nullptr;
        };
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