#define PCH_H
// add headers that you want to pre-compile here
#include <Windows.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <ctime>
#include <ctime>
#include <string.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <conio.h>
#include <array>
#include <map>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <tchar.h>
#include <inttypes.h>



#include "mem.h"
#include "util.h"

#include "Singleton.h"
#include "structs.h"
#include "PacketSend.h"
#include "MemReader.h"
#include "AutoAttack.h"

#pragma comment(lib, "winmm.lib")

#pragma comment(linker,"/manifestdependency:\"type='win32' "\
                   "name='Microsoft.Windows.Common-Controls' "\
                   "version='6.0.0.0' "\
                   "processorArchitecture='x86' "\
                   "publicKeyToken='6595b64144ccf1df' "\
                   "language='*' "\
                   "\"")