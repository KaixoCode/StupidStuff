#pragma once
#include <iostream>
#include <functional>
#define _AsString(x) _AsString2(x)
#define _AsString2(x) #x
#define Assert(x) _Assert(x, " \033[31mFailed at line " _AsString(__LINE__)\
	" of file " __FILE__ "\n   Assertion: " #x " \033[37m")
#define AssertEquals(x, y) _Assert(x == y, " \033[31mFailed at line " _AsString(__LINE__)\
	" of file " __FILE__ "\n   Assertion: " #x " == " #y " \033[37m")
#define AssertNotEquals(x, y) _Assert(x != y, " \033[31mFailed at line " _AsString(__LINE__)\
	" of file " __FILE__ "\n   Assertion: " #x " != " #y " \033[37m")
#define AssertTrue(x) _Assert(x, " \033[31mFailed at line " _AsString(__LINE__) \
	" of file " __FILE__ "\n   Assertion: " #x " is true\033[37m")
#define AssertFalse(x) _Assert(!x, " \033[31mFailed at line " _AsString(__LINE__) \
	" of file " __FILE__ "\n   Assertion: " #x " is false\033[37m")
#define Test(x) std::pair<std::unordered_map<const char const*, std::function<void(void)>>::iterator, bool> \
	_Test##x = _Tests.emplace(#x, [this] { x(); }); void x()
#define For(x) for(int __i = 0; __i < x; __i++)
template<class Type>
struct TestBase {
	static inline void Run() {
		Type e;
		for (auto& i : e._Tests) if (i.second) {
			std::cout << "Running test " << i.first << " from " << typeid(Type).name() << std::endl;
			i.second(); // Run the test
			if (e._Passes == e._Total) std::cout << "\033[32m" << "Passed " << e._Passes
				<< "/" << e._Total << " asserts" << "\033[0m" << std::endl << std::endl;
			else std::cerr << "\033[31m" << "Failed " << e._Total - e._Passes << "/"
				<< e._Total << " asserts" << "\033[0m" << std::endl << std::endl;
			e._Passes = 0, e._Total = 0;
		}
	}
protected:
	bool _Assert(bool a, const char const* line) {
		if (a) _Passes++; else std::cout << line << std::endl; _Total++;
		return a;
	}
	std::unordered_map<const char const*, std::function<void(void)>> _Tests;
private:
	int _Passes = 0, _Total = 0;
};