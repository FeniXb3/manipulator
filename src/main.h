#define _WIN32_IE 0x0600
#include <windows.h>
#include <commctrl.h>   //konieczne dla uzycia kontrolki ListView
#include <psapi.h>

#ifndef TTS_BALLOON
#define TTS_BALLOON 0x40    //definicja stałej, odpowiadającej za "balonowy" styl tooltipów
#endif
#ifndef TTM_SETTITLE
#define TTM_SETTITLE (WM_USER + 32)
#endif

#ifndef TTI_INFO
#define TTI_INFO 0x01
#endif

#define WM_TRAY (WM_USER+1) //komunikat dotyczący zasobnika systemowego

#define PXICON 300
