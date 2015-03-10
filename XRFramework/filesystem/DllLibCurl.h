#pragma once
/*
*      Copyright (C) 2005-2013 Team XBMC
*      http://xbmc.org
*
*  This Program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*
*  This Program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with XBMC; see the file COPYING.  If not, see
*  <http://www.gnu.org/licenses/>.
*
*/

#include "XRThreads/CriticalSection.h"
#define CURL CURL_HANDLE
#include "curl/curl.h"
#undef CURL

class DllLibCurlGlobal 
{
		DllLibCurlGlobal() {};
	public:
		/* extend interface with buffered functions */
		void easy_aquire(const char *protocol, const char *hostname, CURL_HANDLE** easy_handle, CURLM** multi_handle);
		void easy_release(CURL_HANDLE** easy_handle, CURLM** multi_handle);
		void easy_duplicate(CURL_HANDLE* easy, CURLM* multi, CURL_HANDLE** easy_out, CURLM** multi_out);
		CURL_HANDLE* easy_duphandle(CURL_HANDLE* easy_handle);
		void CheckIdle();

		/* overloaded load and unload with reference counter */
		virtual bool Load();
		virtual void Unload();
		virtual void UnloadAll();

		static DllLibCurlGlobal& Get();

		/* structure holding a session info */
		typedef struct SSession 
		{
			unsigned int  m_idletimestamp;  // timestamp of when this object when idle
			std::string    m_protocol;
			std::string    m_hostname;
			bool          m_busy;
			CURL_HANDLE*  m_easy;
			CURLM*        m_multi;
		} SSession;

		typedef std::vector<SSession> VEC_CURLSESSIONS;
	private:
		static long m_curlReferences;
		static VEC_CURLSESSIONS m_sessions;
		static XR::CCriticalSection m_critSection;
	};