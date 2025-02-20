#include <windows.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <versionhelpers.h>
#include "resource.h"
#include <strsafe.h>
#include <Richedit.h>
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "dwmapi.lib")

// DPI API'leri için gerekli tanımlamalar
typedef UINT(WINAPI* PFNGETDPIFORWINDOW)(HWND);
typedef UINT(WINAPI* PFNGETDPIFORSYSTEM)(VOID);

// Global DPI fonksiyonları
PFNGETDPIFORWINDOW pfnGetDpiForWindow = nullptr;
PFNGETDPIFORSYSTEM pfnGetDpiForSystem = nullptr;

// Global değişkenler
#define ID_BUTTON_OK 1001
#define BS_MODERN_STYLE (BS_PUSHBUTTON | BS_OWNERDRAW)
HWND g_hwndButton = nullptr;
HINSTANCE g_hInst = nullptr;
HWND g_hwnd = nullptr;
bool g_isDarkMode = false;
HFONT g_hFont = nullptr;
HFONT g_hFontBig = nullptr;
HTHEME g_buttonTheme = nullptr;

// Global değişkenler kısmına ekle
LPCWSTR g_WindowsVersion = nullptr;
LPCWSTR g_CopyrightText = nullptr;
LPCWSTR g_MicrosoftWindows = nullptr;
LPCWSTR g_RegisteredOwner = nullptr;
LPCWSTR g_RegisteredOrg = nullptr;

// Global değişkenler kısmına eklenecek yeni değişkenler
LPCWSTR g_ProductName = nullptr;
LPCWSTR g_BuildInfo = nullptr;

// Global değişkenlere ekleyin
HBITMAP g_hLogo = nullptr;

// Global değişkenler kısmına ekleyin
LPCWSTR g_WindowsEdition = nullptr;

// Global değişkenler kısmına ekleyin
LPCWSTR g_LicenseText = nullptr;

// Global değişkenlere ekleyin
LPCWSTR g_WindowsNumber = nullptr;

// Global değişkenlere ekleyin
HFONT g_hFontUnderline = nullptr;
RECT g_LicenseLinkRect = {0}; // Lisans link alanını takip etmek için

// Global değişkenlere ekleyin
bool g_isTrackingMouse = false;

// Global değişkenlere ekleyin
HBITMAP g_memBitmap = nullptr;
HDC g_memDC = nullptr;

// Fonksiyon prototipi
void LoadAndScaleLogo(int dpi);

// DPI yardımcı fonksiyonları
UINT GetDPI(HWND hwnd) {
    if (pfnGetDpiForWindow) {
        return pfnGetDpiForWindow(hwnd);
    }
    return 300;
}

UINT GetSystemDPI() {
    if (pfnGetDpiForSystem) {
        return pfnGetDpiForSystem();
    }
    HDC hdc = GetDC(NULL);
    UINT dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(NULL, hdc);
    return dpi;
}

void InitDPIAPI() {
    HMODULE hUser32 = GetModuleHandle(TEXT("user32.dll"));
    if (hUser32) {
        pfnGetDpiForWindow = (PFNGETDPIFORWINDOW)GetProcAddress(hUser32, "GetDpiForWindow");
        pfnGetDpiForSystem = (PFNGETDPIFORSYSTEM)GetProcAddress(hUser32, "GetDpiForSystem");
    }
}

bool IsWindows11OrGreater() {
    OSVERSIONINFOEX osvi = { sizeof(OSVERSIONINFOEX) };
    DWORDLONG dwlConditionMask = 0;
    
    osvi.dwMajorVersion = 10;
    osvi.dwMinorVersion = 0;
    osvi.dwBuildNumber = 22000;

    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, VER_GREATER_EQUAL);

    return VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask);
}

void EnableDarkMode(HWND hwnd) {
    BOOL dark = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(32, 32, 32)));
}

void DisableMicaEffect(HWND hwnd) {
    DWM_SYSTEMBACKDROP_TYPE backdropType = DWMSBT_NONE;
    DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType, sizeof(backdropType));
    
    COLORREF captionColor = g_isDarkMode ? RGB(32, 32, 32) : RGB(255, 255, 255);
    DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &captionColor, sizeof(COLORREF));
}

void UpdateWindowTheme() {
    bool oldDarkMode = g_isDarkMode;
    
    HKEY hKey;
    DWORD value;
    DWORD size = sizeof(value);
    
    if (RegOpenKeyEx(HKEY_CURRENT_USER, 
        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"), 
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        
        if (RegQueryValueEx(hKey, TEXT("AppsUseLightTheme"), NULL, NULL, 
            (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            g_isDarkMode = (value == 0);
        }
        RegCloseKey(hKey);
    }

    // Tema değişti mi kontrol et
    if (oldDarkMode != g_isDarkMode) {
        if (g_isDarkMode) {
            EnableDarkMode(g_hwnd);
        } else {
            // Light mode için arka plan rengini güncelle
            SetClassLongPtr(g_hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(255, 255, 255)));
        }

        DisableMicaEffect(g_hwnd);
        
        // Logo'yu yeniden yükle
        LoadAndScaleLogo(GetDPI(g_hwnd));
        
        if (g_hwndButton) {
            InvalidateRect(g_hwndButton, NULL, TRUE);
            RedrawWindow(g_hwndButton, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
        }
        
        // Tüm pencereyi yeniden çiz
        InvalidateRect(g_hwnd, NULL, TRUE);
        UpdateWindow(g_hwnd);
    }
}

void CreateSegoeUIFont() {
    int dpi = GetDPI(g_hwnd);
    int fontSize = MulDiv(9, dpi, 72);
    g_hFont = CreateFont(
        -fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        TEXT("Segoe UI")
    );

    int fontSizeBig = MulDiv(36, dpi, 72);
    g_hFontBig = CreateFont(
        -fontSizeBig, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        TEXT("Segoe UI")
    );

    // Altı çizili font için
    g_hFontUnderline = CreateFont(
        -fontSize, 0, 0, 0, FW_NORMAL, FALSE, TRUE, 0,  // TRUE parametresi altı çizili yapar
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        TEXT("Segoe UI")
    );
}

// CreateSegoeUIFont fonksiyonundan sonra ekleyin
void LoadAndScaleLogo(int dpi) {
    if (g_hLogo) {
        DeleteObject(g_hLogo);
        g_hLogo = nullptr;
    }

    HICON hIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDB_LOGO),
        IMAGE_ICON, 
        MulDiv(48, dpi, 96),
        MulDiv(48, dpi, 96),
        LR_DEFAULTCOLOR);

    if (hIcon) {
        HDC hdc = GetDC(NULL);
        HDC hdcMem = CreateCompatibleDC(hdc);
        
        g_hLogo = CreateCompatibleBitmap(hdc, 
            MulDiv(48, dpi, 96), 
            MulDiv(48, dpi, 96));
            
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, g_hLogo);
        
        // Arka planı tema rengine uygun olarak doldur
        RECT rcFill = { 0, 0, MulDiv(48, dpi, 96), MulDiv(48, dpi, 96) };
        HBRUSH hBrush = CreateSolidBrush(g_isDarkMode ? RGB(32, 32, 32) : RGB(255, 255, 255));
        FillRect(hdcMem, &rcFill, hBrush);
        DeleteObject(hBrush);
        
        // Icon'u çiz
        DrawIconEx(hdcMem, 0, 0, 
            hIcon, 
            MulDiv(48, dpi, 96), 
            MulDiv(48, dpi, 96), 
            0, NULL, DI_NORMAL);
            
        SelectObject(hdcMem, hOldBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdc);
        DestroyIcon(hIcon);
    }
}

// GetWindowsVersionInfo fonksiyonunda değişiklik yapın
void GetWindowsVersionInfo() {
    HKEY hKey;
    WCHAR buffer[256];
    DWORD bufferSize = sizeof(buffer);
    DWORD type = REG_SZ;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        // Build numarasını al
        WCHAR buildNum[32];
        bufferSize = sizeof(buildNum);
        RegQueryValueEx(hKey, L"CurrentBuild", NULL, &type, (LPBYTE)buildNum, &bufferSize);
        
        // UBR değerini DWORD olarak al
        DWORD ubrValue = 0;
        bufferSize = sizeof(DWORD);
        RegQueryValueEx(hKey, L"UBR", NULL, &type, (LPBYTE)&ubrValue, &bufferSize);
        
        // UBR değerini string'e çevir
        WCHAR ubrNum[32];
        StringCchPrintf(ubrNum, 32, L"%d", ubrValue);

        // DisplayVersion'ı al (22H2 gibi)
        WCHAR displayVersion[32] = L"";
        bufferSize = sizeof(displayVersion);
        RegQueryValueEx(hKey, L"DisplayVersion", NULL, &type, (LPBYTE)displayVersion, &bufferSize);

        // Build bilgisini oluştur - formatı düzelttik
        WCHAR buildInfo[512];
        StringCchPrintf(buildInfo, 512, L"Sürüm %s (İS Derlemesi %s.%s)",
            displayVersion, buildNum, ubrNum);

        if (g_BuildInfo) free((void*)g_BuildInfo);
        g_BuildInfo = _wcsdup(buildInfo);

        // Önce varsayılan olarak Windows numarasını 10 olarak belirle
        g_WindowsNumber = _wcsdup(L"10");

        // Build numarasına göre Windows 11 kontrolü yap
        bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"CurrentBuild", NULL, &type,
            (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS) {
            
            int buildNum = _wtoi(buffer);
            if (buildNum >= 22000) {
                // Windows 11 ise güncelle
                if (g_WindowsNumber) free((void*)g_WindowsNumber);
                g_WindowsNumber = _wcsdup(L"11");
            }
        }

        // EditionID'yi al ve uygun formata çevir
        bufferSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, L"EditionID", NULL, &type,
            (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS) {
            
            WCHAR finalEdition[256];
            
            // Tam eşleşme için wcscmp kullanıyoruz, _wcsicmp yerine
            if (wcscmp(buffer, L"CoreSingleLanguage") == 0) {
                wcscpy_s(finalEdition, L"Home Single Language");
            }
            else if (wcscmp(buffer, L"IoTEnterpriseS") == 0) {
                wcscpy_s(finalEdition, L"IoT Enterprise LTSC");
            }
            else if (wcscmp(buffer, L"EnterpriseS") == 0) {
                wcscpy_s(finalEdition, L"Enterprise LTSC");
            }
            else if (wcscmp(buffer, L"Core") == 0) {
                wcscpy_s(finalEdition, L"Home");
            }
            else if (wcscmp(buffer, L"Professional") == 0) {
                wcscpy_s(finalEdition, L"Pro");
            }
            else if (wcscmp(buffer, L"Enterprise") == 0) {
                wcscpy_s(finalEdition, L"Enterprise");
            }
            else if (wcscmp(buffer, L"Education") == 0) {
                wcscpy_s(finalEdition, L"Education");
            }
            else {
                // Bilinmeyen Edition'lar için orijinal değeri koru
                wcscpy_s(finalEdition, buffer);
            }

            // Debug için Edition bilgilerini yazdır
            OutputDebugString(L"Original EditionID: ");
            OutputDebugString(buffer);
            OutputDebugString(L"\nConverted Edition: ");
            OutputDebugString(finalEdition);
            OutputDebugString(L"\n");

            // Tam Windows adını oluştur (Windows 11 Home Single Language gibi)
            WCHAR fullWindowsName[256];
            StringCchPrintf(fullWindowsName, 256, L"Windows %s %s", 
                g_WindowsNumber ? g_WindowsNumber : L"10", 
                finalEdition);

            // WindowsEdition'a tam adı kaydet
            if (g_WindowsEdition) free((void*)g_WindowsEdition);
            g_WindowsEdition = _wcsdup(fullWindowsName);
        }

        RegCloseKey(hKey);
    }

    // Kullanıcı bilgilerini al
    if (!g_RegisteredOwner || wcslen(g_RegisteredOwner) == 0) {
        DWORD size = 256;
        WCHAR username[256];
        if (GetUserName(username, &size)) {
            g_RegisteredOwner = _wcsdup(username);
        }
    }
}

void GetWindowsBranding() {
    HMODULE hModWinBrand = LoadLibraryEx(L"winbrand.dll", nullptr, 
        LOAD_LIBRARY_SEARCH_SYSTEM32);
    
    if (hModWinBrand) {
        typedef LPCWSTR (WINAPI* BrandingFormatString)(LPCWSTR);
        BrandingFormatString brandingFormat = (BrandingFormatString)
            GetProcAddress(hModWinBrand, "BrandingFormatString");
        
        if (brandingFormat) {
            g_CopyrightText = brandingFormat(L"%WINDOWS_COPYRIGHT%");
            g_MicrosoftWindows = brandingFormat(L"%MICROSOFT_COMPANYNAME% %WINDOWS_GENERIC%");
        }
        
        FreeLibrary(hModWinBrand);
    }
}

// DrawWindowsText fonksiyonunda değişiklikler
void DrawWindowsText(HDC hdc, const RECT& rc) {
    int dpi = GetDPI(g_hwnd);
    RECT rcText = rc;
    rcText.top = MulDiv(20, dpi, 96); // Üstten boşluğu artırdık

    HBRUSH hBrush = CreateSolidBrush(g_isDarkMode ? RGB(32, 32, 32) : RGB(255, 255, 255));
    FillRect(hdc, &rcText, hBrush);
    DeleteObject(hBrush);

    // Logo gösterimi
    if (g_hLogo) {
        BITMAP bm;
        GetObject(g_hLogo, sizeof(bm), &bm);
        
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, g_hLogo);
        
        // Logo boyutlarını ayarla
        int logoWidth = MulDiv(56, dpi, 96);
        int logoHeight = MulDiv(56, dpi, 96);
        
        // Logo ve başlık için toplam alan hesapla
        SIZE textSize;
        HFONT hOldFont = (HFONT)SelectObject(hdc, g_hFontBig);
        GetTextExtentPoint32(hdc, TEXT("Windows 11"), 10, &textSize);
        SelectObject(hdc, hOldFont);
        
        // Toplam genişlik (logo + boşluk + metin)
        int totalWidth = logoWidth + MulDiv(20, dpi, 96) + textSize.cx;
        
        // Logo'yu merkeze hizalı olarak yerleştir
        int startX = (rc.right - totalWidth) / 2;
        int logoY = rcText.top;
        
        // Logo'yu çiz
        StretchBlt(hdc, startX, logoY, logoWidth, logoHeight,
            hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
        
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);
        
        // Windows 11 yazısının konumunu ayarla
        rcText.left = startX + logoWidth + MulDiv(20, dpi, 96);
        rcText.top = logoY + (logoHeight - textSize.cy) / 2; // Dikey ortalama
    }

    // Windows başlığı - dinamik sürüm numarası
    WCHAR windowsTitle[256];
    StringCchPrintf(windowsTitle, 256, L"Windows %s", 
        g_WindowsNumber ? g_WindowsNumber : L"");

    HFONT hOldFont = (HFONT)SelectObject(hdc, g_hFontBig);
    SetTextColor(hdc, RGB(0, 120, 212));
    SetBkMode(hdc, TRANSPARENT);
    
    // Başlık metninin boyutunu al
    SIZE textSize;
    GetTextExtentPoint32(hdc, windowsTitle, wcslen(windowsTitle), &textSize);
    
    // Başlık için yeni bir RECT oluştur ve ortala
    RECT titleRect = rcText;
    titleRect.left = (rc.right - textSize.cx) / 2;
    
    // Başlığı çiz
    DrawText(hdc, windowsTitle, -1, &titleRect, DT_LEFT | DT_TOP | DT_SINGLELINE);

    // Separator çiz (logo altından başlasın)
    int separatorY = rcText.top + MulDiv(80, dpi, 96);
    HPEN hPen = CreatePen(PS_SOLID, 1, g_isDarkMode ? RGB(80, 80, 80) : RGB(200, 200, 200));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    MoveToEx(hdc, MulDiv(30, dpi, 96), separatorY, NULL);
    LineTo(hdc, rc.right - MulDiv(30, dpi, 96), separatorY);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    // Versiyon bilgileri için margin'i düzelt
    SelectObject(hdc, g_hFont);
    SetTextColor(hdc, g_isDarkMode ? RGB(255, 255, 255) : RGB(0, 0, 0));
    rcText.top = separatorY + MulDiv(20, dpi, 96);
    
    RECT rcParagraph = rcText;
    rcParagraph.left = MulDiv(40, dpi, 96);  // Sol margin'i düzelttik
    rcParagraph.right -= MulDiv(40, dpi, 96); // Sağ margin'i düzelttik
    
    if (g_MicrosoftWindows) {
        DrawText(hdc, g_MicrosoftWindows, -1, &rcParagraph, DT_LEFT | DT_TOP | DT_SINGLELINE);
    }
    
    rcParagraph.top += MulDiv(20, dpi, 96);
    if (g_BuildInfo) {
        DrawText(hdc, g_BuildInfo, -1, &rcParagraph, DT_LEFT | DT_TOP | DT_SINGLELINE);
    }
    
    rcParagraph.top += MulDiv(20, dpi, 96);
    if (g_CopyrightText) {
        DrawText(hdc, g_CopyrightText, -1, &rcParagraph, DT_LEFT | DT_TOP | DT_SINGLELINE);
    }

    // Lisans metni kısmı
    rcParagraph.top += MulDiv(40, dpi, 96);
    if (g_WindowsEdition) {
        // Dinamik lisans metni oluştur
        WCHAR firstPart[512];
        StringCchPrintf(firstPart, 512, 
            L"%s işletim sistemi ve bu sistemin\n"
            L"kullanıcı arabirimi, ABD ve diğer ülkelerde/bölgelerde yürürlükte olan\n"
            L"veya onay bekleyen ticari marka ve diğer fikri mülkiyet hakları tarafından\n"
            L"korunmaktadır.\n\n",
            g_WindowsEdition);

        DrawText(hdc, firstPart, -1, &rcParagraph, DT_LEFT | DT_TOP | DT_WORDBREAK);

        SIZE textSize;
        GetTextExtentPoint32(hdc, firstPart, wcslen(firstPart), &textSize);
        rcParagraph.top += textSize.cy + MulDiv(60, dpi, 96); // Paragraf sonrası ek boşluk ekledik

        // Tek satırlık RECT oluştur
        RECT singleLineRect = rcParagraph;

        // "Bu ürün" metnini çiz
        SetTextColor(hdc, g_isDarkMode ? RGB(255, 255, 255) : RGB(0, 0, 0));
        WCHAR buUrun[] = L"Bu ürün ";
        DrawText(hdc, buUrun, -1, &singleLineRect, DT_LEFT | DT_SINGLELINE);

        // "Bu ürün" metninin genişliğini hesapla
        SIZE buUrunSize;
        GetTextExtentPoint32(hdc, buUrun, wcslen(buUrun), &buUrunSize);

        // Link için RECT oluştur
        RECT linkRect = singleLineRect;
        linkRect.left += buUrunSize.cx;

        // Linki çiz
        HFONT hOldFont = (HFONT)SelectObject(hdc, g_hFontUnderline);
        SetTextColor(hdc, RGB(51, 153, 255));
        WCHAR linkText[] = L"Microsoft Yazılım Lisans Koşulları";
        DrawText(hdc, linkText, -1, &linkRect, DT_LEFT | DT_SINGLELINE);

        // Link alanını kaydet
        SIZE linkSize;
        GetTextExtentPoint32(hdc, linkText, wcslen(linkText), &linkSize);
        g_LicenseLinkRect = linkRect;
        g_LicenseLinkRect.right = g_LicenseLinkRect.left + linkSize.cx;
        g_LicenseLinkRect.bottom = g_LicenseLinkRect.top + linkSize.cy;

        // Kalan metni çiz
        SelectObject(hdc, g_hFont);
        SetTextColor(hdc, g_isDarkMode ? RGB(255, 255, 255) : RGB(0, 0, 0));
        linkRect.left += linkSize.cx;
        DrawText(hdc, L" kapsamında lisanslanmıştır.", -1, &linkRect, DT_LEFT | DT_SINGLELINE);
        
        SelectObject(hdc, hOldFont);
        
        rcParagraph.top += MulDiv(20, dpi, 96); // Sonraki metin için boşluk
    }

    // Sol margin'i artır
    rcParagraph.left += MulDiv(8, dpi, 96);
    
    // Kullanıcı bilgilerini göster
    if (g_RegisteredOwner && wcslen(g_RegisteredOwner) > 0) {
        WCHAR ownerText[256];
        StringCchPrintf(ownerText, 256, L"%s", g_RegisteredOwner);
        DrawText(hdc, ownerText, -1, &rcParagraph, DT_LEFT | DT_TOP | DT_SINGLELINE);
    }
    
    // Organizasyon bilgisini göster
    rcParagraph.top += MulDiv(20, dpi, 96);
    if (g_RegisteredOrg && wcslen(g_RegisteredOrg) > 0) {
        WCHAR orgText[256];
        StringCchPrintf(orgText, 256, L"%s", g_RegisteredOrg);
        DrawText(hdc, orgText, -1, &rcParagraph, DT_LEFT | DT_TOP | DT_SINGLELINE);
    }

    // Link ve kalan metinden sonraki boşluk
    rcParagraph.top += MulDiv(50, dpi, 96); // Yeterli boşluk bırak
    SelectObject(hdc, hOldFont);
}

// Yeni fonksiyon ekleyin
void ShowLicenseWindow(HWND hwndParent) {
    // Yeni pencere için sınıf kaydı
    WNDCLASSEX wcLicense = { sizeof(WNDCLASSEX) };
    wcLicense.lpfnWndProc = DefWindowProc;
    wcLicense.hInstance = g_hInst;
    wcLicense.lpszClassName = TEXT("WinverLicense");
    wcLicense.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassEx(&wcLicense);

    // Lisans penceresini oluştur - WS_THICKFRAME kaldırıldı
    HWND hwndLicense = CreateWindowEx(
        WS_EX_DLGMODALFRAME,
        TEXT("WinverLicense"),
        TEXT("Microsoft Yazılım Lisans Koşulları"),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, // Resize özelliğini kaldırdık
        CW_USEDEFAULT, CW_USEDEFAULT,
        1000, 650,
        hwndParent, NULL, g_hInst, NULL);

    if (hwndLicense) {

        // RichEdit kontrolünü oluştur
        LoadLibrary(TEXT("Msftedit.dll"));
        HWND hwndEdit = CreateWindowEx(0, MSFTEDIT_CLASS, TEXT(""),
            ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_READONLY,
            10, 10, 970, 590, // Yüksekliği biraz azalttık
            hwndLicense, NULL, g_hInst, NULL);

        if (hwndEdit) {
            // Dark/Light tema renklerini RichEdit'e uygula
            CHARFORMAT2 cf;
            ZeroMemory(&cf, sizeof(cf));
            cf.cbSize = sizeof(cf);
            cf.dwMask = CFM_COLOR | CFM_BACKCOLOR;
            cf.crTextColor = RGB(0, 0, 0);
            cf.crBackColor = RGB(255, 255, 255);
            SendMessage(hwndEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

            // Arka plan rengini ayarla
            SendMessage(hwndEdit, EM_SETBKGNDCOLOR, 0, RGB(255, 255, 255));

            // RTF dosyasını yükle
            WCHAR rtfPath[MAX_PATH];
            GetSystemDirectory(rtfPath, MAX_PATH);
            wcscat_s(rtfPath, MAX_PATH, L"\\license.rtf");
            
            // Dosyayı aç ve RichEdit'e yükle
            HANDLE hFile = CreateFile(rtfPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
                DWORD fileSize = GetFileSize(hFile, NULL);
                if (fileSize != INVALID_FILE_SIZE) {
                    char* buffer = new char[fileSize + 1];
                    DWORD bytesRead;
                    if (ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
                        buffer[fileSize] = 0;
                        SETTEXTEX text = { ST_DEFAULT, CP_ACP };
                        SendMessage(hwndEdit, EM_SETTEXTEX, (WPARAM)&text, (LPARAM)buffer);
                    }
                    delete[] buffer;
                }
                CloseHandle(hFile);
            }
        }

        ShowWindow(hwndLicense, SW_SHOW);
        UpdateWindow(hwndLicense);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
        {
            g_hwnd = hwnd;
            CreateSegoeUIFont();
            UpdateWindowTheme();
            DisableMicaEffect(hwnd);
            
            // Başlangıçta normal mouse imlecini zorla göster
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            
            int dpi = GetDPI(hwnd);
            g_hwndButton = CreateWindow(
                TEXT("BUTTON"), TEXT("Tamam"),
                WS_CHILD | WS_VISIBLE | BS_MODERN_STYLE,
                0, 0, MulDiv(75, dpi, 96), MulDiv(25, dpi, 96),
                hwnd, (HMENU)ID_BUTTON_OK, g_hInst, NULL);
            
            SendMessage(g_hwndButton, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            LoadAndScaleLogo(GetDPI(hwnd));

            // Double buffering için memory DC oluştur
            HDC hdc = GetDC(hwnd);
            g_memDC = CreateCompatibleDC(hdc);
            RECT rc;
            GetClientRect(hwnd, &rc);
            g_memBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            SelectObject(g_memDC, g_memBitmap);
            ReleaseDC(hwnd, hdc);
            
            return 0;
        }

        // Mouse el işaretçisi için yeni mesaj işleyici ekle
        case WM_SETCURSOR:
        {
            if (LOWORD(lParam) == HTCLIENT) {
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(hwnd, &pt);
                
                if (PtInRect(&g_LicenseLinkRect, pt)) {
                    SetCursor(LoadCursor(NULL, IDC_HAND));
                    return TRUE;
                }
                SetCursor(LoadCursor(NULL, IDC_ARROW));
                return TRUE;
            }
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }

        case WM_MOUSEMOVE:
        {
            POINT pt = { LOWORD(lParam), HIWORD(lParam) };
            static bool wasInLink = false;
            bool isInLink = PtInRect(&g_LicenseLinkRect, pt);
            
            // Mouse takibini başlat
            if (!g_isTrackingMouse) {
                TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hwnd;
                TrackMouseEvent(&tme);
                g_isTrackingMouse = true;
            }

            if (wasInLink != isInLink) {
                wasInLink = isInLink;
                // Sadece link alanını güncelle
                InvalidateRect(hwnd, &g_LicenseLinkRect, FALSE);
                UpdateWindow(hwnd);
            }
            return 0;
        }

        // Yeni mesaj işleyicisi ekleyin
        case WM_MOUSELEAVE:
        {
            g_isTrackingMouse = false;
            // Mouse pencereden ayrıldığında normal imlece dön
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
            return 0;
        }

        case WM_SIZE:
        {
            RECT rc;
            GetClientRect(hwnd, &rc);
            int dpi = GetDPI(hwnd);
            
            // Memory DC'yi yeniden oluştur
            if (g_memBitmap) DeleteObject(g_memBitmap);
            HDC hdc = GetDC(hwnd);
            g_memBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            SelectObject(g_memDC, g_memBitmap);
            ReleaseDC(hwnd, hdc);
            
            // Buton pozisyonunu ayarla
            int btnWidth = MulDiv(75, dpi, 96);
            int btnHeight = MulDiv(25, dpi, 96);
            int margin = MulDiv(10, dpi, 96);
            
            SetWindowPos(g_hwndButton, NULL,
                rc.right - btnWidth - margin,
                rc.bottom - btnHeight - MulDiv(15, dpi, 96),
                btnWidth, btnHeight,
                SWP_NOZORDER);
                
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rc;
            GetClientRect(hwnd, &rc);

            // Memory DC'ye çiz
            HBRUSH hBrush = CreateSolidBrush(g_isDarkMode ? RGB(32, 32, 32) : RGB(255, 255, 255));
            FillRect(g_memDC, &rc, hBrush);
            DeleteObject(hBrush);

            // Tüm içeriği memory DC'ye çiz
            DrawWindowsText(g_memDC, rc);

            // Memory DC'den ekrana kopyala
            BitBlt(hdc, 0, 0, rc.right, rc.bottom, g_memDC, 0, 0, SRCCOPY);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_SETTINGCHANGE:
        {
            if (lParam && (lstrcmp((LPTSTR)lParam, L"ImmersiveColorSet") == 0 ||
                          lstrcmp((LPTSTR)lParam, L"SystemColors") == 0)) {
                UpdateWindowTheme();
                return 0;
            }
            break;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_BUTTON_OK) {
                DestroyWindow(hwnd);
                return 0;
            }
            break;

        case WM_DRAWITEM:
{
    DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
    if (dis->hwndItem == g_hwndButton) {
        RECT rc = dis->rcItem;
        HDC hdc = dis->hDC;
        bool isHot = (dis->itemState & ODS_HOTLIGHT) || (dis->itemState & ODS_SELECTED);
        bool isPressed = dis->itemState & ODS_SELECTED;
        
        // Arkaplan rengi
        COLORREF bgColor;
        if (g_isDarkMode) {
            if (isPressed)
                bgColor = RGB(50, 50, 50);
            else if (isHot)
                bgColor = RGB(70, 70, 70);
            else
                bgColor = RGB(36, 36, 36);
        } else {
            if (isPressed)
                bgColor = RGB(222, 222, 222);
            else if (isHot)
                bgColor = RGB(246, 246, 246);
            else
                bgColor = RGB(255, 255, 255);
        }
        
        // Kenarlık rengi - biraz daha ince kenarlık için rengi ayarla
        COLORREF borderColor = g_isDarkMode ? RGB(70, 70, 70) : RGB(210, 210, 210);
        
        // Double buffering için memory DC oluştur
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
        
        // Önce arka planı temizle
        HBRUSH hBackBrush = CreateSolidBrush(g_isDarkMode ? RGB(32, 32, 32) : RGB(255, 255, 255));
        FillRect(memDC, &rc, hBackBrush);
        DeleteObject(hBackBrush);
        
        // Buton için rect
        RECT buttonRect = {0, 0, rc.right - rc.left, rc.bottom - rc.top};
        
        // Arkaplanı doldur
        HBRUSH hBrush = CreateSolidBrush(bgColor);
        HPEN hPen = CreatePen(PS_SOLID, 1, borderColor);
        HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, hBrush);
        HPEN oldPen = (HPEN)SelectObject(memDC, hPen);
        
        // Radius değerini 8'e çıkardık
        RoundRect(memDC, buttonRect.left, buttonRect.top, 
                 buttonRect.right, buttonRect.bottom, 8, 8);
        
        // Metni çiz
        TCHAR text[256];
        GetWindowText(dis->hwndItem, text, 256);
        SetTextColor(memDC, g_isDarkMode ? RGB(255, 255, 255) : RGB(0, 0, 0));
        SetBkMode(memDC, TRANSPARENT);
        HFONT hOldFont = (HFONT)SelectObject(memDC, g_hFont);
        DrawText(memDC, text, -1, &buttonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // Cleanup
        SelectObject(memDC, oldBrush);
        SelectObject(memDC, oldPen);
        SelectObject(memDC, hOldFont);
        DeleteObject(hBrush);
        DeleteObject(hPen);
        
        // Memory DC'den gerçek DC'ye kopyala
        BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, memDC, 0, 0, SRCCOPY);
        
        // Memory DC cleanup
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);
        
        return TRUE;
    }
    break;
}


        case WM_LBUTTONDOWN:
        {
            POINT pt = { LOWORD(lParam), HIWORD(lParam) };
            if (PtInRect(&g_LicenseLinkRect, pt)) {
                ShowLicenseWindow(hwnd);
                return 0;
            }
            break;
        }

        // WM_DESTROY içinde bellek temizliği
        case WM_DESTROY:
        {
            if (g_buttonTheme) CloseThemeData(g_buttonTheme);
            if (g_hFont) DeleteObject(g_hFont);
            if (g_hFontBig) DeleteObject(g_hFontBig);
            if (g_WindowsVersion) free((void*)g_WindowsVersion);
            if (g_CopyrightText) free((void*)g_CopyrightText);
            if (g_MicrosoftWindows) free((void*)g_MicrosoftWindows);
            if (g_RegisteredOwner) free((void*)g_RegisteredOwner);
            if (g_RegisteredOrg) free((void*)g_RegisteredOrg);
            if (g_ProductName) free((void*)g_ProductName);
            if (g_BuildInfo) free((void*)g_BuildInfo);
            if (g_WindowsEdition) free((void*)g_WindowsEdition);
            if (g_LicenseText) free((void*)g_LicenseText);
            if (g_WindowsNumber) free((void*)g_WindowsNumber);
            if (g_hLogo) DeleteObject(g_hLogo);
            if (g_hFontUnderline) DeleteObject(g_hFontUnderline);
            
            // Memory DC temizliği
            if (g_memBitmap) DeleteObject(g_memBitmap);
            if (g_memDC) DeleteDC(g_memDC);
            
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    LPSTR lpCmdLine, int nCmdShow) {
    
    // DPI ve versiyon bilgilerini önceden yükle
    InitDPIAPI();
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    g_hInst = hInstance;

    // Tüm bilgileri ana pencere oluşturulmadan önce yükle
    GetWindowsVersionInfo();
    GetWindowsBranding();
    
    // Logo'yu önceden yükle
    LoadAndScaleLogo(GetSystemDPI());

    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("Winver");
    wc.hbrBackground = nullptr;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    // İkon kullanımını kaldırdık
    wc.hIcon = nullptr;
    wc.hIconSm = nullptr;
    
    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, TEXT("Pencere sınıfı kaydedilemedi!"), TEXT("Hata"), MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowEx(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        TEXT("Winver"),
        TEXT("Windows Hakkında"),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT,
        MulDiv(480, GetSystemDPI(), 96),
        MulDiv(440, GetSystemDPI(), 96),
        NULL, NULL, hInstance, NULL);

    if (hwnd) {
        // Pencere oluşturulur oluşturulmaz normal imleci göster
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        ShowWindow(hwnd, nCmdShow);
        UpdateWindow(hwnd);
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
