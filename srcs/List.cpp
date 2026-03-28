#ifndef LIST_CPP_SENTINEL
#define LIST_CPP_SENTINEL

#include "List.hpp"

template<class U>
Item<U>::Item( U value )
{
	data = value;
	next = nullptr;
	prev = nullptr;
}

template<class U>
void Item<U>::SetData( U value )
{
	data = value;
}

#endif
