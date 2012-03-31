// XMLWriter.h: interface for the CXMLWriter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__XMLWriter_H__)
#define __XMLWriter_H__

#pragma once

//#define XML_BINARY

class CXMLWriter
{
public:
	CXMLWriter();
	CXMLWriter(LPCTSTR szFileName, BOOL bStandalone=FALSE);
	~CXMLWriter();

	BOOL Create(LPCTSTR szFileName, BOOL bStandalone=FALSE);
	BOOL IsOk() const	{ return m_pFile != NULL; }

	void WriteDocTypeSystem(LPCTSTR szElement, LPCTSTR szURI, LPCTSTR szDecl=NULL);
	void WriteDocTypePublic(LPCTSTR szElement, LPCTSTR szId, LPCTSTR szURI, LPCTSTR szDecl=NULL);

	void WriteComment(LPCTSTR szComment);

	void WriteKeyStart(LPCTSTR szKey);
#ifdef XML_BINARY
	void WriteKeyStartBinary(LPCTSTR szKey, LPVOID pData, int nSize);
#endif
	void WriteKeyStartBoolean(LPCTSTR szKey, bool bValue);
	void WriteKeyStartDouble(LPCTSTR szKey, double dValue);
	void WriteKeyStartInteger(LPCTSTR szKey, int nValue);
	void WriteKeyStartString(LPCTSTR szKey, LPCTSTR szValue);
	void WriteKeyStartEnd(LPCTSTR szKey);
	void WriteKeyEnd();

#ifdef XML_BINARY
	void WriteBinary(LPCTSTR szKey, LPVOID pData, int nSize);
#endif
	void WriteBoolean(LPCTSTR szKey, bool bValue);
	void WriteDouble(LPCTSTR szKey, double dValue);
	void WriteInteger(LPCTSTR szKey, int nValue);
	void WriteString(LPCTSTR szKey, LPCTSTR szValue);

	void WriteAttrBoolean(LPCTSTR szKey, bool bValue);
	void WriteAttrDouble(LPCTSTR szKey, double dValue);
	void WriteAttrInteger(LPCTSTR szKey, int nValue);
	void WriteAttrString(LPCTSTR szKey, LPCTSTR szValue);
	void WriteAttrURL(LPCTSTR szKey, LPCTSTR szValue);

	void WriteString(LPCTSTR szValue);
	void WriteRawString(LPCTSTR szValue);

protected:
	FILE*			m_pFile;
	TVector<wxString>	m_strKeys;
	bool			m_bKeyOpen;
	bool			m_bValueWritten;

	void WriteKeyClose(bool bCanWriteNL = false);
	void WriteAttrUnhexifyString(LPCTSTR szKey, LPCTSTR szValue);
};

#endif // !defined(__XMLWriter_H__)
