#pragma once


class ContextOpaque;

/**
* This class sets up a few instances and services. Currently it only
*  provides a logger to the CThread functionality. If it never does
*  more than this it can be removed.
*/
class Context
{
	ContextOpaque* impl;
public:
	Context();
	virtual ~Context();
};