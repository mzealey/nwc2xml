// nwcfile.cpp : Defines the entry point for the console application.
//

#include "Precompile.h"
#include "nwcfile.h"
#include "wx/wfstream.h"
#include "wx/mstream.h"
#include "wx/zstream.h"
#include "wx/filename.h"

DWORD UncompressNSave(LPCTSTR szFileName, BYTE* compr, size_t comprLen)
{
	wxMemoryInputStream mis(compr, comprLen);

	wxZlibInputStream zis(mis, wxZLIB_ZLIB);

	wxFile file;
	if ( file.Create(szFileName, true) == false )
	{
		wxRemoveFile(szFileName);
		return wxSysErrorCode();
	}

	wxFileOutputStream fos(file);
	zis.Read(fos);

	return ERROR_SUCCESS;
}

DWORD CNWCFile::LoadCompressed(wxFile& in, FILE* out, FILELOAD fl)
{
#define HEADER_SIZE	6

	long lSize = in.Length();
	if ( lSize <= HEADER_SIZE )
		return ERROR_INVALID_DATA;

	wxString strTmpName = wxFileName::CreateTempFileName(_T("N2X"));

	DWORD dwResult = ERROR_GEN_FAILURE;
	// save compressed file content without [NWZ]\0
	{
		TByteArray dtCompressed;
		dtCompressed.SetCount(lSize - HEADER_SIZE);
		in.Seek(HEADER_SIZE, wxFromStart);
		in.Read(&dtCompressed[0], lSize-HEADER_SIZE);
		in.Close();

		dwResult = UncompressNSave(strTmpName, &dtCompressed[0], dtCompressed.GetCount());
		if ( ERROR_SUCCESS != dwResult )
		{
			wxRemoveFile(strTmpName);
			return dwResult;
		}
	}

	wxFile file;
	if ( file.Open(strTmpName) )
	{
		dwResult = Load(file, out, fl);

		file.Close();
	}

	wxRemoveFile(strTmpName);

	return dwResult;
#undef	HEADER_SIZE
}

LPCTSTR	GetMeasureNumbersAsString(BYTE btMeasureNumbers)
{
	static LPCTSTR s_String[] = {_T("None"), _T("Plain"), _T("Circled"), _T("Boxed") };
	if ( btMeasureNumbers < _countof(s_String) )
		return s_String[btMeasureNumbers];
	else
		return _T("unknown");
}

LPCTSTR	GetStaffLabelsAsString(BYTE btStaffLabels)
{
	static LPCTSTR s_String[] = {_T("None"), _T("First Systems"), _T("Top Systems"), _T("All Systems") };
	if ( btStaffLabels < _countof(s_String) )
		return s_String[btStaffLabels];
	else
		return _T("unknown");
}

CNWCFile::CNWCFile()
{
	nVersion = 0;
	memset(btUnknown2, 0, sizeof(btUnknown2));
	memset(&chExtendLastSystem, 0, offsetof(CNWCFile, strMarginTop) - offsetof(CNWCFile, chExtendLastSystem));
	memset(&btTitlePageInfo, 0, offsetof(CNWCFile, mStaffs) - offsetof(CNWCFile, btTitlePageInfo));
}

CNWCFile::~CNWCFile()
{
	size_t nCount = mStaffs.GetCount();
	for ( size_t i=0 ; i<nCount ; i++ )
	{
		CStaff* pStaff = mStaffs[i];
		delete pStaff;
	}

	mStaffs.RemoveAll();
}

void	CNWCFile::Init()
{
	strUser.Empty();
	strUnknown1.Empty();
	nVersion = 0;

	strTitle.Empty();
	strAuthor.Empty();
	strLyricist.Empty();
	strCopyright1.Empty();
	strCopyright2.Empty();
	strComment.Empty();

	strMarginTop.Empty();
	strMarginInside.Empty();
	strMarginOutside.Empty();
	strMarginBottom.Empty();

	mFontInfos.RemoveAll();

	size_t nCount = mStaffs.GetCount();
	for ( size_t i=0 ; i<nCount ; i++ )
	{
		CStaff* pStaff = mStaffs[i];
		delete pStaff;
	}

	mStaffs.RemoveAll();
}

static	char szCompressHeader1[] = "[NWZ]";	// implies NUL at last
static	char szNWCHeader[] = "[NoteWorthy ArtWare]\0\0\0[NoteWorthy Composer]";	// implies NUL at last

DWORD CNWCFile::Load(wxFile& in, FILE* out, FILELOAD fl)
{
	char buf[64];
	if ( sizeof(szNWCHeader) != in.Read(buf, sizeof(szNWCHeader)) ||
		 memcmp(szNWCHeader, buf, sizeof(szNWCHeader)) != 0 )
	{
		if ( memcmp(szCompressHeader1, buf, sizeof(szCompressHeader1)) == 0 )
		{
			return LoadCompressed(in, out, fl);
		}

		wxFprintf(stderr, _T("invalid szNWCHeader\n"));
		return ERROR_INVALID_DATA;
	}

	if ( !ReadLEShort(in, nVersion) )
	{
		wxFprintf(stderr, _T("unexpected EOF while reading version\n"));
		wxASSERT(false);
		return ERROR_INVALID_DATA;
	}

	if ( IsValidVersion(nVersion) )
	{
		wxFprintf(out, _T("version:%d.%d\n"), nVerMajor, nVerMinor);
	}
	else
	{
		wxFprintf(stderr, _T("unknown version number\n"));
		return ERROR_INVALID_VERSION;
	}

	long nPos = in.Tell();
	char _szHeader2[4] = "\x02\x00\x00";							// implies NUL at last
	if ( !ReadBytes(in, btUnknown2) ||
		 btUnknown2[0] != _szHeader2[0] || btUnknown2[1] != _szHeader2[1] || btUnknown2[3] != _szHeader2[3] )
	{
		//wxFprintf(stderr, "invalid btUnknown2 at 0x%08x\n", nPos);
		//wxASSERT(false);
	}

	if ( btUnknown2[2] == 5 )
	{
		wxFprintf(out, _T("saved by unregistered version\n"));
	}
	else if ( btUnknown2[2] != 0 )
	{
		//wxFprintf(stderr, "invalid btUnknown2 at 0x%08x\n", nPos);
	}

	strUser = LoadStringNULTerminated(in);
	strUnknown1 = LoadStringNULTerminated(in);
	wxFprintf(out, _T("user:'%s', '%s'\n"), strUser.c_str(), strUnknown1.c_str());

	nPos = in.Tell();
	char szHeader3[8] = "\x00\x00\x00\x00\x00\x00\x00";					// implies NUL at last
	if ( sizeof(szHeader3) != in.Read(buf, sizeof(szHeader3)) ||
		 memcmp(szHeader3, buf, sizeof(szHeader3)) != 0 )
	{
		wxFprintf(stderr, _T("invalid btUnknown2 at 0x%08x\n"), nPos);
		//wxASSERT(false);
	}

	short nUnknown4;
	if ( !ReadBytes(in, nUnknown4) )
	{
		wxFprintf(stderr, _T("unexpected EOF while reading\n"));
		wxASSERT(false);
		return ERROR_INVALID_DATA;
	}

	strTitle = LoadStringNULTerminated(in);
	strAuthor = LoadStringNULTerminated(in);
	if ( nVersion >= NWC_Version200 )
		strLyricist = LoadStringNULTerminated(in);
	strCopyright1 = LoadStringNULTerminated(in);
	strCopyright2 = LoadStringNULTerminated(in);
	strComment = LoadStringNULTerminated(in);
	wxFprintf(out, _T("title:'%s'\n"), strTitle.c_str());
	wxFprintf(out, _T("author:'%s'\n"), strAuthor.c_str());
	if ( nVersion >= NWC_Version200 )
		wxFprintf(out, _T("lyricist:'%s'\n"), strLyricist.c_str());
	wxFprintf(out, _T("copyright1:'%s'\n"), strCopyright1.c_str());
	wxFprintf(out, _T("copyright2:'%s'\n"), strCopyright2.c_str());
	wxFprintf(out, _T("comment:'%s'\n"), strComment.c_str());

	nPos = in.Tell();
	char _szReserved5[] = "\x5F\x00\x46\x32\x00";
	bool bResult = ReadBytes(in, chExtendLastSystem) &&
				   ReadBytes(in, chIncreaseNoteSpacing) &&
				   ReadBytes(in, btUnknown3) &&
				   ReadBytes(in, btMeasureNumbers) &&
				   ReadBytes(in, btUnknown4) &&
				   ReadLEShort(in, nMeasureStart);
	if ( !bResult || memcmp(_szReserved5, btUnknown3, sizeof(btUnknown3)) != 0 )
	{
//		wxFprintf(stderr, "invalid szReserved5 at 0x%08x\n", nPos);
//		wxASSERT(false);
//		return ERROR_INVALID_DATA;
	}
	wxFprintf(out, _T("extend last system:'%c'\n"), chExtendLastSystem);
	wxFprintf(out, _T("increase note spacing:'%c'\n"), chIncreaseNoteSpacing);
	wxFprintf(out, _T("measurenumbers:'%s'\n"), GetMeasureNumbersAsString(btMeasureNumbers));
	wxFprintf(out, _T("measurestart:%d\n"), nMeasureStart);

	if ( nVersion >= NWC_Version130 )
	{
		strMarginTop = LoadStringSpaceTerminated(in);
		strMarginInside = LoadStringSpaceTerminated(in);
		strMarginOutside = LoadStringSpaceTerminated(in);
		strMarginBottom = LoadStringSpaceTerminated(in);
	}

	wxFprintf(out, _T("margin:%s %s %s %s\n"), strMarginTop.c_str(), strMarginInside.c_str(), strMarginOutside.c_str(), strMarginBottom.c_str());

	ReadBytes(in, bMirrorMargin);
	wxFprintf(out, _T("mirrormargin:%d\n"), bMirrorMargin);

	nPos = in.Tell();
	if ( !ReadBytes(in, btUnknown5) )
	{
		return ERROR_INVALID_DATA;
	}

	if ( nVersion > NWC_Version130 )
	{
		ReadBytes(in, nGroupVisibility);
		ReadBytes(in, bAllowLayering);
		wxFprintf(out, _T("group visibility:"));
		CObj::DumpBinary(out, nGroupVisibility, sizeof(nGroupVisibility), false);
		wxFprintf(out, _T("\nallowlayering:%d\n"), bAllowLayering);
	}
	nPos = in.Tell();
	//if ( btUnknown5[0] == 0x07 || btUnknown5[0] == 0x08 )
	//{
	//	strNotationTypeface = LoadStringNULTerminated(in);
	//	wxFprintf(out, _T("notationtypeface:%s\n"), strNotationTypeface);
	//}
	ReadLEShort(in, nStaffHeight);
	wxFprintf(out, _T("staffheight=%d\n"), nStaffHeight);

	nPos = in.Tell();

	int nFontCount = 0;
	if ( nVersion > NWC_Version130 )
		nFontCount = ( nVersion <= NWC_Version170 ? 10 : 12 );
	mFontInfos.SetCount(nFontCount);
	int i;
	for ( i=0; i<nFontCount; i++ )
	{
		mFontInfos[i].Load(in);
	}

	// some 1.70 version have 12 fontinfo
	if ( nVersion == NWC_Version170 )
	{
		nPos = in.Tell();
		unsigned char ch;
		if ( in.Read(&ch, 1) != 1 )
			return ERROR_INVALID_DATA;
		in.Seek(nPos, wxFromStart);

		if ( ch == 0 || ch == 0xFF )
		{
		}
		else
		{
			nFontCount += 2;
			mFontInfos.SetCount(nFontCount);
			for ( ; i<nFontCount; i++ )
			{
				mFontInfos[i].Load(in);
			}
		}
	}

	wxFprintf(out, _T("#font=%d\n"), nFontCount);
	for ( i=0; i<nFontCount; i++ )
	{
		mFontInfos[i].Dump(out, i);
	}

	ReadBytes(in, btTitlePageInfo);
	ReadBytes(in, btStaffLabels);	// index of [None, First Systems, Top Systems, All Systems]
	ReadLEShort(in, nStartPageNo);
	if ( nVersion >= NWC_Version200 )
	{
		unsigned char ch;
		if ( in.Read(&ch, 1) != 1 )
			return ERROR_INVALID_DATA;

		if ( ch != 0xFF )
			in.Seek(-1, wxFromCurrent);
		//ReadBytes(in, btJustifyPrintedSystemVertically);
	}
	ReadLEShort(in, nStaffCount);
	wxFprintf(out, _T("titlepageinfo=%d\n"), btTitlePageInfo);
	wxFprintf(out, _T("stafflabels=%s\n"), GetStaffLabelsAsString(btStaffLabels));
	wxFprintf(out, _T("startpageno=%d\n"), nStartPageNo);
	//if ( nVersion >= NWC_Version200 )
	//	wxFprintf(out, _T("justifyprintedsystemvertically=%d\n"), btJustifyPrintedSystemVertically);
	wxFprintf(out, _T("staffcount=%d\n"), nStaffCount);

	fflush(out);

	for ( i=0; i<nStaffCount; i++ )
	{
		nPos = in.Tell();
		CStaff* pStaff = new CStaff;
		pStaff->SetParent(this);

		wxFprintf(out, _T("\nstaff%d\n"), i);
		if ( false == pStaff->Load(in, out, fl) )
		{
			delete pStaff;
			return ERROR_INVALID_STAFF;
		}
		mStaffs.Add(pStaff);
		fflush(out);

		// load until staff which has lyric
		//if ( fl == FILELOAD_INFO && pStaff->m_strLyric.size() )
		//	break;
	}

	return ERROR_SUCCESS;
}
