#pragma once
#include <vector>
#include <map>
#include <algorithm>

/*!
* \struct CResolution
* \brief represents possible resolutions of monitor
*/
struct Resolution 
{
	int width,
		height;
	float refreshReate;
	int ordinalMonitor;
};


class CMonitor
{
public:
	CMonitor() : m_hMonitor(nullptr), m_ordinalMonitor(-1) {};
	CMonitor(const CMonitor& monitor);
	virtual ~CMonitor();

	void Attach(const HMONITOR hMonitor, int ordinal);
	HMONITOR Detach();

	void ClipRectToMonitor(LPRECT lprc, const BOOL UseWorkAreaRect = FALSE) const;
	void CenterRectToMonitor(LPRECT lprc, const BOOL UseWorkAreaRect = FALSE) const;
	void CenterWindowToMonitor(const HWND pWnd, const BOOL UseWorkAreaRect = FALSE) const;

	HDC CreateDC() const;

	//properties
	void GetMonitorRect(LPRECT lprc) const;
	void GetWorkAreaRect(LPRECT lprc) const;

	void GetName(std::wstring& string) const;
	int GetOrdinal() const {
		return m_ordinalMonitor;
	};

	int GetBitsPerPixel() const;

	BOOL IsOnMonitor(const POINT& pt) const;
	BOOL IsOnMonitor(const HWND pWnd) const;
	BOOL IsOnMonitor(const LPRECT lprc) const;

	BOOL IsPrimaryMonitor() const;
	BOOL IsMonitor() const;

	//operators
	operator HMONITOR() const {
		return this == nullptr ? nullptr : m_hMonitor;
	}

	BOOL operator ==(const CMonitor& monitor) const {
		return m_hMonitor == (HMONITOR)monitor;
	}

	BOOL operator !=(const CMonitor& monitor) const {
		return !(*this == monitor);
	}

	CMonitor& operator =(const CMonitor& monitor) {
		m_hMonitor = (HMONITOR)monitor;
		return *this;
	}

	void AddResolution(const Resolution& resolution);
	int GetNumResolutions() { return m_Resolutions.size() - 1; }

	std::vector<Resolution>& GetResolutions() {
		return m_Resolutions;
	}

	static std::string StringFromResolution(const Resolution& res);
	static std::string ResolutionFromString(const std::string& str);

private:
	HMONITOR m_hMonitor;
	std::vector<Resolution> m_Resolutions;
	int m_ordinalMonitor;
};



class CMonitors
{
public:
	CMonitors();
	virtual ~CMonitors();

	CMonitor* GetMonitor(const int index) const;


	int GetCount() const {
		return (int)m_MonitorMap.size();
	}

	//static members
	CMonitor* GetNearestMonitor(const LPRECT lprc);
	CMonitor* GetNearestMonitor(const POINT& pt);
	CMonitor* GetNearestMonitor(const HWND pWnd);

	static BOOL IsOnScreen(const POINT& pt);
	static BOOL IsOnScreen(const HWND pWnd);
	static BOOL IsOnScreen(const LPRECT lprc);
	static BOOL IsMonitor(const HMONITOR hMonitor);

	static void GetVirtualDesktopRect(LPRECT lprc);
	
	CMonitor* GetPrimaryMonitor();
	BOOL AllMonitorsShareDisplayFormat();

	int GetMonitorCount();

private:
	std::map<HMONITOR, CMonitor*> m_MonitorMap;

	typedef struct tagMATCHMONITOR {
		HMONITOR target;
		BOOL foundMatch;
	} MATCHMONITOR, *LPMATCHMONITOR;

	static BOOL CALLBACK FindMatchingMonitorHandle(
		HMONITOR hMonitor,  // handle to display monitor
		HDC hdcMonitor,     // handle to monitor DC
		LPRECT lprcMonitor, // monitor intersection rectangle
		LPARAM dwData       // data
		);


	typedef struct tagADDMONITOR {
		std::map<HMONITOR, CMonitor*>* pMonitors;
		int currentIndex;
	} ADDMONITOR, *LPADDMONITOR;

	static BOOL CALLBACK AddMonitorsCallBack(
		HMONITOR hMonitor,  // handle to display monitor
		HDC hdcMonitor,     // handle to monitor DC
		LPRECT lprcMonitor, // monitor intersection rectangle
		LPARAM dwData       // data
		);
};

