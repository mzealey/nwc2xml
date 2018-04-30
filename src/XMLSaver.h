// XMLSaver.h: interface for the XMLSaver class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "nwcfile.h"

class CTextParser;
class CXMLWriter;

class XMLSaver
{
public:
	XMLSaver();
	virtual ~XMLSaver();

	typedef enum
	{
		MXML_10	= 0x100,
		MXML_11	= 0x101,
	} MXML_VERSION;

	bool	Save(LPCTSTR szNWCFile, LPCTSTR szFile, CNWCFile* pNWCObj, MXML_VERSION nVersion=MXML_10);

protected:

	inline	void	SaveClef(CXMLWriter& xw, const CClefObj& obj);
	inline	void	SaveKeySig(CXMLWriter& xw, const CKeySigObj& obj);
	inline	void	SaveBarLine(CXMLWriter& xw, const CBarLineObj& obj, int& nMeasurement, bool bNewMeasure,
								CObj *const * pNext, CObj *const * pLast);
	inline	void	SaveEnding(CXMLWriter& xw, const CEndingObj& obj);
	inline	void	SaveInstrument(CXMLWriter& xw, const CInstrumentObj& obj);
	inline	void	SaveTimeSig(CXMLWriter& xw, const CTimeSigObj& obj);
	inline	void	SaveTempo(CXMLWriter& xw, const CTempoObj& obj);
	inline	void	SaveDynamic(CXMLWriter& xw, const CDynamicObj& obj);
	inline	void	SaveNote(CXMLWriter& xw, const CNoteObj& obj, long nDivision,
								const CLyricArray& strLyrics, int& nLyricIndex, bool bChord=false);
	inline	void	UpdateNoteAcc(const CNoteObj& obj);
	inline	void	SaveRest(CXMLWriter& xw, const CRestObj& obj, long nDivision, bool bChord=false);
	inline	void	SaveNoteCM(	CXMLWriter& xw, const CNoteCMObj& obj, long nDivision,
								const CLyricArray& strLyrics, int& nLyricIndex);
	inline	void	SavePedal(CXMLWriter& xw, const CPedalObj& obj);
	inline	void	SaveFlowDir(CXMLWriter& xw, const CFlowDirObj& obj);
	inline	void	SaveMPC(CXMLWriter& xw, const CMPCObj& obj);
	inline	void	SaveTempVar(CXMLWriter& xw, const CTempVarObj& obj);
	inline	void	SaveDynVar(CXMLWriter& xw, const CDynVarObj& obj);
	inline	void	SavePerform(CXMLWriter& xw, const CPerformObj& obj);
	inline	void	SaveText(CXMLWriter& xw, const CTextObj& obj, const wxChar* szLang);
	inline	void	SaveRestCM(CXMLWriter& xw, const CRestCMObj& obj, long nDivision);

	inline	void	SaveForward(CXMLWriter& xw, int nDuration);
	inline	void	CheckMultiVoice(const CStaff& staff, int nIndex, int& nStartIndex);
	inline	void	SaveMultiVoice(CXMLWriter& xw, const CStaff& staff, long nDivision, int nStartIndex, int nEndIndex, int nMeasureDuration);
	inline	int		GetFirstVoiceDuration(TVector<CObj*>& objArray, int nDivision) const;

	inline	void	Save(CXMLWriter& xw, const CStaff& staff, int nMeasurement, const wxChar* szLang=NULL);
	inline	void	SaveEndingBarLine(CXMLWriter& xw, const CStaff& staff);

protected:
	MXML_VERSION	m_nVersion;
	CHROMALTER		m_CA;
	int				m_nSlurNo;
	int				m_nVoice;
	wxString			m_strEnding;
};
