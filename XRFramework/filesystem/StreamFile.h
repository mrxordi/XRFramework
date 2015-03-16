#pragma once
#include <iostream>
class CStreamFile : public std::istream
{
public:
	CStreamFile();
	virtual ~CStreamFile();
};

