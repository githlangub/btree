#ifndef __LEGACY_ARRAY_HPP__
#define __LEGACY_ARRAY_HPP__

#include "type.hpp"


template<typename T>
class LegacyArray
{
public:
	typedef T* Ptr;

public:
	static void Copy(const Ptr&, const Ptr&, const SizeType&);
};


template<typename T>
void CopyLegacyArray(T*const&, T*const&, const SizeType&);

#include "legacy-array.tpp"

#endif
