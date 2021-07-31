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

        virtual inline bool Lambda() const { return !std::is_same_v<T, Return(*)(Args...)>; }
        virtual Return Call(Args&&...args) override { return function(static_cast<Args&&>(args)...); };
        T function;
    };

    template<typename T>
    struct Function;

    // Main partial application Function class
    template<typename Return, typename ...Args>
    struct Function<Return(Args...)> {
        using FunType = Return(*)(Args...);
        using BinderType = _TypedFunctionStorage<FunType, Return(Args...)>*;

        // Capturing lambda constructor
        template<typename T, typename = typename std::_Deduce_signature<T>::type,
            typename = std::enable_if_t<sizeof(T) >= 2>>
        Function(const T& t)
            : m_Binder(new _TypedFunctionStorage<T, typename std::_Deduce_signature<T>::type>{ t }) {}

        // Lambda constructor
        template<typename T, typename = std::enable_if_t<sizeof(T) == 1 && std::is_same_v<FunType, typename std::_Deduce_signature<T>::type*>>>
        Function(const T& t)
            : m_Binder(new _TypedFunctionStorage<FunType, Return(Args...)>{ (FunType)t }) {}

        // Function pointer constructor
        Function(FunType fun)
            : m_Binder(new _TypedFunctionStorage<FunType, Return(Args...)>{ fun }) {}

        // Copy Constructor
        Function(const Function<Return(Args...)>& f)
            : m_Binder(f.m_Binder->Clone()) {}

        // Move constructor
        Function(Function<Return(Args...)>&& f)
            : m_Binder(f.m_Binder->Clone()) {}

        ~Function() {
            m_Binder->m_RefCount--;
            if (m_Binder->m_RefCount == 0)
                delete m_Binder, m_Binder = nullptr;
        }

        inline Return operator()(Args ...args) const {
            if (!m_Binder->Lambda())
                return ((_TypedFunctionStorage<FunType, Return(Args...)>*)m_Binder)->function(static_cast<Args&&>(args)...);
            else
                return m_Binder->Call(static_cast<Args&&>(args)...);
        }

    private:
        _FunctionStorageCaller<Return, Args...>* m_Binder;
    };

    // Function constructor deduction guide for function pointers
    template <class Ret, class ...Args>
    Function(Ret(Args...))->Function<Ret(Args...)>;

    // Function constructor deduction guide for lambdas
    template <class _Fx>
    Function(_Fx)->Function<typename std::_Deduce_signature<_Fx>::type>;
}