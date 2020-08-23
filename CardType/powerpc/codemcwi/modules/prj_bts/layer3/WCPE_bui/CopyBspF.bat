if not exist %WIND_BASE%\target\config\all md %WIND_BASE%\target\config\all
@attrib -r %WIND_BASE%\target\config\all\*.*
@copy  ..\..\bts_bsp\all\*.*  %WIND_BASE%\target\config\all\*.*

if not exist %WIND_BASE%\target\config\L3_bootrom md %WIND_BASE%\target\config\L3_bootrom
@attrib -r %WIND_BASE%\target\config\L3_bootrom\*.*
@copy  ..\..\bts_bsp\L3_bootrom\*.*  %WIND_BASE%\target\config\L3_bootrom\*.*

if not exist %WIND_BASE%\target\config\Mcwill_L3 md %WIND_BASE%\target\config\Mcwill_L3
@attrib -r %WIND_BASE%\target\config\Mcwill_L3\*.*
@copy  ..\..\bts_bsp\Mcwill_L3\*.*  %WIND_BASE%\target\config\Mcwill_L3\*.*

if not exist %WIND_BASE%\target\lib\ppc\PPC604\priGnu md %WIND_BASE%\target\lib\ppc\PPC604\priGnu
@attrib -r %WIND_BASE%\target\lib\ppc\PPC604\priGnu\*.*
@copy  ..\..\bts_bsp\lib\ppc\ppc604\priGnu\*.*  %WIND_BASE%\target\lib\ppc\PPC604\priGnu\*.*

if not exist %WIND_BASE%\target\config\comps\src md %WIND_BASE%\target\config\comps\src
@attrib -r %WIND_BASE%\target\config\comps\src\wdbEnd.c
@copy  ..\..\bts_bsp\src\wdbEnd.c  %WIND_BASE%\target\config\comps\src\wdbEnd.c
@copy  ..\..\bts_bsp\src\usrNetTelnetdCfg.c  %WIND_BASE%\target\config\comps\src\net\usrNetTelnetdCfg.c
