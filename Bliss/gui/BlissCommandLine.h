#pragma once

class BlissCommandLine : public CCommandLineInfo
{
public:
    BlissCommandLine();
    virtual void ParseParam(const char* pszParam, BOOL bFlag, BOOL bLast);

    BOOL m_bFullScreen;

};

