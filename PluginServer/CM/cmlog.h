
#ifndef _CMLOG_H_561A9A7E_C430_48a4_8AFB_CC71E22101AE
#define _CMLOG_H_561A9A7E_C430_48a4_8AFB_CC71E22101AE

#include "CMHelper.h"
#include "CMFile.h"
#include "CMLock.h"
#include "CMSharePtr.h"
#include <map>
#include <sstream> 
#include <iostream> 
#include "ca_api.h"


//日志开关宏
#define  _CMLOG 

using namespace  std; 

_CMHELPER_BEGIN

class CCMLog_Sig
{
public:
	enum 
	{
		DATA_LOG_SIG_NONE = 0, 
		DATA_LOG_SIG_END = 1, 
	};
	CCMLog_Sig() {m_nType = DATA_LOG_SIG_NONE;} 
	CCMLog_Sig(int nType) {m_nType = nType; } 
	BOOL IsEndWrite() { return m_nType == DATA_LOG_SIG_END;}
private:
	int m_nType; 
};

class CCMLog
{
public:

public:
	static CCMLog& Instance(LPCTSTR lpstrFilename)
	{
#ifdef _CMLOG 
 
		static CMHelper::CCritSec s_csMap;
		CMHelper::CAutoLock_CritSec Lock(&s_csMap);
 
		TCHAR szAppData[MAX_PATH] = {0};
		::SHGetSpecialFolderPath(NULL,szAppData, CSIDL_APPDATA, true);
		TCHAR szPathFmt[MAX_PATH] = {0};
		if (lpstrFilename)
		{ 
			_sntprintf(szPathFmt, _countof(szPathFmt), _T("%s\\ftnn\\1.0\\logs\\%s"), szAppData, lpstrFilename); 
		} 
 
		std::basic_string<TCHAR> strFilename(szPathFmt);

		CMHelper::CCMSharePtr<CMHelper::CCMLog> sp;

		static std::map<std::basic_string<TCHAR>, CMHelper::CCMSharePtr<CMHelper::CCMLog> > s_map;
		std::map<std::basic_string<TCHAR>, CMHelper::CCMSharePtr<CMHelper::CCMLog> >::iterator it = s_map.find(strFilename);
		if(it == s_map.end())
		{
			sp = new CMHelper::CCMLog(szPathFmt);
			s_map.insert(std::map<std::basic_string<TCHAR>, CMHelper::CCMSharePtr<CMHelper::CCMLog> >::value_type(strFilename, sp));
		}
		else
		{
			sp = it->second;
		} 
		return *sp;
#else 
	 static CMHelper::CCMLog  s_logNoUse(NULL); 
	 return s_logNoUse; 
#endif 
	}
	CCMLog(LPCTSTR lpstrFilename)
	{
#ifdef _CMLOG 
		if (lpstrFilename) 
		{
			CString strName = lpstrFilename; 
			CString strExt; 

			int nPos = strName.ReverseFind(_T('.'));
			if (nPos > 0) 
			{
				strExt = strName.Right(strName.GetLength()-nPos); 
				strName = strName.Left(nPos); 
			}
			
			CTime tm=CTime::GetCurrentTime(); 
			CString strNameNew; 
			strNameNew.Format(_T("%s_%d%.02d%.02d%s"), strName, tm.GetYear(),
				tm.GetMonth(),tm.GetDay(), strExt); 

			std::basic_string<TCHAR> strFilename(strNameNew);
			CA::CreateDirectory(CString(strFilename.c_str())); 

			m_File.Open(strFilename.c_str(), false, false);
			m_File.Seek(0, SEEK_END);
		} 
#endif 
	}
	void Write(LPCSTR lpstrContent)
	{
#ifdef _CMLOG 
		SYSTEMTIME st = {0};
		::GetLocalTime(&st);

		static char szBuf[16276] = {0};
		int nSize = sizeof(szBuf)/sizeof(char); 
		int nRet = _snprintf_s(szBuf, nSize, nSize-1, "[%02u\\%02u %02u:%02u:%02u:%02u] %s\r\n", st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, lpstrContent);
		if(nRet < 0) nRet = nSize -1;

		CMHelper::CAutoLock_CritSec Lock(&m_csFile);
		m_File.Seek(0, SEEK_END);
		m_File.Write((unsigned char *)szBuf, nRet);
#endif 
	}
	void Write2(LPCSTR lpstrFormat, ...)
	{
#ifdef _CMLOG 
		va_list args;

		va_start(args, lpstrFormat);
		static char szBuf[16276] = {0};
		int nSize = sizeof(szBuf)/sizeof(char); 
		_vsnprintf_s(szBuf, nSize, nSize-1, lpstrFormat, args);
		va_end(args);

		Write(szBuf);
#endif 
	}

	CCMLog& operator <<  (UINT  nVal) 
	{
#ifdef _CMLOG 
		 //cout << nVal; 
		 m_strLog << nVal; 
#endif 
		return *this; 
	}
	CCMLog& operator << (LPCWSTR pwstrLog) 
	{
#ifdef _CMLOG 
		//cout << pwstrLog; 
		 m_strLog << (LPCSTR)CW2A(pwstrLog); 
#endif 
		 return *this; 
	} 
	CCMLog& operator << (LPCSTR pstrLog)  
	{
#ifdef _CMLOG 
		//cout << pstrLog; 
		 m_strLog << pstrLog; 
#endif 
		return *this;
	}
	CCMLog& operator << (int nVal)
	{
#ifdef _CMLOG 
		//cout << nVal; 
		m_strLog << nVal; 
#endif
		return *this; 
	}
	CCMLog& operator << (UINT64 ddwVal)
	{
#ifdef _CMLOG 
		 //cout << ddwVal; 
		 m_strLog << ddwVal; 
#endif 
		 return *this; 
	}
	CCMLog& operator << (INT64 ddwVal)
	{
#ifdef _CMLOG 
		//cout << ddwVal; 
		m_strLog << ddwVal; 
#endif 
		return *this; 
	}
	CCMLog& operator << (DWORD dwVal)
	{
#ifdef _CMLOG 
		//cout << dwVal; 
		m_strLog << dwVal; 
#endif 
		return *this; 
	}
	CCMLog& operator << (float fVal) 
	{
#ifdef _CMLOG 
		//cout << fVal; 
		m_strLog <<fVal; 
#endif 
		 return *this; 
	}
	CCMLog& operator << (double dVal)
	{
#ifdef _CMLOG 
		//cout << dVal; 
		m_strLog << dVal; 
#endif 
		return *this; 
	}
	CCMLog& operator << (BYTE byVal) 
	{
#ifdef _CMLOG 
		//cout << byVal; 
		m_strLog << byVal; 
#endif 
		return *this; 
	}
	
	CCMLog& operator << (short sVal) 
	{
#ifdef _CMLOG 
		//cout << sVal; 
		m_strLog << sVal; 
#endif 
		return *this; 
	}
	CCMLog& operator << (USHORT sVal) 
	{
#ifdef _CMLOG 
		//cout << sVal; 
		m_strLog << sVal; 
#endif 
		return *this; 
	}
	CCMLog& operator << (CCMLog_Sig& stSig)
	{
#ifdef _CMLOG 
		//cout << endl; 
		if (stSig.IsEndWrite()) 
		{ 
			std::string  strLog = m_strLog.str();  
			if (strLog.size() > 0) 
			{
				Write(strLog.c_str()); 
			} 
			m_strLog.str(""); 
		}
#endif 
		return *this; 
	}
	
#ifdef _CMLOG
	private:
		CMHelper::CCMFile	m_File;
		CMHelper::CCritSec	m_csFile;
		std::ostringstream		m_strLog; 
#endif 
};
_CMHELPER_END

static CMHelper::CCMLog_Sig  endLog(CMHelper::CCMLog_Sig::DATA_LOG_SIG_END);  
#define CMLog(x) (CMHelper::CCMLog::Instance(x))


#endif	// _CMLOG_H_561A9A7E_C430_48a4_8AFB_CC71E22101AE