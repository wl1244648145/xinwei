#ifndef _INC_DEBUGLEVEL
#define _INC_DEBUGLEVEL

#include "LogArea.h"

#ifndef _INC_LOG
#include "Log.h"
#endif

#define AREA_NAME_LEN 8
#define SHELL_NAME_LEN 8

class CDebugLevel
{
public:
    CDebugLevel(LOGLEVEL level, const char *lpszAreaName, const char *shellName); 
     
    void SetDebugLevel(LOGLEVEL lvlNew);
    inline LOGLEVEL GetDebugLevel(void) {return m_lvlLevel;}
    void Show();

private:
    LOGLEVEL m_lvlLevel;
    char m_szName[AREA_NAME_LEN+1];
    char m_shellName[SHELL_NAME_LEN+1];
};




#endif //__DEBUG_LEVEL_H
