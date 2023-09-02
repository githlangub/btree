#ifndef __LEGACY_ARRAY_TPP__
#define __LEGACY_ARRAY_TPP__

#include "type.hpp"

template<typename T>
void LegacyArray<T>::Copy(const Ptr& pSrc, const Ptr& pDst, const SizeType& size)
{
	if(pDst - pSrc < 0)
	{
		for(int i = 0; i < size; ++i)
		{
			pDst[i] = pSrc[i];
		}
	}
	else if(pDst - pSrc > 0)
	{
		for(int i = size - 1; i >= 0; --i)
		{
			pDst[i] = pSrc[i];
		}
	}
	else
	{
		;
	}

	return;
}


template<typename T>
void CopyLegacyArray(T*const& pSrc, T*const& pDst, const SizeType& size)
{
	return LegacyArray<T>::Copy(pSrc, pDst, size);
}

#endif
