#pragma once
#include "TestBase.hpp"
#include "PartialApplication.hpp"
#include <time.h>

using namespace faster;

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
			}
			{
				auto call5 = call4(e+0);
				auto call6 = call5(f+0);
				auto call7 = call6(g+0);
				auto call8 = call7(h+0);
				auto call9 = call8(i+0);
				auto call10 = call9(j+0);
				auto call11 = call10(k+0);
				auto call12 = call11(l+0);
				auto call13 = call12(m+0);
				auto call14 = call13(n+0);
				auto call15 = call14(o+0);
				auto call16 = call15(p+0);
				auto call17 = call16(e+0);
				auto call18 = call17(f+0);
				auto call19 = call18(g+0);
				auto call20 = call19(h+0);
				auto call21 = call20(i+0);
				auto call22 = call21(j+0);
				auto call23 = call22(k+0);
				auto call24 = call23(l+0);
				auto call25 = call24(m+0);
				auto call26 = call25(n+0);
				auto call27 = call26(o+0);
				T res = call27(p+0);
				Assert(res == (T)((T)a + (T)b + (T)c + (T)d + 2 * ((T)e + (T)f + (T)g + (T)h + (T)i + (T)j + (T)k + (T)l + (T)m + (T)n + (T)o + (T)p)));
			}
		}
	}

	Test(PrimitiveTypes) {
		std::srand(time(nullptr));
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
	}



};