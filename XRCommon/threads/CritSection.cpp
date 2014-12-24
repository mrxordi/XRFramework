#include "stdafx.h"
#include "CritSection.h"

namespace XR {

#ifdef DEBUG
	CCriticalSection::CCriticalSection() {
		InitializeCriticalSection(&m_CritSection);
		m_currentOwner = m_lockCount = 0;
		m_fTrace = FALSE;
	}

	CCriticalSection::~CCriticalSection() {
		DeleteCriticalSection(&m_CritSection);
	}








#endif // DEBUG

}
