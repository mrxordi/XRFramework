#pragma once
class ID3DResource
{
public:
	virtual ~ID3DResource() {};

	virtual void OnDestroyDevice() {};
	virtual void OnCreateDevice() {};
	virtual void OnFreeResources() {};
	virtual void OnResizeDevice() {};
};

class Texture : public ID3DResource
{
public:
	Texture(){};
	~Texture(){};

	void OnDestroyDevice(){};
	void OnCreateDevice(){};
	void OnResizeDevice(){};
};
