#pragma once
#include "TestBase.hpp"
#include "PartialApplication.hpp"
#include <time.h>
#include "Thing.hpp"

namespace faster
{
	class PartialApplicationTests : public TestBase<PartialApplicationTests> {

		template<typename T>
		void TestPrimitive()
		{
			Function fun = [](const T* a, const T& b, T* c, T& d,
				T e, T f, T g, T h, T i, T j, T k, T l, T m, T n, T o, T p,
				const T& q, const T& r, const T& s, const T& t, const T& u, const T& v,
				const T& w, const T& x, const T& y, const T& z, const T& _, const T& $) -> T
			{ return *a + b + *c + d + e + f + g + h + i + j + k + l + m + n +
				o + p + q + r + s + t + u + v + w + x + y + z + _ + $; };

			auto random = []() -> T {
				size_t range = (size_t)std::numeric_limits<T>::max() - (size_t)std::numeric_limits<T>::min() + 1;
				if (range == 0) range = 1;
				T num = std::rand() % range + std::numeric_limits<T>::min();
				return num;
			};

			For(1000) {
				T a = random();
				T b = random();
				T c = random();
				T d = random();

				bool e = (bool)random();
				char f = random();
				unsigned char g = random();
				short h = random();
				unsigned short i = random();
				int j = random();
				unsigned int k = random();
				long long l = random();
				unsigned long long m = random();
				float n = random();
				double o = random();
				long double p = random();

				auto call1 = fun(&a);
				auto call2 = call1(b);
				auto call3 = call2(&c);
				auto call4 = call3(d);
				{
					auto call5 = call4(e);
					auto call6 = call5(f);
					auto call7 = call6(g);
					auto call8 = call7(h);
					auto call9 = call8(i);
					auto call10 = call9(j);
					auto call11 = call10(k);
					auto call12 = call11(l);
					auto call13 = call12(m);
					auto call14 = call13(n);
					auto call15 = call14(o);
					auto call16 = call15(p);
					auto call17 = call16(e);
					auto call18 = call17(f);
					auto call19 = call18(g);
					auto call20 = call19(h);
					auto call21 = call20(i);
					auto call22 = call21(j);
					auto call23 = call22(k);
					auto call24 = call23(l);
					auto call25 = call24(m);
					auto call26 = call25(n);
					auto call27 = call26(o);
					T res = call27(p);
					Assert(res == (T)((T)a + (T)b + (T)c + (T)d + 2 * ((T)e + (T)f + (T)g + (T)h + (T)i + (T)j + (T)k + (T)l + (T)m + (T)n + (T)o + (T)p)));
					T res2 = fun(&a, b, &c, d,
						e, f, g, h, i, j, k, l, m, n, o, p,
						e, f, g, h, i, j, k, l, m, n, o, p);
					Assert(res2 == (T)((T)a + (T)b + (T)c + (T)d + 2 * ((T)e + (T)f + (T)g + (T)h + (T)i + (T)j + (T)k + (T)l + (T)m + (T)n + (T)o + (T)p)));
				}
				{
					auto call5 = call4(e + 0);
					auto call6 = call5(f + 0);
					auto call7 = call6(g + 0);
					auto call8 = call7(h + 0);
					auto call9 = call8(i + 0);
					auto call10 = call9(j + 0);
					auto call11 = call10(k + 0);
					auto call12 = call11(l + 0);
					auto call13 = call12(m + 0);
					auto call14 = call13(n + 0);
					auto call15 = call14(o + 0);
					auto call16 = call15(p + 0);
					auto call17 = call16(e + 0);
					auto call18 = call17(f + 0);
					auto call19 = call18(g + 0);
					auto call20 = call19(h + 0);
					auto call21 = call20(i + 0);
					auto call22 = call21(j + 0);
					auto call23 = call22(k + 0);
					auto call24 = call23(l + 0);
					auto call25 = call24(m + 0);
					auto call26 = call25(n + 0);
					auto call27 = call26(o + 0);
					volatile auto res = call27(p + 0);
					Assert(res == (T)((T)a + (T)b + (T)c + (T)d + 2 * ((T)e + (T)f + (T)g + (T)h + (T)i + (T)j + (T)k + (T)l + (T)m + (T)n + (T)o + (T)p)));
					auto res2 = fun(&a, b, &c, d,
						e + 0, f + 0, g + 0, h + 0, i + 0, j + 0, k + 0, l + 0, m + 0, n + 0, o + 0, p + 0,
						e + 0, f + 0, g + 0, h + 0, i + 0, j + 0, k + 0, l + 0, m + 0, n + 0, o + 0, p + 0);
					Assert(res2 == (T)((T)a + (T)b + (T)c + (T)d + 2 * ((T)e + (T)f + (T)g + (T)h + (T)i + (T)j + (T)k + (T)l + (T)m + (T)n + (T)o + (T)p)));
				}
			}
		}

		Test(PrimitiveTypes) {
			std::srand(time(nullptr));
			_BinderBase::refcount = 0;
			TestPrimitive<bool>();
			TestPrimitive<char>();
			TestPrimitive<signed char>();
			TestPrimitive<unsigned char>();
			TestPrimitive<short>();
			TestPrimitive<signed short>();
			TestPrimitive<short int>();
			TestPrimitive<signed short int>();
			TestPrimitive<unsigned short>();
			TestPrimitive<unsigned short int>();
			TestPrimitive<int>();
			TestPrimitive<signed>();
			TestPrimitive<signed int>();
			TestPrimitive<unsigned>();
			TestPrimitive<unsigned int>();
			TestPrimitive<long>();
			TestPrimitive<long int>();
			TestPrimitive<signed long>();
			TestPrimitive<signed long int>();
			TestPrimitive<unsigned long>();
			TestPrimitive<unsigned long int>();
			TestPrimitive<long long>();
			TestPrimitive<long long int>();
			TestPrimitive<signed long long>();
			TestPrimitive<signed long long int>();
			TestPrimitive<unsigned long long>();
			TestPrimitive<unsigned long long int>();
			TestPrimitive<float>();
			TestPrimitive<double>();
			TestPrimitive<long double>();
			Assert(_BinderBase::refcount == 0); // no binders left
		}

		Test(SmallObject) {
			std::srand(time(nullptr));
			Thing::refcount = 0;
			Thing::constrcount = 0;
			Thing::copycount = 0;
			Thing::movecount = 0;
			_BinderBase::refcount = 0;

			int n = 100000;
			{
				Function fun = [](const Thing& a, const Thing* b, Thing& c, Thing* d, Thing e, Thing f, Thing g) {
					return a.v + b->v + c.v + d->v + e.v + f.v + g.v;
				};

				auto random = []() -> int {
					size_t range = (size_t)std::numeric_limits<int>::max() - (size_t)std::numeric_limits<int>::min() + 1;
					if (range == 0) range = 1;
					int num = std::rand() % range + std::numeric_limits<int>::min();
					return num;
				};

				For(n) {
					volatile int a = random();
					Thing b{ random() };
					Thing c{ random() };
					Thing d{ random() };
					Thing e{ random() };
					int f = random();
					double g = random();
					auto call1 = fun(Thing{ a });
					auto call2 = call1(&b);
					auto call3 = call2(c);
					int res2 = fun(Thing{ a }, &b, c, &d, e, f, g);
					Assert(res2 == (int)(a + b.v + c.v + d.v + e.v + f + (int)g));
					auto call4 = call3(&d);
					auto callagain1 = fun(Thing{ a }, &b, c, &d, e, f);
					auto call5 = call4(e);
					auto call6 = call5(f);
					volatile int res = call6(g);
					Assert(res == (int)(a + b.v + c.v + d.v + e.v + f + (int)g));
					auto res3 = callagain1(g);
					(int)res3;
					Assert(res3 == (int)(a + b.v + c.v + d.v + e.v + f + (int)g));
				}
			}
			Assert(Thing::refcount == 0);
			Assert(Thing::constrcount == n * 13);
			Assert(Thing::copycount == n * 9); 
			Assert(Thing::movecount == 0); // It shouldn't move any
			Assert(_BinderBase::refcount == 0); // No binders left
		}

		Test(MultipleInvocations) {
			std::srand(time(nullptr));
			Thing::refcount = 0;
			Thing::constrcount = 0;
			Thing::movecount = 0;
			_BinderBase::refcount = 0;

			int n = 1000;
			int m = 100;
			{
				Function fun = [](const Thing& a, Thing b, Thing c) { return a.v + b.v + c.v; };

				auto random = []() -> int {
					size_t range = (size_t)std::numeric_limits<int>::max() - (size_t)std::numeric_limits<int>::min() + 1;
					if (range == 0) range = 1;
					int num = std::rand() % range + std::numeric_limits<int>::min();
					return num;
				};

				For(n) {
					Function addToI = fun(__i);
					int ti = __i;
					For(m)
					{
						Function addToI2 = addToI(__i);
						int add = random();
						int res = addToI2(add);
						Assert(res == ti + __i + add);

						int add2 = random();
						int res2 = addToI2(add2);
						Assert(res2 == ti + __i + add2);

						int add3 = random();
						int res3 = addToI2(add3);
						Assert(res3 == ti + __i + add3);
					}
				}
			}
			Assert(Thing::refcount == 0); 
			Assert(Thing::constrcount == n + n * m * 4);
			Assert(Thing::copycount == 6 * n * m);
			Assert(Thing::movecount == 0); // It shouldn't move any
			Assert(_BinderBase::refcount == 0); // No binders left
		}

		Test(StringsAndArrays)
		{
			_BinderBase::refcount = 0;
			{ // Normal string
				Function fun = [](const char* c) -> std::string { return c; };
				const char test[] = "Helloworld";
				std::string res = fun(test);
				Assert(res == test);
			}
			{ // literal to std::string
				Function fun = [](std::string c) -> std::string { return c; };
				const char test[] = "Helloworld";
				std::string res = fun(test);
				Assert(res == test);
			}
			{ // std::string to std::string
				Function fun = [](std::string c) -> std::string { return c; };
				std::string test = "Helloworld";
				std::string res = fun(test);
				Assert(res == test);
			}
			{ // std::string to std::string&
				Function fun = [](std::string& c) -> std::string { return c; };
				std::string test = "Helloworld";
				std::string res = fun(test);
				Assert(res == test);
			}
			{ // literal to const std::string&
				Function fun = [](const std::string& c) -> std::string { return c; };
				const char test[] = "Helloworld";
				std::string res = fun(test);
				Assert(res == test);
			}
			{ // std::string to const std::string&
				Function fun = [](const std::string& c) -> std::string { return c; };
				std::string test = "Helloworld";
				std::string res = fun(test);
				Assert(res == test);
			}
			{ // array
				Function fun = [](int* c, size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1]);
			}
			{ // array
				Function fun = [](const int* c, size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1]);
			}
			{ // array
				Function fun = [](const int* c, size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1]);
			}
			{ // array passed as pointer
				Function fun = [](int* c, size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1]);
			}
			{ // array passed as pointer
				Function fun = [](const int* c, size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1]);
			}
			{ // array passed as pointer
				Function fun = [](const int* c, size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1]);
			}
			{ // array with size
				Function fun = [](int c[5], size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1]);
			}
			{ // array with size
				Function fun = [](const int c[5], size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1]);
			}
			{ // array with size passed as pointer
				Function fun = [](int c[5], size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1]);
			}
			{ // array with size passed as pointer
				Function fun = [](const int c[5], size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1]);
			}
			{ // array reference with size
				Function fun = [](int(&c)[5], size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1]);
			}
			{ // array reference with size
				Function fun = [](const int(&c)[5], size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun((const int(&)[5])arr, size);
				Assert(res == arr[size - 1]);
			}
			{ // array reference passed as pointer
				Function fun = [](int* (&c), size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1]);
			}
			{ // array reference passed as pointer
				Function fun = [](const int* (&c), size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				const int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1]);
			}
			{ // Normal string
				Function fun = [](const char* c) -> std::string { return c; };
				const char test[] = "Helloworld";
				std::string res = fun(test);
				Assert(res == test);
			}
			{ // literal to std::string
				Function fun = [](std::string c) -> std::string { return c; };
				const char test[] = "Helloworld";
				std::string res = fun(test);
				Assert(res == test);
			}
			{ // std::string to std::string
				Function fun = [](std::string c) -> std::string { return c; };
				std::string test = "Helloworld";
				std::string res = fun(test);
				Assert(res == test);
			}
			{ // std::string to std::string&
				Function fun = [](std::string& c) -> std::string { return c; };
				std::string test = "Helloworld";
				std::string res = fun(test);
				Assert(res == test);
			}
			{ // literal to const std::string&
				Function fun = [](const std::string& c) -> std::string { return c; };
				const char test[] = "Helloworld";
				std::string res = fun(test);
				Assert(res == test);
			}
			{ // std::string to const std::string&
				Function fun = [](const std::string& c) -> std::string { return c; };
				std::string test = "Helloworld";
				std::string res = fun(test);
				Assert(res == test);
			}
			{ // array
				Function fun = [](int* c, size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1]);
			}
			{ // array
				Function fun = [](const int* c, size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1]);
			}
			{ // array
				Function fun = [](const int* c, size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1]);
			}
			{ // array passed as pointer
				Function fun = [](int* c, size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1]);
			}
			{ // array passed as pointer
				Function fun = [](const int* c, size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1]);
			}
			{ // array passed as pointer
				Function fun = [](const int* c, size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1]);
			}
			{ // array with size
				Function fun = [](int c[5], size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1]);
			}
			{ // array with size
				Function fun = [](const int c[5], size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1]);
			}
			{ // array with size passed as pointer
				Function fun = [](int(c)[5], size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1]);
			}
			{ // array with size passed as pointer
				Function fun = [](const int c[5], size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1]);
			}
			{ // array reference with size
				Function fun = [](int(&c)[5], size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1]);
			}
			{ // array reference with size
				Function fun = [](const int(&c)[5], size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun((const int(&)[5])arr, size);
				Assert(res == arr[size - 1]);
			}
			{ // array reference passed as pointer
				Function fun = [](int* (&c), size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1]);
			}
			{ // array reference passed as pointer
				Function fun = [](const int* (&c), size_t size) -> int { return c[size - 1]; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				const int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1]);
			}
			std::string add = "apple";
			{ // Normal string
				Function fun = [&](const char* c) -> std::string { return c + add; };
				const char test[] = "Helloworld";
				std::string res = fun(test);
				Assert(res == test + add);
			}
			{ // literal to std::string
				Function fun = [&](std::string c) -> std::string { return c + add; };
				const char test[] = "Helloworld";
				std::string res = fun(test);
				Assert(res == test + add);
			}
			{ // std::string to std::string
				Function fun = [&](std::string c) -> std::string { return c + add; };
				std::string test = "Helloworld";
				std::string res = fun(test);
				Assert(res == test + add);
			}
			{ // std::string to std::string&
				Function fun = [&](std::string& c) -> std::string { return c + add; };
				std::string test = "Helloworld";
				std::string res = fun(test);
				Assert(res == test + add);
			}
			{ // literal to const std::string&
				Function fun = [&](const std::string& c) -> std::string { return c + add; };
				const char test[] = "Helloworld";
				std::string res = fun(test);
				Assert(res == test + add);
			}
			{ // std::string to const std::string&
				Function fun = [&](const std::string& c) -> std::string { return c + add; };
				std::string test = "Helloworld";
				std::string res = fun(test);
				Assert(res == test + add);
			}
			int addition = 10;
			{ // array
				Function fun = [&](int* c, size_t size) -> int { return c[size - 1] + addition; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1] + addition);
			}
			{ // array
				Function fun = [&](const int* c, size_t size) -> int { return c[size - 1] + addition; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1] + addition);
			}
			{ // array
				Function fun = [&](const int* c, size_t size) -> int { return c[size - 1] + addition; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1] + addition);
			}
			{ // array passed as pointer
				Function fun = [&](int* c, size_t size) -> int { return c[size - 1] + addition; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arr, size);
				Assert(res == arr[size - 1] + addition);
			}
			{ // array passed as pointer
				Function fun = [&](const int* c, size_t size) -> int { return c[size - 1] + addition; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1] + addition);
			}
			{ // array passed as pointer
				Function fun = [&](const int* c, size_t size) -> int { return c[size - 1] + addition; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1] + addition);
			}
			{ // array with size
				Function fun = [&](int c[5], size_t size) -> int { return c[size - 1] + addition; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1] + addition);
			}
			{ // array with size
				Function fun = [&](const int c[5], size_t size) -> int { return c[size - 1] + addition; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1] + addition);
			}
			{ // array with size passed as pointer
				Function fun = [&](int c[5], size_t size) -> int { return c[size - 1] + addition; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1] + addition);
			}
			{ // array with size passed as pointer
				Function fun = [&](const int c[5], size_t size) -> int { return c[size - 1] + addition; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1] + addition);
			}
			{ // array reference with size
				Function fun = [&](int(&c)[5], size_t size) -> int { return c[size - 1] + addition; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun(arr, size);
				Assert(res == arr[size - 1] + addition);
			}
			{ // array reference with size
				Function fun = [&](const int(&c)[5], size_t size) -> int { return c[size - 1] + addition; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int res = fun((const int(&)[5])arr, size);
				Assert(res == arr[size - 1] + addition);
			}
			{ // array reference passed as pointer
				Function fun = [&](int* (&c), size_t size) -> int { return c[size - 1] + addition; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1] + addition);
			}
			{ // array reference passed as pointer
				Function fun = [&](const int* (&c), size_t size) -> int { return c[size - 1] + addition; };
				int arr[5]{ 1, 2, 3, 4, 5 };
				int size = 5;
				const int* arrp = arr;
				int res = fun(arrp, size);
				Assert(res == arr[size - 1] + addition);
			}
			Assert(_BinderBase::refcount == 0);
		}


		struct NonTrivial
		{
			NonTrivial(int v)
				: myint(std::make_unique<int>(v)) {}

			NonTrivial(const NonTrivial& copy)
				: myint(std::make_unique<int>(copy.Value())) {}

			int Value() const { return *myint; }
			std::unique_ptr<int> myint;
		};
		int AOINnnon(NonTrivial a)
		{
			return a.Value();
		}

		Test(NonTrivialTypes)
		{
			Function fun = [](NonTrivial a, int other) -> int { return a.Value() + other; };

			int size = sizeof(NonTrivial);

			NonTrivial a{ 30 };
			auto res1 = fun(a);
			auto res = res1(1);
			Assert(res == a.Value() + 1);

		}

		static int Return99() { return 99; }

		Test(Functions)
		{
			Function fun = [](int(a)(void)) { return a(); };

			int(*a1)(void) = []() -> int { return 99; };
			int res1 = fun(a1);
			Assert(res1 == 99);

			int(&a2)(void) = Return99;
			int res2 = fun(a2);
			Assert(res2 == 99);

			int res3 = fun(Return99);
			Assert(res3 == 99);

			int res4 = fun([]() -> int { return 99; });
			Assert(res4 == 99);
		}

		Test(NoArguments)
		{
			{
				Function fun = []() { return 99; };
				int res = fun();
				Assert(res == 99);
			}
			{
				Function fun = []() { return "AppleJuice"; };
				auto res = fun();
				Assert(res == "AppleJuice");
			}
			{
				int val = 99;
				Function fun = [=]() { return val; };
				int res = fun();
				Assert(res == val);
			}
			{
				int val = 99;
				Function fun = [&]() { return val; };
				int res = fun();
				Assert(res == val);
			}
			{
				std::string val = "AppleJuice";
				Function fun = [=]() { return val; };
				auto res = fun();
				Assert(res == val);
			}
			{
				std::string val = "AppleJuice";
				Function fun = [&]() { return val; };
				auto res = fun();
				Assert(res == val);
			}
		}
	};
}