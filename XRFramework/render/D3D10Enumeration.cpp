#include "stdafxf.h"
#include "D3D10Enumeration.h"


D3D10Enumeration::D3D10Enumeration(void)
{
	m_pDXGIFactory = NULL;
	m_bHasEnumerated = false;
	if (!EnsureD3D10APIs())
		LOGERR("Enumeration - DirectX10 API not present!");

	ClearAdapterInfoList();

	CreateDXGIFactory(__uuidof(IDXGIFactory), (LPVOID*)&m_pDXGIFactory);
	if (!m_pDXGIFactory)
		LOGFATAL("Failed to create DXGIFactory - %s", GetLastError());
}


D3D10Enumeration::~D3D10Enumeration(void)
{
	Shutdown();
}

bool D3D10Enumeration::Initialize()
{
	if (m_pDXGIFactory == NULL)
		LOGERR("Enumeration - DXGIFactory not created!");

	m_bHasEnumerated = true;

	for (int index = 0;; ++index)
	{
		IDXGIAdapter* pAdapter = NULL;
		if (FAILED(m_pDXGIFactory->EnumAdapters(index, &pAdapter))){
			LOGDEBUG("Enumeration - Found %u adapters.", EnumAdapterInfo::GetAdapterCount());
			break;
		}

		DXGI_ADAPTER_DESC AdapterDesc;
		pAdapter->GetDesc(&AdapterDesc);

		EnumAdapterInfo* pAdapterInfo = new EnumAdapterInfo(index, AdapterDesc, pAdapter);
		if (!pAdapterInfo)
		{
			SAFE_RELEASE(pAdapter);
			LOGERR("Enumeration - Out of memory!");
		}

		m_AdapterInfoList.push_back(pAdapterInfo);
		D3D10Enumeration::s_adapterCount++;

	}
	return true;
}


bool D3D10Enumeration::Shutdown()
{
	ClearAdapterInfoList();
	SAFE_RELEASE(m_pDXGIFactory);
	D3D10Enumeration::s_adapterCount = 0;
	return true;
}


bool D3D10Enumeration::EnsureD3D10APIs(void)
{
	// If any module is non-NULL, this function has already been called.  Note
	// that this doesn't guarantee that all ProcAddresses were found.
	if (s_hModD3D10 != NULL || s_hModDXGI != NULL)
		return true;

	// This may fail if Direct3D 10 isn't installed
	s_hModD3D10 = LoadLibrary(_T("d3d10.dll"));
	if (s_hModD3D10 == NULL)
	{
		LOGERR("Enumeration - Failed to get d3d10.dll module");
	}


	s_hModDXGI = LoadLibrary(_T("dxgi.dll"));
	if (s_hModDXGI)
	{
		s_DynamicCreateDXGIFactory = (LPCREATEDXGIFACTORY)GetProcAddress(s_hModDXGI, "CreateDXGIFactory");
	}
	else
	{
		LOGERR("Enumeration - Failed to get CreateDXGIFactory process");
	}

	return (s_hModDXGI != NULL) && (s_hModD3D10 != NULL);
}


void D3D10Enumeration::CreateDXGIFactory(REFIID rInterface, void** ppOut)
{
	if (EnsureD3D10APIs() && s_DynamicCreateDXGIFactory != NULL)
		s_DynamicCreateDXGIFactory(rInterface, ppOut);
	else
		LOGERR("Enumeration - Failed to create DXGIFactory");
}

void D3D10Enumeration::ClearAdapterInfoList()
{
	for (std::vector<EnumAdapterInfo*>::iterator it = m_AdapterInfoList.begin(); it != m_AdapterInfoList.end(); it++)
	{
		delete *it;
	}

	m_AdapterInfoList.clear();
}

EnumAdapterInfo* D3D10Enumeration::GetAdapterInfo(UINT adapterOrdinal)
{
	if (m_AdapterInfoList.empty() || !m_bHasEnumerated)
		LOGERR("Enumeration - Object not initialized");

	for (std::vector<EnumAdapterInfo*>::iterator it = m_AdapterInfoList.begin(); it != m_AdapterInfoList.end(); ++it)
	{
		EnumAdapterInfo* pAdapter = *it;
		if (pAdapter->AdapterOrdinal == adapterOrdinal)
			return *it;
	}
	return NULL;
}

IDXGIOutput* D3D10Enumeration::GetOutputFromMonitor(HMONITOR hMonitor)
{
	for (unsigned int i = 0; i < m_AdapterInfoList.size(); i++)
	{
		EnumAdapterInfo* adapterinfo = m_AdapterInfoList[i];
		std::vector<IDXGIOutput*>& outputList = adapterinfo->GetOutputList();
		for (unsigned int j = 0; j < outputList.size(); j++)
		{
			DXGI_OUTPUT_DESC outDesc;
			outputList[j]->GetDesc(&outDesc);
			if (outDesc.Monitor == hMonitor)
			{
				return outputList[j];
			}
		}

	}
	LOGERR("Output return from MONITOR failed! No MONITOR found.");
	return NULL;

}

unsigned int D3D10Enumeration::s_adapterCount = 0;

LPCREATEDXGIFACTORY D3D10Enumeration::s_DynamicCreateDXGIFactory = NULL;

HMODULE D3D10Enumeration::s_hModD3D10 = NULL;

HMODULE D3D10Enumeration::s_hModDXGI = NULL;

UINT EnumAdapterInfo::s_AdapterCount = 0;

void EnumAdapterInfo::InitOutputList()
{
	if (m_pAdapter == NULL)
		return;

	for (int index = 0;; index++)
	{
		IDXGIOutput* pOutput = NULL;
		if (FAILED(m_pAdapter->EnumOutputs(index, &pOutput))){
			LOGDEBUG("Enumeration - Found %u Outputs.", m_OutputList.size());
			break;
		}

		m_OutputList.push_back(pOutput);
	}
}