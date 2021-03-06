#include "stdafxf.h"
#include "AutoBuffer.h"
#include <new> // for std::bad_alloc
#include <stdlib.h> // for malloc(), realloc() and free()


auto_buffer::auto_buffer(size_t size) : p(0), s(0)
{
	if (!size)
		return;

	p = malloc(size); // "malloc()" instead of "new" allow to use "realloc()"
	if (!p)
		throw std::bad_alloc();
	s = size;
}

auto_buffer::~auto_buffer()
{
	free(p);
}

auto_buffer& auto_buffer::allocate(size_t size)
{
	clear();
	if (size)
	{
		p = malloc(size);
		if (!p)
			throw std::bad_alloc();
		s = size;
	}
	return *this;
}

auto_buffer& auto_buffer::resize(size_t newSize)
{
	if (!newSize)
		return clear();

	void* newPtr = realloc(p, newSize);
	if (!newPtr)
		throw std::bad_alloc();
	p = newPtr;
	s = newSize;
	return *this;
}

auto_buffer& auto_buffer::clear(void)
{
	free(p);
	p = 0;
	s = 0;
	return *this;
}

auto_buffer& auto_buffer::attach(void* pointer, size_t size)
{
	clear();
	if ((pointer && size) || (!pointer && !size))
	{
		p = pointer;
		s = size;
	}
	return *this;
}

void* auto_buffer::detach(void)
{
	void* returnPtr = p;
	p = 0;
	s = 0;
	return returnPtr;
}

