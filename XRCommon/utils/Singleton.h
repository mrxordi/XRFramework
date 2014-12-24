#pragma once

#include <cassert>

/* Copyright (C) Scott Bilas, 2000.
* All rights reserved worldwide.
*
* This software is provided "as is" without express or implied
* warranties. You may freely copy and compile this source into
* applications you distribute provided that the copyright text
* below is included in the resulting source code, for example:
* "Portions Copyright (C) Scott Bilas, 2000"
*/


template <typename T> class Singleton {
protected:
	// TODO: Come up with something better than this!
	// TODO:
	// TODO: This super-nasty piece of nastiness was put in for continued
	// TODO: compatability with MSVC++ and MinGW - the latter apparently
	// TODO: needs this.
	static T* ms_Singleton;

public:
	Singleton(void)
	{
		assert(!ms_Singleton);
		ms_Singleton = static_cast<T*>(this);
	}
	~Singleton(void)
	{
		assert(ms_Singleton);  ms_Singleton = 0;
	}
	static T& getSingleton(void)
	{
		assert(ms_Singleton);  return (*ms_Singleton);
	}
	static T* getSingletonPtr(void)
	{
		return (ms_Singleton);
	}

private:
	Singleton& operator=(const Singleton&) { return this; }
	Singleton(const Singleton&) {}
};