#include "stdafxf.h"
#include "D3D10Enumeration.h"


D3D10Enumeration::D3D10Enumeration(void)
{
	m_pDXGIFactory = NULL;
	m_bHasEnumerated = false;
	if (!EnsureD3D10APIs())
		LOGERR("(!) Enumeration - DirectX10 API not present!");

	ClearAdapterInfoList();

	CreateDXGIFactory(__uuidof(IDXGIFactory), (LPVOID*)&m_pDXGIFactory);
}


D3D10Enumeration::~D3D10Enumeration(void)
{
	Shutdown();
}

bool D3D10Enumeration::Initialize()
{
	if (m_pDXGIFactory == NULL)
		LOGERR("(!) Enumeration - DXGIFactory not created!");

	m_bHasEnumerated = true;

	for (int index = 0;; ++index)
	{
		IDXGIAdapter* pAdapter = NULL;
		if (FAILED(m_pDXGIFactory->EnumAdapters(index, &pAdapter))){
			LOGINFO("(i) Enumeration - Found %u adapters.", EnumAdapterInfo::GetAdapterCount());
			break;
		}

		DXGI_ADAPTER_DESC AdapterDesc;
		pAdapter->GetDesc(&AdapterDesc);

		EnumAdapterInfo* pAdapterInfo = new EnumAdapterInfo(index, AdapterDesc, pAdapter);
		if (!pAdapterInfo)
		{
			SAFE_RELEASE(pAdapter);
			LOGERR("(!) Enumeration - Out of memory!");
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
	s_hModD3D10 = LoadLibrary("d3d10.dll");
	if (s_hModD3D10 == NULL)
	{
		LOGERR("(!) Enumeration - Failed to get d3d10.dll module");
	}


	s_hModDXGI = LoadLibrary("dxgi.dll");
	if (s_hModDXGI)
	{
		s_DynamicCreateDXGIFactory = (LPCREATEDXGIFACTORY)GetProcAddress(s_hModDXGI, "CreateDXGIFactory");
	}
	else
	{
		LOGERR("(!) Enumeration - Failed to get CreateDXGIFactory process");
	}

	return (s_hModDXGI != NULL) && (s_hModD3D10 != NULL);
}


void D3D10Enumeration::CreateDXGIFactory(REFIID rInterface, void** ppOut)
{
	if (EnsureD3D10APIs() && s_DynamicCreateDXGIFactory != NULL)
		s_DynamicCreateDXGIFactory(rInterface, ppOut);
	else
		LOGERR("(!)Enumeration - Failed to create DXGIFactory");
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
		LOGERR("(!)Enumeration - Object not initialized");

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
			LOGINFO("(i) Enumeration - Found %u Outputs.", m_OutputList.size());
			break;
		}

		m_OutputList.push_back(pOutput);
	}
}

/*bool EnumAdapterInfo::GetBestModeMach(RESOLUTION& res, DXGI_FORMAT EnumFormat, UINT Flags, UINT *pNumModes, DXGI_MODE_DESC& pDesc_out)
{
UINT numModes;
HRESULT hr;
IDXGIOutput* p_output = m_OutputList[res.iScreen];
//TODO: Pull these in from elsewhere
int matchWidth = res.iWidth;
int matchHeight = res.iHeight;
//Get the Display Modes that fit the DXGI FORMAT
//TODO: Which format should we use? Which Flags? Why?
hr = p_output->GetDisplayModeList(EnumFormat, Flags, &numModes, NULL);
if (FAILED(hr)) {
LOGERR("GetDisplayModeList for Formats Failed!");
return false;
}

//Create the DXGI Mode Description

DXGI_MODE_DESC* pDesc = new DXGI_MODE_DESC[numModes];
if (pDesc == NULL) {
LOGERR("Creation of DXGI_MODE_DESC Array with %i modes Failed!", numModes);
return false;
}

//Populate the DXGI_MODE_DESC list with structures of supported Display Modes
hr = p_output->GetDisplayModeList(EnumFormat, Flags, &numModes, pDesc);
if (FAILED(hr)) {
LOGERR("GetDisplayModeList for populating DXGI_MODE_DESC Array Failed!");
return false;
}

//Choose the Display Mode that most closely matches our current screen resolution
ZeroMemory(&pDesc_out, sizeof(pDesc_out));
UINT i;
UINT len = numModes;
for (i = 0; i < len; ++i) {
if (pDesc[i].Width == matchWidth && pDesc[i].Height == matchHeight) {
pDesc_out = pDesc[i];
delete[] pDesc;
return true;
}
}

LOGERR("No suitable display mode was found for resolution of %i, %i!", matchWidth, matchHeight);
return false;
}
*/