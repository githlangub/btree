#include "../workbase/legacy-array.hpp"

#include <iostream>

int main()
{
	int array1[10] = {0};
	int array2[10] = {0};

	for(int i = 0; i < 10; ++i)
	{
		array1[i] = i;
	}

	//LegacyArray<int>::Copy(array1, array2, 10);
	CopyLegacyArray<int>(array1 + 0, array2 + 0, 10);

	for(int i = 0; i < 10; ++i)
	{
		std::cout << array2[i] << ' ';
	}
	std::cout << std::endl;

	return 0;
}
