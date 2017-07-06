/*********************************************************************
创建日期：2007年8月10日
作    者：kvls
文件描述：文件操作
使用说明：1、实例化类
2、通过Open函数打开文件
3、之后可以调用类中的其他函数
历史版本：1.00 － 创建
*********************************************************************/
#ifndef _CMFILE_H_813F0BE0_FDCA_4168_AD80_5CB11C30AEAF
#define _CMFILE_H_813F0BE0_FDCA_4168_AD80_5CB11C30AEAF

#include "CMHelper.h"
#include <string>

_CMHELPER_BEGIN
class CCMFile
{
public:
	CCMFile()
	{
		m_hFile = INVALID_HANDLE_VALUE;
	}
	~CCMFile()
	{
		Close();
	}
	bool Open(LPCTSTR lpstrFilename, unsigned long nAccess, unsigned long nCreate, unsigned long nShare = FILE_SHARE_READ|FILE_SHARE_WRITE)
	{
		if(!lpstrFilename || m_hFile != INVALID_HANDLE_VALUE)
			return false;

		m_hFile = ::CreateFile(lpstrFilename, nAccess, nShare, NULL, nCreate, FILE_ATTRIBUTE_NORMAL, NULL);
		m_strFilename = CT2CW(lpstrFilename);
		return m_hFile != INVALID_HANDLE_VALUE;
	}
	bool Open(LPCTSTR lpstrFilename, bool bRead, bool bTrunc)
	{
		unsigned long nAccess = GENERIC_READ, nCreate = OPEN_EXISTING;

		if(!bRead)
		{
			nAccess = GENERIC_WRITE;
			nCreate = OPEN_ALWAYS;
			if(bTrunc && _taccess(lpstrFilename, 0) == 0)
				nCreate |= TRUNCATE_EXISTING;
		}

		return Open(lpstrFilename, nAccess, nCreate);
	}
	inline bool IsOpen()
	{
		return m_hFile != INVALID_HANDLE_VALUE;
	}
	void Close()
	{
		if(m_hFile != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
			m_strFilename = _T("");
		}
	}
	unsigned long GetSize()
	{
		if(m_hFile == INVALID_HANDLE_VALUE)
			return 0;
		DWORD size = ::GetFileSize(m_hFile, NULL);
		if(size == INVALID_FILE_SIZE)
			return 0;
		return size;
	}
	bool GetTime(unsigned __int64 *pCreateTime, unsigned __int64 *pAccessTime, unsigned __int64 *pWriteTime)
	{
		if(m_hFile == INVALID_HANDLE_VALUE)
			return false;

		FILETIME CreateTime, AccessTime, WriteTime;
		if(!::GetFileTime(m_hFile, &CreateTime, &AccessTime, &WriteTime))
			return false;

		if(pCreateTime)
		{
			*pCreateTime = CreateTime.dwHighDateTime;
			*pCreateTime <<= 32;
			*pCreateTime |= CreateTime.dwLowDateTime;
		}
		if(pAccessTime)
		{
			*pAccessTime = AccessTime.dwHighDateTime;
			*pAccessTime <<= 32;
			*pAccessTime |= AccessTime.dwLowDateTime;
		}
		if(pWriteTime)
		{
			*pWriteTime = WriteTime.dwHighDateTime;
			*pWriteTime <<= 32;
			*pWriteTime |= WriteTime.dwLowDateTime;
		}

		return true;
	};
	unsigned long Read(unsigned char *pBuf, unsigned long nCount)
	{
		if((!pBuf) || m_hFile == INVALID_HANDLE_VALUE)
			return 0;

		unsigned long nRead = 0;
		if(::ReadFile(m_hFile, pBuf, nCount, &nRead, NULL))
			return nRead;

		return 0;
	}
	bool Read(std::string &strDoc)
	{
		if(m_hFile == INVALID_HANDLE_VALUE)
			return false;

		strDoc = "";

		unsigned long nSize = ::GetFileSize(m_hFile, NULL);
		if(nSize == 0) return true;

		unsigned char *pBuf = new unsigned char[nSize+1];
		if(!pBuf) return false;

		unsigned long nRead = 0;
		if((!::ReadFile(m_hFile, pBuf, nSize, &nRead, NULL)) || nRead != nSize)
		{
			delete [] pBuf;
			return false;
		}
		pBuf[nRead] = 0;

		strDoc = (char *)pBuf;
		delete [] pBuf;

		return true;
	}
	unsigned long Write(const unsigned char *pBuf, const unsigned long nCount)
	{
		if((!pBuf) || m_hFile == INVALID_HANDLE_VALUE)
			return 0;

		unsigned long nWrite = 0;
		if(::WriteFile(m_hFile, pBuf, nCount, &nWrite, NULL))
			return nWrite;

		return 0;
	}
	bool Write(const std::string &strDoc)
	{
		if(m_hFile == INVALID_HANDLE_VALUE)
			return false;

		DWORD nSize = (DWORD)strDoc.length();
		if(nSize == 0) return true;

		DWORD nWrite = 0;
		if(!::WriteFile(m_hFile, strDoc.c_str(), nSize, &nWrite, NULL))
			return false;

		return nWrite == nSize;
	}
	bool Seek(const long nDistanceToMove, const unsigned long nMoveMethod)
	{
		if(m_hFile == INVALID_HANDLE_VALUE)
			return false;

		return ::SetFilePointer(m_hFile, nDistanceToMove, NULL, nMoveMethod) != (DWORD)-1;
	}
	unsigned int GetCurPosition()
	{
		if(m_hFile == INVALID_HANDLE_VALUE)
			return ((unsigned int)-1);

		return ::SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT);
	}
	static unsigned int GetSize(LPCTSTR lpstrFilename)
	{
		if(!lpstrFilename || _taccess(lpstrFilename, 0) != 0)
			return 0;

		HANDLE hFile = ::CreateFile(lpstrFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile == INVALID_HANDLE_VALUE)
			return 0;

		unsigned int nFileSize = ::GetFileSize(hFile, NULL);

		::CloseHandle(hFile);

		return nFileSize;
	}
	LPCTSTR GetFilename()
	{
		return m_strFilename.c_str();
	}
public:
	HANDLE m_hFile;
	std::wstring m_strFilename;
};
_CMHELPER_END

#endif	// _CMFILE_H_813F0BE0_FDCA_4168_AD80_5CB11C30AEAF