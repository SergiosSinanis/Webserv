#include "../../includes/ConfigFile.hpp"

template<typename T>
void print_vector(const std::vector<T> &v)
{
	typename std::vector<T>::const_iterator it;
	typename std::vector<T>::const_iterator ite = v.end();

	if (ite - v.begin() > 200)
		ite = v.begin() + 200;
	for (it = v.begin(); it != ite; it++)
		std::cout << " " << *it;

	std::cout << std::endl;
}