// settings/helpers/Monitors.cpp : implementation file
//

#include "stdafx.h"
#include "log/Log.h"
#include "settings/helpers/Monitors.h"
#include "utils/StringUtils.h"
#include "utils/XRect.h"

/*!
 * \class CMonitor
 *
 * \brief Represents monitor and its possible resolutions
 */
CMonitor::CMonitor(const CMonitor& monitor)
{
	m_hMonitor = monitor.m_hMonitor;
	m_Resolutions = monitor.m_Resolutions;
	m_ordinalMonitor = monitor.m_ordinalMonitor;
}
CMonitor::~CMonitor()
{
	m_Resolutions.clear();
}

void CMonitor::Attach(const HMONITOR hMonitor, int ordinal)
{
	ASSERT(CMonitors::IsMonitor(hMonitor));

	m_hMonitor = hMonitor;
	m_ordinalMonitor = ordinal;
}

HMONITOR CMonitor::Detach()
{
	HMONITOR hMonitor = m_hMonitor;
	m_hMonitor = nullptr;
	m_Resolutions.clear();
	m_ordinalMonitor = -1;
	return hMonitor;
}

// creates an HDC for the monitor
// it is up to the client to call DeleteDC
//
// for normal multimonitor drawing it is not necessary to get a
// dc for each monitor. Windows takes care of drawing correctly
// on all monitors
//
// Only very exacting applications would need a DC for each monitor
HDC CMonitor::CreateDC() const
{
	ASSERT(IsMonitor());

	std::wstring name;
	GetName(name);

	//create a dc for this display
	HDC hdc = ::CreateDC(name.c_str(), name.c_str(), nullptr, nullptr);
	ASSERT(hdc != nullptr);

	//set the viewport based on the monitor rect's relation to the primary monitor
	XRect rect;
	GetMonitorRect(&rect);

	::SetViewportOrgEx(hdc, -rect.left, -rect.top, nullptr);
	::SetViewportExtEx(hdc, rect.Width(), rect.Height(), nullptr);

	return hdc;
}

int CMonitor::GetBitsPerPixel() const
{
	HDC hdc = CreateDC();
	int ret = ::GetDeviceCaps(hdc, BITSPIXEL) * ::GetDeviceCaps(hdc, PLANES);
	ASSERT(::DeleteDC(hdc));

	return ret;
}

void CMonitor::GetName(std::wstring& string) const
{
	ASSERT(IsMonitor());

	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);
	::GetMonitorInfo(m_hMonitor, &mi);

	string = mi.szDevice;
}

//
// these methods return true if any part of the item intersects the monitor rect
BOOL CMonitor::IsOnMonitor(const POINT& pt) const
{
	XRect rect;
	GetMonitorRect(rect);

	return rect.PtInRect(pt);
}

BOOL CMonitor::IsOnMonitor(const HWND pWnd) const
{
	XRect rect;
	GetMonitorRect(rect);

	ASSERT(::IsWindow(pWnd));
	XRect wndRect;
	GetWindowRect(pWnd, &wndRect);

	return rect.IntersectRect(rect, wndRect);
}

BOOL CMonitor::IsOnMonitor(const LPRECT lprc) const
{
	XRect rect;
	GetMonitorRect(rect);

	return rect.IntersectRect(rect, lprc);
}

void CMonitor::GetMonitorRect(LPRECT lprc) const
{
	ASSERT(IsMonitor());

	MONITORINFO mi;
	RECT        rc;

	mi.cbSize = sizeof(mi);
	::GetMonitorInfo(m_hMonitor, &mi);
	rc = mi.rcMonitor;

	::SetRect(lprc, rc.left, rc.top, rc.right, rc.bottom);
}

//
// the work area does not include the start bar
void CMonitor::GetWorkAreaRect(LPRECT lprc) const
{
	ASSERT(IsMonitor());

	MONITORINFO mi;
	RECT        rc;

	mi.cbSize = sizeof(mi);
	::GetMonitorInfo(m_hMonitor, &mi);
	rc = mi.rcWork;

	::SetRect(lprc, rc.left, rc.top, rc.right, rc.bottom);
}

//these two center methods are adapted from David Campbell's
//MSJ article (see comment at the top of the header file)
void CMonitor::CenterRectToMonitor(LPRECT lprc, const BOOL UseWorkAreaRect) const
{
	int  w = lprc->right - lprc->left;
	int  h = lprc->bottom - lprc->top;

	XRect rect;
	if (UseWorkAreaRect) {
		GetWorkAreaRect(&rect);
	}
	else {
		GetMonitorRect(&rect);
	}

	// MPC-HC custom code start
	// Added rounding to get exactly the same rect as the CWnd::CenterWindow method returns.
	lprc->left = std::lround(rect.left + (rect.Width() - w) / 2.0);
	lprc->top = std::lround(rect.top + (rect.Height() - h) / 2.0);
	// MPC-HC custom code end
	lprc->right = lprc->left + w;
	lprc->bottom = lprc->top + h;
}

void CMonitor::CenterWindowToMonitor(const HWND pWnd, const BOOL UseWorkAreaRect) const
{
	ASSERT(IsMonitor());
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd));

	XRect rect;
	GetWindowRect(pWnd, &rect);
	CenterRectToMonitor(&rect, UseWorkAreaRect);
	// MPC-HC custom code start
	// Check if we are a child window and modify the coordinates accordingly

	if (GetWindowLong(pWnd, GWL_STYLE) & WS_CHILD) {
		HWND parentHwnd = (HWND)GetWindowLongPtr(pWnd, GWL_HWNDPARENT);
		POINT topleft, bottomright;
		ScreenToClient(parentHwnd, &topleft);
		ScreenToClient(parentHwnd, &bottomright);
		rect.SetRect(topleft, bottomright);
	}
	// MPC-HC custom code end
	SetWindowPos(pWnd, nullptr, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CMonitor::ClipRectToMonitor(LPRECT lprc, const BOOL UseWorkAreaRect) const
{
	int w = lprc->right - lprc->left;
	int h = lprc->bottom - lprc->top;

	XRect rect;
	if (UseWorkAreaRect) {
		GetWorkAreaRect(&rect);
	}
	else {
		GetMonitorRect(&rect);
	}

	lprc->left = std::max(rect.left, std::min(rect.right - w, lprc->left));
	lprc->top = std::max(rect.top, std::min(rect.bottom - h, lprc->top));
	lprc->right = lprc->left + w;
	lprc->bottom = lprc->top + h;
}

//
// is the instance the primary monitor
BOOL CMonitor::IsPrimaryMonitor() const
{
	ASSERT(IsMonitor());

	MONITORINFO mi;

	mi.cbSize = sizeof(mi);
	::GetMonitorInfo(m_hMonitor, &mi);

	return mi.dwFlags == MONITORINFOF_PRIMARY;
}

//
// is the instance currently attached to a valid monitor handle
BOOL CMonitor::IsMonitor() const
{
	return CMonitors::IsMonitor(m_hMonitor);
}

void CMonitor::AddResolution(const Resolution& resolution)
{

	for (auto& res : m_Resolutions) {
		if (res.width == resolution.width && res.height == resolution.height &&
			res.refreshReate == res.refreshReate && res.ordinalMonitor == resolution.ordinalMonitor &&
			m_ordinalMonitor == resolution.ordinalMonitor)
			return;
	}
	m_Resolutions.push_back(resolution);
}

std::string CMonitor::StringFromResolution(const Resolution& res)
{
	return StringUtils::Format("%1i%05i%05i%09.6f", res.ordinalMonitor, res.width, res.height, res.refreshReate);
}

std::string CMonitor::ResolutionFromString(const std::string& str)
{

	return std::string("");
}


/*!
* \class CMonitors
* \brief represents all found monitors
*/
CMonitors::CMonitors()
{
	// WARNING : GetSystemMetrics(SM_CMONITORS) return only visible display monitors, and  EnumDisplayMonitors
	// enumerate visible and pseudo invisible monitors !!!
	//  m_MonitorArray.SetSize( GetMonitorCount() );

	ADDMONITOR addMonitor;
	addMonitor.pMonitors = &m_MonitorMap;
	addMonitor.currentIndex = 0;

	::EnumDisplayMonitors(nullptr, nullptr, AddMonitorsCallBack, (LPARAM)&addMonitor);
}

CMonitors::~CMonitors()
{
	std::map<HMONITOR, CMonitor*>::iterator it = m_MonitorMap.begin();
		for (; it != m_MonitorMap.end(); it++) {
		delete it->second;
	}
	m_MonitorMap.clear();
}

// CMonitors member functions
BOOL CALLBACK CMonitors::AddMonitorsCallBack(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	LPADDMONITOR pAddMonitor = (LPADDMONITOR)dwData;

	CMonitor* pMonitor = new CMonitor;
	pMonitor->Attach(hMonitor, pAddMonitor->currentIndex);

	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);
	::GetMonitorInfo(hMonitor, &mi);

	DEVMODEW devmode;
	ZeroMemory(&devmode, sizeof(devmode));
	devmode.dmSize = sizeof(devmode);

	if (EnumDisplaySettingsExW(mi.szDevice, ENUM_CURRENT_SETTINGS, &devmode, 0) == FALSE)
		EnumDisplaySettingsExW(mi.szDevice, ENUM_REGISTRY_SETTINGS, &devmode, 0);

	Resolution res;
	res.width = devmode.dmPelsWidth; res.height = devmode.dmPelsHeight;
	res.refreshReate = devmode.dmDisplayFrequency;
	res.ordinalMonitor = pAddMonitor->currentIndex;
	pMonitor->AddResolution(res);										//Current default mode for monitor

	for (int i = 0;; i++) {												//And now we can enumerate through additional modes

		if (EnumDisplaySettingsW(mi.szDevice, i, &devmode) == 0)
			break;
		if (devmode.dmBitsPerPel != 32)
			continue;

		float refreshRate;
		if (devmode.dmDisplayFrequency == 59 || devmode.dmDisplayFrequency == 29 || devmode.dmDisplayFrequency == 23)
			refreshRate = (float)(devmode.dmDisplayFrequency + 1) / 1.001f;
		else
			refreshRate = (float)(devmode.dmDisplayFrequency);

		Resolution res;
		res.width = devmode.dmPelsWidth; res.height = devmode.dmPelsHeight;
		res.refreshReate = refreshRate;
		res.ordinalMonitor = pAddMonitor->currentIndex;
		pMonitor->AddResolution(res);
	}

	pAddMonitor->pMonitors->insert(std::make_pair(hMonitor, pMonitor));
	LOGINFO("Found #%i monitor with %i modes.", pAddMonitor->currentIndex, pMonitor->GetNumResolutions());
	pAddMonitor->currentIndex++;

	return TRUE;
}

//
// returns the primary monitor
CMonitor* CMonitors::GetPrimaryMonitor()
{
	//the primary monitor always has its origin at 0,0
	HMONITOR hMonitor = ::MonitorFromPoint(XPoint(0, 0), MONITOR_DEFAULTTOPRIMARY);
	ASSERT(IsMonitor(hMonitor));

	std::map<HMONITOR, CMonitor*>::iterator it = m_MonitorMap.find(hMonitor);
	if (it != m_MonitorMap.end())
		ASSERT(it->second->IsPrimaryMonitor());

	return it->second;
}

//
// is the given handle a valid monitor handle
BOOL CMonitors::IsMonitor(const HMONITOR hMonitor)
{
	if (hMonitor == nullptr) {
		return FALSE;
	}

	MATCHMONITOR match;
	match.target = hMonitor;
	match.foundMatch = FALSE;

	::EnumDisplayMonitors(nullptr, nullptr, FindMatchingMonitorHandle, (LPARAM)&match);

	return match.foundMatch;
}

//this is the callback method that gets called via IsMontior
BOOL CALLBACK CMonitors::FindMatchingMonitorHandle(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	LPMATCHMONITOR pMatch = (LPMATCHMONITOR)dwData;

	if (hMonitor == pMatch->target) {
		//found a monitor with the same handle we are looking for
		pMatch->foundMatch = TRUE;
		return FALSE; //stop enumerating
	}

	//haven't found a match yet
	pMatch->foundMatch = FALSE;
	return TRUE;    //keep enumerating
}

BOOL CMonitors::AllMonitorsShareDisplayFormat()
{
	return ::GetSystemMetrics(SM_SAMEDISPLAYFORMAT);
}

//
// the number of monitors on the system
int CMonitors::GetMonitorCount()
{
	return ::GetSystemMetrics(SM_CMONITORS);
}

CMonitor* CMonitors::GetMonitor(const int index) const
{
	ASSERT(index >= 0 && index < m_MonitorMap.size());

	for (auto& map : m_MonitorMap) {
		if (map.second->GetOrdinal() == index) {
			return static_cast<CMonitor*>(map.second);
		}

	}
	return nullptr;
}

//
// returns the rectangle that is the union of all active monitors
void CMonitors::GetVirtualDesktopRect(LPRECT lprc)
{
	::SetRect(lprc,
		::GetSystemMetrics(SM_XVIRTUALSCREEN),
		::GetSystemMetrics(SM_YVIRTUALSCREEN),
		::GetSystemMetrics(SM_CXVIRTUALSCREEN),
		::GetSystemMetrics(SM_CYVIRTUALSCREEN));

}

//
// these methods determine whether the given item is
// visible on any monitor
BOOL CMonitors::IsOnScreen(const LPRECT lprc)
{
	return ::MonitorFromRect(lprc, MONITOR_DEFAULTTONULL) != nullptr;
}

BOOL CMonitors::IsOnScreen(const POINT& pt)
{
	return ::MonitorFromPoint(pt, MONITOR_DEFAULTTONULL) != nullptr;
}

BOOL CMonitors::IsOnScreen(const HWND pWnd)
{
	return ::MonitorFromWindow(pWnd, MONITOR_DEFAULTTONULL) != nullptr;
}

CMonitor* CMonitors::GetNearestMonitor(const LPRECT lprc)
{
	std::map<HMONITOR, CMonitor*>::iterator it = m_MonitorMap.find(::MonitorFromRect(lprc, MONITOR_DEFAULTTONEAREST));
	if (it != m_MonitorMap.end())
		return it->second;

	return nullptr;
}

CMonitor* CMonitors::GetNearestMonitor(const POINT& pt)
{
	std::map<HMONITOR, CMonitor*>::iterator it = m_MonitorMap.find(::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST));

	if (it != m_MonitorMap.end())
		return it->second;

	return nullptr;
}

CMonitor* CMonitors::GetNearestMonitor(const HWND pWnd)
{
	ASSERT(pWnd);
	ASSERT(::IsWindow(pWnd));

	std::map<HMONITOR, CMonitor*>::iterator it = m_MonitorMap.find(::MonitorFromWindow(pWnd, MONITOR_DEFAULTTONEAREST));

	if (it != m_MonitorMap.end())
		return it->second;

	return nullptr;
}
