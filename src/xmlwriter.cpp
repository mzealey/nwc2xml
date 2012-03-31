// XMLWriter.cpp: implementation of the CXMLWriter class.
//
//////////////////////////////////////////////////////////////////////

#include "Precompile.h"
#include "XMLWriter.h"
//#include "xmlutils.h"
#ifdef XML_BINARY
#include "base64coder.h"
#endif

#ifdef LOCALE_SUPPORT
#include "localehelper.h"
#else
#define SAVE_CURRENT_LOCALE()
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static wxString Str2XML( const wxString& str )
{
	wxString s = str;
	s.Replace(_T("&"), _T("&amp;"));
	s.Replace(_T("<"), _T("&lt;"));
	s.Replace(_T(">"), _T("&gt;"));
	return s;
}

wxString	StrWriteXMLString(LPCTSTR szKey, LPCTSTR szValue)
{
	wxString strHexy = Str2XML(szValue);
	wxString str;
	str.Printf(_T("<%s>%s</%s>\n"), szKey, strHexy, szKey);

	return str;
}

wxString	StrWriteXMLInteger(LPCTSTR szKey, int nValue)
{
	TCHAR szValue[32];
	_stprintf(szValue, _T("%d"), nValue);

	wxString str;
	str.Printf(_T("<%s>%s</%s>\n"), szKey, szValue, szKey);

	return str;
}

wxString	StrWriteXMLDouble(LPCTSTR szKey, double dValue)
{
	wxString s;
	{
		SAVE_CURRENT_LOCALE();
		s.Printf(_T("%g"), dValue);
	}

	wxString str;
	str.Printf(_T("<%s>%s</%s>\n"), szKey, s, szKey);

	return str;
}

void WriteStringToFile(FILE* pFile, const char* str)
{
	size_t len = strlen(str);
	fwrite(str, 1, len, pFile);
}

#ifdef UNICODE
void WriteStringToFile(FILE* pFile, const wxString& str)
{
	wxCharBuffer cb = str.ToUTF8();
	size_t len = strlen(cb);
	fwrite(cb, 1, len, pFile);
}
#endif

CXMLWriter::CXMLWriter()
	: m_pFile(NULL)
	, m_bKeyOpen(FALSE)
	, m_bValueWritten(FALSE)
{
}

CXMLWriter::CXMLWriter(LPCTSTR szFileName, BOOL bStandalone)
: m_pFile(NULL)
, m_bKeyOpen(FALSE)
, m_bValueWritten(FALSE)
{
	Create(szFileName, bStandalone);
}

CXMLWriter::~CXMLWriter()
{
	fclose(m_pFile);
	m_pFile = NULL;
}

BOOL CXMLWriter::Create(LPCTSTR szFileName, BOOL bStandalone)
{
	FILE* pFile = wxFopen(szFileName, _T("wt"));
	if ( pFile == NULL )
	{
		return FALSE;
	}

	if ( bStandalone )
		WriteStringToFile(pFile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	else
		WriteStringToFile(pFile, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone='no'?>\n");

	m_pFile = pFile;

	return TRUE;
}

// can write \n after closing > : no plain string after close
void CXMLWriter::WriteKeyClose(bool bCanWriteNL)
{
	if ( m_bKeyOpen )
	{
		if ( bCanWriteNL )
			WriteStringToFile(m_pFile, ">\n");
		else
			WriteStringToFile(m_pFile, ">");
		m_bKeyOpen = FALSE;
	}
}

void CXMLWriter::WriteDocTypeSystem(LPCTSTR szElement, LPCTSTR szURI, LPCTSTR szDecl)
{
	wxString str;
	str.Printf(_T("<!DOCTYPE %s SYSTEM \"%s\""), szElement, szURI);
	WriteStringToFile(m_pFile, str);

	if ( szDecl )
	{
		str.Printf(_T("\n[%s]"), szDecl);
		WriteStringToFile(m_pFile, ">\n");
	}

	WriteStringToFile(m_pFile, ">\n");
}

void CXMLWriter::WriteDocTypePublic(LPCTSTR szElement, LPCTSTR szId, LPCTSTR szURI, LPCTSTR szDecl)
{
	wxString str;
	str.Printf(_T("<!DOCTYPE %s PUBLIC \"%s\" \"%s\""), szElement, szId, szURI);
	WriteStringToFile(m_pFile, str);

	if ( szDecl )
	{
		str.Printf(_T("\n[%s]"), szDecl);
		WriteStringToFile(m_pFile, ">\n");
	}

	WriteStringToFile(m_pFile, ">\n");
}

void CXMLWriter::WriteComment(LPCTSTR szComment)
{
	WriteKeyClose(true);

	wxString str;
	if ( szComment )
	{
		LPCTSTR szStart = wxStrstr(szComment, _T("<!--"));
		LPCTSTR szEnd = wxStrstr(szComment, _T("-->"));
		if ( szStart == 0 && szEnd == 0 )
		{
			str.Printf(_T("<!--%s-->\n"), szComment);
			WriteStringToFile(m_pFile, str);
		}
		else if ( szStart && szEnd )
		{
			// after szEnd, all chars should be whitespaces.
			LPCTSTR s = szEnd + 3;
			while( *s )
			{
				if ( !isspace(*s) )
					return;
			}

			// looking for whether there is another starting comment
			LPCTSTR szFind = wxStrstr(szComment+1, _T("<!--"));
			if ( szFind )
				return;

			str.Printf(_T("%s\n"), szComment);
			WriteStringToFile(m_pFile, str);
		}
	}
}

void CXMLWriter::WriteKeyStart(LPCTSTR szKey)
{
	WriteKeyClose(true);

	wxString str;
	str.Printf(_T("<%s"), szKey);
	WriteStringToFile(m_pFile, str);

	m_bKeyOpen = TRUE;
	m_bValueWritten = FALSE;

	m_strKeys.Add(szKey);
}

#ifdef XML_BINARY
#define ENCODING_BASE64			"encoding=\"base64\""

void CXMLWriter::WriteKeyStartBinary(LPCTSTR szKey, LPVOID pData, int nSize)
{
	WriteKeyClose(true);

	Base64Coder coder;
	coder.Encode((const PBYTE)pData, nSize);
	wxString str;
	str.Printf(_T("<%s %s>%s\n"), szKey, ENCODING_BASE64, coder.EncodedMessage());
	WriteStringToFile(m_pFile, str);
	m_bValueWritten = TRUE;

	m_strKeys.Add(szKey);
}
#endif

void CXMLWriter::WriteKeyStartBoolean(LPCTSTR szKey, bool bValue)
{
	wxString s(bValue ? _T("true") : _T("false"));
	WriteKeyStartString(szKey, s);
}

void CXMLWriter::WriteKeyStartDouble(LPCTSTR szKey, double dValue)
{
	wxString s;
	{
		SAVE_CURRENT_LOCALE();
		s.Printf(_T("%g"), dValue);
	}

	WriteKeyStartString(szKey, s);
}

void CXMLWriter::WriteKeyStartInteger(LPCTSTR szKey, int nValue)
{
	TCHAR szValue[32];
	_stprintf(szValue, _T("%d"), nValue);

	WriteKeyStartString(szKey, szValue);
}

void CXMLWriter::WriteKeyStartString(LPCTSTR szKey, LPCTSTR szValue)
{
	WriteKeyClose(true);

	wxString strHexy = Str2XML(szValue);
	wxString str;
	str.Printf(_T("<%s>%s\n"), szKey, strHexy);
	WriteStringToFile(m_pFile, str);
	m_bValueWritten = TRUE;

	m_strKeys.Add(szKey);
}

void CXMLWriter::WriteKeyStartEnd(LPCTSTR szKey)
{
	WriteKeyClose(true);

	wxString str;
	str.Printf(_T("<%s/>\n"), szKey);
	WriteStringToFile(m_pFile, str);
}

void CXMLWriter::WriteKeyEnd()
{
	if ( m_bValueWritten == FALSE && m_bKeyOpen != FALSE )
	{
		WriteStringToFile(m_pFile, _T("/>\n"));
		m_bKeyOpen = FALSE;

		size_t nSize = m_strKeys.GetCount();
		if ( nSize )
		{
			m_strKeys.RemoveAt(nSize-1);
		}
	}
	else
	{
		WriteKeyClose();

		size_t nSize = m_strKeys.GetCount();
		if ( nSize )
		{
			wxString strKey = m_strKeys[nSize-1];
			m_strKeys.RemoveAt(nSize-1);

			wxString str;
			str.Printf(_T("</%s>\n"), strKey);
			WriteStringToFile(m_pFile, str);
		}
	}
}

#ifdef XML_BINARY
void CXMLWriter::WriteBinary(LPCTSTR szKey, LPVOID pData, int nSize)
{
	WriteKeyClose(true);

	Base64Coder coder;
	coder.Encode((const PBYTE)pData, nSize);
	wxString str;
	str.Printf(_T("<%s %s>\n%s\n</%s>\n"), szKey, ENCODING_BASE64, coder.EncodedMessage(), szKey);
	WriteStringToFile(m_pFile, str);
}
#endif

void CXMLWriter::WriteBoolean(LPCTSTR szKey, bool bValue)
{
	WriteKeyClose(true);

	wxString str = StrWriteXMLString(szKey, bValue ? _T("true") : _T("false"));
	WriteStringToFile(m_pFile, str);
}

void CXMLWriter::WriteDouble(LPCTSTR szKey, double dValue)
{
	WriteKeyClose(true);

	wxString str = StrWriteXMLDouble(szKey, dValue);
	WriteStringToFile(m_pFile, str);
}

void CXMLWriter::WriteInteger(LPCTSTR szKey, int nValue)
{
	WriteKeyClose(true);

	wxString str = StrWriteXMLInteger(szKey, nValue);
	WriteStringToFile(m_pFile, str);
}

void CXMLWriter::WriteString(LPCTSTR szKey, LPCTSTR szValue)
{
	WriteKeyClose(true);

	wxString str = StrWriteXMLString(szKey, szValue);
	WriteStringToFile(m_pFile, str);
}

void CXMLWriter::WriteAttrBoolean(LPCTSTR szKey, bool bValue)
{
	wxASSERT( m_bKeyOpen );

	wxString s(bValue ? _T("true") : _T("false"));
	WriteAttrUnhexifyString(szKey, s);
}

void CXMLWriter::WriteAttrDouble(LPCTSTR szKey, double dValue)
{
	wxASSERT( m_bKeyOpen );

	wxString s;
	{
		SAVE_CURRENT_LOCALE();
		s.Printf(_T("%g"), dValue);
	}

	WriteAttrUnhexifyString(szKey, s);
}

void CXMLWriter::WriteAttrInteger(LPCTSTR szKey, int nValue)
{
	wxASSERT( m_bKeyOpen );

	TCHAR szValue[32];
	_stprintf(szValue, _T("%d"), nValue);

	WriteAttrUnhexifyString(szKey, szValue);
}

void CXMLWriter::WriteAttrString(LPCTSTR szKey, LPCTSTR szValue)
{
	wxASSERT( m_bKeyOpen );

	wxString str;
	str.Printf(_T(" %s=\"%s\""), szKey, Str2XML(szValue));
	WriteStringToFile(m_pFile, str);
}

void CXMLWriter::WriteAttrURL(LPCTSTR szKey, LPCTSTR szValue)
{
	wxString str;
	str.Printf(_T(" %s=\"file:///%s\""), szKey, Str2XML(szValue));
	WriteStringToFile(m_pFile, str);
}

void CXMLWriter::WriteAttrUnhexifyString(LPCTSTR szKey, LPCTSTR szValue)
{
	wxString str;
	str.Printf(_T(" %s=\"%s\""), szKey, szValue);
	WriteStringToFile(m_pFile, str);
}

void CXMLWriter::WriteString(LPCTSTR szValue)
{
	WriteKeyClose();

	wxString strHexy = Str2XML(szValue);
	WriteStringToFile(m_pFile, strHexy);
	m_bValueWritten = TRUE;
}

void CXMLWriter::WriteRawString(LPCTSTR szValue)
{
	WriteKeyClose();

	WriteStringToFile(m_pFile, szValue);
	m_bValueWritten = TRUE;
}
