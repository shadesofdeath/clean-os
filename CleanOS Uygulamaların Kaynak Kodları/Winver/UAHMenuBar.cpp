#include "UAHMenuBar.h"

static HTHEME g_menuTheme = nullptr;
static HBRUSH g_brBarBackground = CreateSolidBrush(RGB(32, 32, 32));

void UAHDrawMenuNCBottomLine(HWND hWnd)
{
    MENUBARINFO mbi = { sizeof(mbi) };
    if (!GetMenuBarInfo(hWnd, OBJID_MENU, 0, &mbi))
    {
        return;
    }

    RECT rcClient = { 0 };
    GetClientRect(hWnd, &rcClient);
    MapWindowPoints(hWnd, nullptr, (POINT*)&rcClient, 2);

    RECT rcWindow = { 0 };
    GetWindowRect(hWnd, &rcWindow);

    OffsetRect(&rcClient, -rcWindow.left, -rcWindow.top);

    RECT rcAnnoyingLine = rcClient;
    rcAnnoyingLine.bottom = rcAnnoyingLine.top;
    rcAnnoyingLine.top--;

    HDC hdc = GetWindowDC(hWnd);
    FillRect(hdc, &rcAnnoyingLine, g_brBarBackground);
    ReleaseDC(hWnd, hdc);
}

bool UAHWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT* lr)
{
    switch (message)
    {
        case WM_UAHDRAWMENU:
        {
            UAHMENU* pUDM = (UAHMENU*)lParam;
            RECT rc = { 0 };

            {
                MENUBARINFO mbi = { sizeof(mbi) };
                GetMenuBarInfo(hWnd, OBJID_MENU, 0, &mbi);

                RECT rcWindow;
                GetWindowRect(hWnd, &rcWindow);

                rc = mbi.rcBar;
                OffsetRect(&rc, -rcWindow.left, -rcWindow.top);
            }

            FillRect(pUDM->hdc, &rc, g_brBarBackground);
            return true;
        }

        case WM_UAHDRAWMENUITEM:
        {
            UAHDRAWMENUITEM* pUDMI = (UAHDRAWMENUITEM*)lParam;
            static HBRUSH g_brItemBackground = CreateSolidBrush(RGB(32, 32, 32));
            static HBRUSH g_brItemBackgroundHot = CreateSolidBrush(RGB(70, 70, 70));
            static HBRUSH g_brItemBackgroundSelected = CreateSolidBrush(RGB(90, 90, 90));

            HBRUSH* pbrBackground = &g_brItemBackground;

            wchar_t menuString[256] = { 0 };
            MENUITEMINFO mii = { sizeof(mii), MIIM_STRING };
            {
                mii.dwTypeData = menuString;
                mii.cch = (sizeof(menuString) / 2) - 1;

                GetMenuItemInfo(pUDMI->um.hmenu, pUDMI->umi.iPosition, TRUE, &mii);
            }

            DWORD dwFlags = DT_CENTER | DT_SINGLELINE | DT_VCENTER;

            int iTextStateID = 0;
            int iBackgroundStateID = 0;
            {
                if ((pUDMI->dis.itemState & ODS_INACTIVE) || (pUDMI->dis.itemState & ODS_DEFAULT)) {
                    iTextStateID = MBI_NORMAL;
                    iBackgroundStateID = MBI_NORMAL;
                }
                if (pUDMI->dis.itemState & ODS_HOTLIGHT) {
                    iTextStateID = MBI_HOT;
                    iBackgroundStateID = MBI_HOT;
                    pbrBackground = &g_brItemBackgroundHot;
                }
                if (pUDMI->dis.itemState & ODS_SELECTED) {
                    iTextStateID = MBI_PUSHED;
                    iBackgroundStateID = MBI_PUSHED;
                    pbrBackground = &g_brItemBackgroundSelected;
                }
                if ((pUDMI->dis.itemState & ODS_GRAYED) || (pUDMI->dis.itemState & ODS_DISABLED)) {
                    iTextStateID = MBI_DISABLED;
                    iBackgroundStateID = MBI_DISABLED;
                }
                if (pUDMI->dis.itemState & ODS_NOACCEL) {
                    dwFlags |= DT_HIDEPREFIX;
                }
            }

            if (!g_menuTheme) {
                g_menuTheme = OpenThemeData(hWnd, L"Menu");
            }

            DTTOPTS opts = { sizeof(opts), DTT_TEXTCOLOR, RGB(255, 255, 255) };

            FillRect(pUDMI->um.hdc, &pUDMI->dis.rcItem, *pbrBackground);
            DrawThemeTextEx(g_menuTheme, pUDMI->um.hdc, MENU_BARITEM, iTextStateID, 
                menuString, mii.cch, dwFlags, &pUDMI->dis.rcItem, &opts);

            return true;
        }

        case WM_THEMECHANGED:
        {
            if (g_menuTheme) {
                CloseThemeData(g_menuTheme);
                g_menuTheme = nullptr;
            }
            return false;
        }

        case WM_NCPAINT:
        case WM_NCACTIVATE:
            *lr = DefWindowProc(hWnd, message, wParam, lParam);
            UAHDrawMenuNCBottomLine(hWnd);
            return true;

        default:
            return false;
    }
}
