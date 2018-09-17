#include <iostream>
#include <functional>

template<typename T>
void println(const T & arg)
{
	std::cout << arg << '\n';
}

template<typename T>
void printerr(const T & arg)
{
	std::cerr << arg << '\n';
}

