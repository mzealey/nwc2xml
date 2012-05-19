// nwc2xml.cpp : Defines the entry point for the console application.
//

#include "Precompile.h"
#include "nwcfile.h"
#include "XMLSaver.h"
#include "wx/cmdline.h"

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


static wxChar* s_ProgramLogo =
_T("nwc2xml v1.5 Copyright (C) 2005-2012 james lee (juria90@yahoo.com)\n")
_T("nwc2xml converts <file.nwc> file to musicxml file <file.xml>.\n")
_T("This program supports upto nwc v2.0.\n");

#if defined(__WXMSW__) && defined(UNICODE)
int wmain(int argc, wchar_t *argv[])
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
		{ wxCMD_LINE_SWITCH, _T("h"), _T("help"),	_T("show this help message"),
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
		{ wxCMD_LINE_SWITCH, _T("l"), _T("log"),	_T("generate log file") },

		{ wxCMD_LINE_OPTION, NULL, _T("charset"),	_T("conversion charset for lyric; default is system setting"),
		wxCMD_LINE_VAL_NUMBER },

		{ wxCMD_LINE_PARAM,  NULL, NULL, _T("input file"),
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
