
#include "stdafx.h"
#include "BlissCommandLine.h"

BlissCommandLine::BlissCommandLine()
: m_bFullScreen(FALSE)
{}
void BlissCommandLine::ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast)
{
    if (bFlag && strcmpi(pszParam, "fullscreen") == 0) {
        m_bFullScreen = TRUE;
    }
    else
        CCommandLineInfo::ParseParam(pszParam, bFlag, bLast);
}
