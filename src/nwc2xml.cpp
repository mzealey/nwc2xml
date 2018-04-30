// nwc2xml.cpp : Defines the entry point for the console application.
//

#include "Precompile.h"
#include "nwcfile.h"
#include "XMLSaver.h"
#include "wx/cmdline.h"
#include "nwc2xml-version.h"

DWORD ConvertNWC2MusicXML(const wxString& strNWC, bool bGenerateLog)
{
	wxFile in;
	if ( in.Open(strNWC) == FALSE )
		return wxSysErrorCode();

	wxString strFile;
	if ( bGenerateLog )
	{
		strFile = strNWC.Left(strNWC.Length()-3);
		strFile += _T("txt");
	}
	else
	{
#ifdef	_WIN32
		strFile = _T("NUL");
#else
		strFile = _T("/dev/null");
#endif
	}

	wxFFile out;
#ifdef	_WIN32
	if ( out.Open(strFile, _T("wt, ccs=UTF-8")) == FALSE )
#else
	if ( out.Open(strFile, _T("wt")) == FALSE )
#endif
	{
		wxFprintf(stderr, _T("%s : conversion failed.\n"), strNWC.c_str());
		return ERROR_GEN_FAILURE;
	}

	CNWCFile nwc;
	DWORD dwResult = nwc.Load(in, out.fp());

	out.Close();

	if ( dwResult != ERROR_SUCCESS )
	{
		wxFprintf(stderr, _T("%s : conversion failed.\n"), strNWC.c_str());
		wxRemoveFile(strFile);
		return dwResult;
	}

	strFile = strNWC.Left(strNWC.Length()-3);
	strFile += _T("xml");

	XMLSaver saver;
	saver.Save(strNWC, strFile, &nwc);

	if ( dwResult == ERROR_SUCCESS )
		wxFprintf(stderr, _T("%s : conversion saved to %s.\n"), strNWC.c_str(), strFile.c_str());
	else
		wxFprintf(stderr, _T("%s : conversion failed.\n"), strNWC.c_str());

	return dwResult;
}

#ifndef	_WIN32
wxCSConv* CreateWindowsCompatibleConv()
{
	wxFontEncoding encoding = wxLocale::GetSystemEncoding();
	wxFontEncoding oldEncoding = encoding;
	switch ( encoding )
	{
		// ISO Encoding
	case	wxFONTENCODING_ISO8859_1 :		encoding = wxFONTENCODING_CP1252;	break;
	case	wxFONTENCODING_ISO8859_2 :		encoding = wxFONTENCODING_CP1250;	break;
	case	wxFONTENCODING_ISO8859_5 :		encoding = wxFONTENCODING_CP1251;	break;
	case	wxFONTENCODING_ISO8859_6 :		encoding = wxFONTENCODING_CP1256;	break;
	case	wxFONTENCODING_ISO8859_7 :		encoding = wxFONTENCODING_CP1253;	break;
	case	wxFONTENCODING_ISO8859_8 :		encoding = wxFONTENCODING_CP1255;	break;
	case	wxFONTENCODING_ISO8859_9 :		encoding = wxFONTENCODING_CP1254;	break;
	case	wxFONTENCODING_ISO8859_11 :		encoding = wxFONTENCODING_CP874;	break;
	case	wxFONTENCODING_ISO8859_13 :		encoding = wxFONTENCODING_CP1257;	break;

		// Mac Encoding
	case	wxFONTENCODING_MACROMAN :		encoding = wxFONTENCODING_CP1252;	break;
	case	wxFONTENCODING_MACJAPANESE :	encoding = wxFONTENCODING_CP932;	break;
	case	wxFONTENCODING_MACCHINESETRAD :	encoding = wxFONTENCODING_CP950;	break;
	case	wxFONTENCODING_MACKOREAN :		encoding = wxFONTENCODING_CP949;	break;
	case	wxFONTENCODING_MACARABIC :		encoding = wxFONTENCODING_CP1256;	break;
	case	wxFONTENCODING_MACHEBREW :		encoding = wxFONTENCODING_CP1255;	break;
	case	wxFONTENCODING_MACGREEK :		encoding = wxFONTENCODING_CP1253;	break;
	case	wxFONTENCODING_MACCYRILLIC :	encoding = wxFONTENCODING_CP1251;	break;
	//case	wxFONTENCODING_MACDEVANAGARI :	encoding = ;	break;
	//case	wxFONTENCODING_MACGURMUKHI :	encoding = ;	break;
	//case	wxFONTENCODING_MACGUJARATI :	encoding = ;	break;
	//case	wxFONTENCODING_MACORIYA :		encoding = ;	break;
	//case	wxFONTENCODING_MACBENGALI :		encoding = ;	break;
	//case	wxFONTENCODING_MACTAMIL :		encoding = ;	break;
	//case	wxFONTENCODING_MACTELUGU :		encoding = ;	break;
	//case	wxFONTENCODING_MACKANNADA :		encoding = ;	break;
	//case	wxFONTENCODING_MACMALAJALAM :	encoding = ;	break;
	//case	wxFONTENCODING_MACSINHALESE :	encoding = ;	break;
	//case	wxFONTENCODING_MACBURMESE :		encoding = ;	break;
	//case	wxFONTENCODING_MACKHMER :		encoding = ;	break;
	case	wxFONTENCODING_MACTHAI :		encoding = wxFONTENCODING_ISO8859_11;	break;
	//case	wxFONTENCODING_MACLAOTIAN :		encoding = ;	break;
	//case	wxFONTENCODING_MACGEORGIAN :	encoding = ;	break;
	//case	wxFONTENCODING_MACARMENIAN :	encoding = ;	break;
	case	wxFONTENCODING_MACCHINESESIMP :	encoding = wxFONTENCODING_CP936;	break;
	//case	wxFONTENCODING_MACTIBETAN :		encoding = ;	break;
	//case	wxFONTENCODING_MACMONGOLIAN :	encoding = ;	break;
	//case	wxFONTENCODING_MACETHIOPIC :	encoding = ;	break;
	//case	wxFONTENCODING_MACCENTRALEUR :	encoding = ;	break;
	//case	wxFONTENCODING_MACVIATNAMESE :	encoding = ;	break;
	case	wxFONTENCODING_MACARABICEXT :	encoding = wxFONTENCODING_CP1256;	break;
	//case	wxFONTENCODING_MACDINGBATS :	encoding = ;	break;
	case	wxFONTENCODING_MACTURKISH :		encoding = wxFONTENCODING_CP1254;	break;
	//case	wxFONTENCODING_MACCROATIAN :	encoding = ;	break;
	//case	wxFONTENCODING_MACICELANDIC :	encoding = ;	break;
	//case	wxFONTENCODING_MACROMANIAN :	encoding = ;	break;
	//case	wxFONTENCODING_MACCELTIC :		encoding = ;	break;
	//case	wxFONTENCODING_MACGAELIC :		encoding = ;	break;
	}

	wxCSConv* pCSConv = NULL;
	if ( oldEncoding != encoding )
	{
		pCSConv = new wxCSConv(encoding);
	}

	return pCSConv;
}
#endif

static wxChar* s_ProgramLogo = _T(PROGRAM_ABOUT);

#if defined(__WXMSW__) && defined(UNICODE)
int wmain(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
#if defined(__WXMSW__) && defined(UNICODE)
#define wxArgv argv
#elif wxUSE_UNICODE
	wxChar **wxArgv = new wxChar *[argc + 1];
	{
		int n;

		for (n = 0; n < argc; n++ )
		{
			wxWCharBuffer warg = wxConvUTF8.cMB2WX(argv[n]);
			wxArgv[n] = wxStrdup(warg);
		}

		wxArgv[n] = NULL;
	}
#else // !wxUSE_UNICODE
#define wxArgv argv
#endif // wxUSE_UNICODE/!wxUSE_UNICODE

	wxInitializer initializer(argc, wxArgv);
	if ( !initializer )
	{
		fprintf(stderr, "Failed to initialize the wxWidgets library, aborting.");
		return -1;
	}

	static const wxCmdLineEntryDesc cmdLineDesc[] =
	{
		{ wxCMD_LINE_SWITCH, "h", "help",	"show this help message",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
		{ wxCMD_LINE_SWITCH, "l", "log",	"generate log file" },

		{ wxCMD_LINE_OPTION, NULL, "charset",	"conversion charset for lyric; default is system setting",
		wxCMD_LINE_VAL_STRING },

		{ wxCMD_LINE_PARAM,  NULL, NULL, "input file",
		wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_MULTIPLE },

		{ wxCMD_LINE_NONE }
	};

	wxCmdLineParser parser(cmdLineDesc, argc, wxArgv);
	parser.SetLogo(s_ProgramLogo);

	switch ( parser.Parse() )
	{
	case -1:
		wxLogMessage(_T("Help was given, terminating."));
		parser.Usage();
		break;

	case 0:
		break;

	default:
		wxLogMessage(_T("Syntax error detected, aborting."));
		return -1;
	}


	wxString strFile, strExt;

	g_pMBConv = &wxConvLocal;
	wxCSConv* pCSConv = NULL;
	wxString strCharset;
	if ( parser.Found(_T("charset"), &strCharset) )
	{
		pCSConv = new wxCSConv(strCharset);
		if ( !pCSConv->IsOk() )
		{
			wxFprintf(stderr, _T("unknown charset=%s.\n"), strCharset.c_str());
			delete pCSConv;
			pCSConv = NULL;
		}
		else
		{
			g_pMBConv = pCSConv;
		}
	}

#ifndef	_WIN32
	if ( g_pMBConv == &wxConvLocal )
	{
		pCSConv = CreateWindowsCompatibleConv();
		if ( pCSConv && pCSConv->IsOk() )
		{
			g_pMBConv = pCSConv;
		}
		else
		{
			delete pCSConv;
			pCSConv = NULL;
		}
	}
#endif

	bool bGenerateLog = parser.Found(_T("log"));

	size_t nParamCount = parser.GetParamCount();
	for ( size_t i=0 ; i <nParamCount ; i++ )
	{
		strFile = parser.GetParam(i);

		//printf("%s : ", strFile);
		DWORD dwResult = ERROR_SUCCESS;
		strExt = strFile.Right(4);
		if ( strExt.CmpNoCase(_T(".nwc")) == 0 )
			dwResult = ConvertNWC2MusicXML(strFile, bGenerateLog);
	}

	if ( !pCSConv )
	{
		delete pCSConv;
		pCSConv = NULL;
	}

#if defined(__WXMSW__) && defined(UNICODE)
#elif wxUSE_UNICODE
	{
		for ( int n = 0; n < argc; n++ )
			free(wxArgv[n]);

		delete [] wxArgv;
	}
#endif // wxUSE_UNICODE

	return 0;
}
