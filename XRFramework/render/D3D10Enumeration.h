#pragma once

#include <vector>
#include "Log/Log.h"
//#include "window/Resolution.h"
#include "Base.h"



#define DXGI_MAX_DEVICE_IDENTIFIER_STRING 128
typedef HRESULT(WINAPI* LPCREATEDXGIFACTORY)(REFIID, void**);

//Forward declarations
class EnumAdapterInfo;

class D3D10Enumeration
{
public:
	D3D10Enumeration(void);
	~D3D10Enumeration(void);
	bool Initialize();
	bool Shutdown();

	EnumAdapterInfo* GetAdapterInfo(UINT adapterOrdinal);
	IDXGIOutput* GetOutputFromMonitor(HMONITOR hMonitor);
	IDXGIFactory* GetDXGIFactory() { if (m_pDXGIFactory) return m_pDXGIFactory; else return NULL; }

	static bool EnsureD3D10APIs(void);
private:
	void ClearAdapterInfoList();
	void CreateDXGIFactory(REFIID rInterface, void** ppOut);

private:
	bool m_bHasEnumerated;
	IDXGIFactory* m_pDXGIFactory;
	std::vector<DXGI_FORMAT>	  m_DepthStencilPossibleList;
	std::vector<EnumAdapterInfo*> m_AdapterInfoList;


private:
	static unsigned int s_adapterCount;
	static HMODULE s_hModDXGI;
	static HMODULE s_hModD3D10;

	static LPCREATEDXGIFACTORY s_DynamicCreateDXGIFactory;

};


class EnumAdapterInfo
{
	const EnumAdapterInfo& operator =(const EnumAdapterInfo& rhs);
	static UINT s_AdapterCount;

public:
	EnumAdapterInfo(UINT ordinal, DXGI_ADAPTER_DESC Desc, IDXGIAdapter* pAdapter)
		: AdapterOrdinal(ordinal), AdapterDesc(Desc), m_pAdapter(pAdapter)
	{
		wcscpy_s(szUniqueDescription, 100, AdapterDesc.Description);
		WCHAR sz[100];
		swprintf_s(sz, 100, L" (#%d)", AdapterOrdinal);
		wcscat_s(szUniqueDescription, DXGI_MAX_DEVICE_IDENTIFIER_STRING, sz);
		s_AdapterCount++;

		InitOutputList();

	}

	virtual ~EnumAdapterInfo(){
		s_AdapterCount--;
		for (UINT i = 0; i < m_OutputList.size(); i++)
			SAFE_RELEASE(m_OutputList[i]);

		m_OutputList.clear();
		SAFE_RELEASE(m_pAdapter);
	}

	static UINT GetAdapterCount()
	{
		return s_AdapterCount;
	}

	virtual void InitOutputList();
	virtual IDXGIOutput* GetOutputordinal(unsigned int ordinal) {
		return m_OutputList[ordinal];
	}
	virtual unsigned int GetOutputCount() {
		return m_OutputList.size();
	}
	virtual std::vector<IDXGIOutput*>& GetOutputList() {
		return m_OutputList;
	}

	//virtual bool GetBestModeMach(RESOLUTION& res, DXGI_FORMAT EnumFormat, UINT Flags, UINT *pNumModes, DXGI_MODE_DESC& pDesc_out);


	UINT AdapterOrdinal;
	DXGI_ADAPTER_DESC AdapterDesc;
	WCHAR   szUniqueDescription[DXGI_MAX_DEVICE_IDENTIFIER_STRING];
	IDXGIAdapter* m_pAdapter;
	std::vector<IDXGIOutput*> m_OutputList;
};
