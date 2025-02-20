#pragma once
#include <Windows.h>
#include <Uxtheme.h>
#include <vsstyle.h>

// UAH = "User Account Hub" menü çizim yapıları
typedef struct _UAHMENUPOPUPMETRICS {
    BOOL    fLeftAligned;
    int     iMaxHeight;
    int     iXOffset;
    int     iYOffset;
    int     iPopupDelay;
    int     iVMargin;
    int     iHMargin;
    int     iHThickness;
    int     iVThickness;
    int     iIndent;
} UAHMENUPOPUPMETRICS, *PUAHMENUPOPUPMETRICS;

typedef struct _UAHMENU {
    HMENU hmenu;
    HDC hdc;
    DWORD dwFlags;
} UAHMENU, *PUAHMENU;

typedef struct _UAHMENUITEM {
    int iPosition;
    UAHMENUPOPUPMETRICS umpm;
} UAHMENUITEM, *PUAHMENUITEM;

typedef struct _UAHDRAWMENUITEM {
    DRAWITEMSTRUCT dis;
    UAHMENU um;
    UAHMENUITEM umi;
} UAHDRAWMENUITEM, *PUAHDRAWMENUITEM;

typedef struct _UAHMEASUREMENUITEM {
    MEASUREITEMSTRUCT mis;
    UAHMENU um;
    UAHMENUITEM umi;
} UAHMEASUREMENUITEM, *PUAHMEASUREMENUITEM;

#define WM_UAHDRAWMENU        0x0091
#define WM_UAHDRAWMENUITEM    0x0092
#define WM_UAHMEASUREMENUITEM 0x0093

bool UAHWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT* lr);
