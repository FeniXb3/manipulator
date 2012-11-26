/*
    Konrad Gadzina <fenix.b3@gmail.com>
    Grupa 1131, Informatyka, rok akad. 2008/2009
    Pracownia In¿ynierii Oprogramowania M-74
    Instytut Informatyki Stosowanej
    Wydzia³ Mechaniczny, Politechnika Krakowska
    Jêzyki i Techniki Programowania
*/
#include "main.h" //tutaj zawarte s¹ wszelkie potrzebne includy i definicje

char szClassName[] = "Manipulator_v.1";   //nazwa klasy g³ównego okna jako zmienna globalna

HWND hList, hPasek = NULL, hRefresh, hInfo, hShow;
HINSTANCE hInst;
HANDLE proces;  //uchwyt na proces explorera
LVITEM item;    //zmienna do przechowywania danych danego elementu listy
int ile;    //zmienna do przechowuj¹ca ilosc przycisków paska zadañ
HICON ikona;    //zmienna na uchwyt ikony programu
HIMAGELIST imgtmp;  //tymczasowa lista obrazków - potrzebna przy przesuwaniu grupy przycisków


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);   //deklaracja funkcji obs³ugi komunikatów  okna g³ównego
/*--------------------------*/
BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
{
    const int MAX = 256;
    char klasa[MAX];

    GetClassName(hwnd, klasa, MAX);

    if (!strcmp(klasa, "ToolbarWindow32"))
    {
        HWND hParent = GetParent(hwnd);
        GetClassName(hParent, klasa, MAX);
        if (!strcmp(klasa, "MSTaskSwWClass"))
        {
            GetWindowText(hwnd, (char*)lParam, MAX);
            hPasek = hwnd;
            return false;
        }
    }

    return true;
}
/*--------------------------------*/
void refresh()  //odswiezanie widoku ListView
{
    LVCOLUMN col;   //struktura kolumny w kontrolce ListView
    HIMAGELIST hImgList;    //lista obrazków, wykorzystana potem w ListView
    //RECT rect;
    DWORD pid;
    TBBUTTON tbb, *ptbb, tmp;
    char txt[256], *ptxt, path[512];
    HWND hOkno;
    HICON hIcon = NULL;
    HANDLE hproc;

    SetCursor(LoadCursor(NULL, IDC_WAIT));  //zmiana kursora na klepsydrê, by u¿ytkownik
                                            //wiedzia³, ¿e coœ siê dzieje
    if (!hPasek) //je¿eli nie znany jest jeszcze uchwyt paska zadañ
    {
        hPasek = FindWindowEx(NULL, NULL, "Shell_TrayWnd", NULL);
        EnumChildWindows(hPasek, EnumChildProc, (LPARAM)txt);

        col.pszText = txt;
        col.cchTextMax = strlen(txt);

        GetWindowThreadProcessId(hPasek, &pid); //pobieranie ID procesu explorera
        proces = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ, false, pid);   //otwieranie procesu explorera
    }
    else    //je¿eli mamy ju¿ uchwyt paska zadañ
    {
        col.mask = LVCF_TEXT;
        col.pszText = txt;      //tytu³ kolumny
        col.cchTextMax = 256;   //maksymalny rozmiar tytu³u kolumny
        ListView_GetColumn(hList, 0, &col);

        ListView_DeleteAllItems(hList);
        ListView_DeleteColumn(hList, 0);
        ListView_DeleteColumn(hList, 0);
    }
    /*---alokowanie pamiêci dla przycisku i jego nazwy---*/
    ptbb = (TBBUTTON*)VirtualAllocEx(proces, NULL, sizeof(TBBUTTON), MEM_COMMIT, PAGE_READWRITE);
    ptxt = (char*)VirtualAllocEx(proces, NULL, 256, MEM_COMMIT, PAGE_READWRITE);

    ile = SendMessage(hPasek, TB_BUTTONCOUNT, 0, 0);
    hImgList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, ile-1, 0);
    ListView_SetImageList(hList, hImgList, LVSIL_SMALL);

    item.mask = LVIF_STATE | LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM | LVIF_TEXT;
    item.iItem = 0;
    item.iSubItem = 0;

    col.cx = 400;
    col.mask = LVCF_TEXT | LVCF_WIDTH;
    col.iSubItem = 0;
    ListView_InsertColumn(hList, 0, &col);
    strcpy(txt, "Stan");
    col.iSubItem = 1;
    col.cx = 70;
    ListView_InsertColumn(hList, 1, &col);

    for (int i = 0; i < ile; i++)
    {
        SendMessage(hPasek, TB_GETBUTTON, (WPARAM)i, (LPARAM)ptbb);
        ReadProcessMemory(proces, (void*)ptbb, (void*)&tbb, sizeof(TBBUTTON), NULL);

        SendMessage(hPasek, TB_GETBUTTONTEXT, (WPARAM)tbb.idCommand, (LPARAM)ptxt);
        ReadProcessMemory(proces, (void*)ptxt, (void*)txt, 256, NULL);

        item.pszText = txt;
        item.cchTextMax = 256;
        item.iItem = i;
        item.iSubItem = 0;

        hIcon = NULL;
        if (tbb.fsStyle & BTNS_WHOLEDROPDOWN)   //je¿eli przycisk jest przyciskiem grupowym
        {
            SendMessage(hPasek, TB_GETBUTTON, (WPARAM)(i+1), (LPARAM)ptbb); //i+1, by pobraæ ikonê przycisku tej grupy
            ReadProcessMemory(proces, (void*)ptbb, (void*)&tmp, sizeof(TBBUTTON), NULL);
            ReadProcessMemory(proces, (void*)tmp.dwData, (void*)&hOkno, sizeof(HWND), NULL);

            if (!hIcon)
            {
                GetWindowThreadProcessId(hOkno, &pid);
                hproc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);
                GetModuleFileNameEx(hproc, NULL, path, 512);
                ExtractIconEx(path, 0, NULL, &hIcon, 1);
                CloseHandle(hproc);
            }
            if (!hIcon)
                hIcon = LoadIcon(NULL, IDI_APPLICATION);

            item.iImage = ImageList_AddIcon(hImgList, hIcon);
            item.iIndent = 0;
        }
        else
        {
            //w tbb.dwData umieszczony jest uchwyt do okna skojarzonego z przyciskiem
            ReadProcessMemory(proces, (void*)tbb.dwData, (void*)&hOkno, sizeof(HWND), NULL);


            if (!hIcon)
                hIcon = (HICON)SendMessage(hOkno, WM_GETICON, ICON_SMALL, 0);
            if (!hIcon)
                hIcon = (HICON)SendMessage(hOkno, WM_GETICON, ICON_BIG, 0);
            if (!hIcon)
                hIcon = (HICON)GetClassLong(hOkno, GCL_HICONSM);
            if (!hIcon)
                hIcon = (HICON)GetClassLong(hOkno, GCL_HICON);
            if (!hIcon)
                hIcon = (HICON)GetClassLongPtr(hOkno, GCLP_HICONSM);
            if (!hIcon)
                hIcon = (HICON)GetClassLongPtr(hOkno, GCLP_HICON);
            if (!hIcon)
            {
                GetWindowThreadProcessId(hOkno, &pid);
                hproc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);
                GetModuleFileNameEx(hproc, NULL, path, 512);
                ExtractIconEx(path, 0, NULL, &hIcon, 1);
                CloseHandle(hproc);
            }
            if (!hIcon)
                hIcon = LoadIcon(NULL, IDI_APPLICATION);


            item.iImage = ImageList_AddIcon(hImgList, hIcon);
            item.iIndent = 1;
            item.lParam = (LPARAM)hOkno;
        }
        ListView_InsertItem(hList,&item);

        if (tbb.fsState & TBSTATE_HIDDEN)
            strcpy(txt, "[ukryty]");
        else
            strcpy(txt, "[widoczny]");
        ListView_SetItemText(hList, i, 1, txt);
    }

    VirtualFreeEx(proces, ptbb, 0, MEM_RELEASE);
    VirtualFreeEx(proces, ptxt, 0, MEM_RELEASE);
}
/*********************************************************/

/*------------------------*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    HWND hMain;

    MSG komunikaty;
    WNDCLASSEX wincl;
    INITCOMMONCONTROLSEX icc;

    ikona = LoadIcon(hInstance, MAKEINTRESOURCE(PXICON));

    wincl.hInstance = hInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WndProc;
    wincl.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hIcon = ikona;
    wincl.hIconSm = ikona;
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.hbrBackground = (HBRUSH) (COLOR_BTNFACE+1);
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;

    if (!RegisterClassEx(&wincl))
        return 0;

    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);
    hInst = hInstance;

    hMain = CreateWindowEx(0, szClassName, "Manipulator", WS_CAPTION | WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, 500, 287, HWND_DESKTOP, NULL, hInstance, NULL);

    //je¿eli uruchomimy program z parametrem -hide, to automatycznie siê on zminimalizuje do zasobnika
    if (strcmp(lpCmdLine, "-hide"))
        ShowWindow(hMain, nShowCmd);

    while (GetMessage(&komunikaty, NULL, 0, 0)) //pêtla obs³ugi komunikatów
    {
        TranslateMessage(&komunikaty);
        DispatchMessage(&komunikaty);
    }

    return komunikaty.wParam;
}
/*-------------------------------*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT komunikat, WPARAM wParam, LPARAM lParam)
{
    static BOOL bDragging;
    static int poz;
    POINT pkt;
    HIMAGELIST hDragImgList;
    static NOTIFYICONDATA nim;

    switch (komunikat)
    {
        case WM_CREATE:
            HWND hToolTip;
            HFONT hFont;
            TOOLINFO tinf;
            RECT rect;

            /*-----tworzenie kontrolek-----*/
            hList = CreateWindow("SysListView32", "lista_przyciskow", WS_VISIBLE | WS_CHILD | WS_BORDER |
                                 LVS_REPORT | LVS_SINGLESEL,0,0,493,230,hWnd,NULL,hInst, NULL);
            hRefresh = CreateWindow("button", "Odœwie¿ listê", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                    135, 232, 100, 20, hWnd, NULL, hInst, NULL);
            hShow = CreateWindow("button", "Poka¿ przycisk", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                 15, 232, 100, 20,/*255, 232, 100, 20,*/ hWnd, NULL, hInst, NULL);
            hInfo = CreateWindow("button", "O programie...", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                 375, 232, 100, 20, hWnd, NULL, hInst, NULL);

            /*-----tworzenie i ustawianie czcionki dla przycisków-----*/
            hFont = CreateFont(14, 0, 0, 0, 0, false, false, false, DEFAULT_CHARSET,
                               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                               DEFAULT_PITCH, NULL);

            SendMessage(hRefresh, WM_SETFONT, (WPARAM) hFont, true);
            SendMessage(hShow, WM_SETFONT, (WPARAM) hFont, true);
            SendMessage(hInfo, WM_SETFONT, (WPARAM) hFont, true);

            ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);

            /*-----ustawianie tooltipów-----*/
            hToolTip = CreateWindow(TOOLTIPS_CLASS, NULL, TTS_ALWAYSTIP | TTS_BALLOON, 0,0,0,0, hWnd, NULL, hInst, NULL);

            //Ustawianie parametrów wspó³nych dla ka¿dego tooltipa: tytu³, kolor.
            SendMessage(hToolTip, TTM_SETTITLE, TTI_INFO, (LPARAM)"Skrót");
            SendMessage(hToolTip, TTM_SETTIPTEXTCOLOR, (LPARAM)RGB(0, 0, 100), 0);
            SendMessage(hToolTip, TTM_SETTIPBKCOLOR, (LPARAM)RGB(250, 250, 250), 0);

            GetClientRect (hWnd, &rect);
            tinf.cbSize = sizeof(TOOLINFO);
            tinf.uFlags = TTF_SUBCLASS;
            tinf.hinst = hInst;
            tinf.uId = 0;
            tinf.rect.left = rect.left;
            tinf.rect.top = rect.top;
            tinf.rect.right = rect.right;
            tinf.rect.bottom = rect.bottom;

            tinf.hwnd = hRefresh;
            tinf.lpszText = "F5";
            SendMessage(hToolTip, TTM_ADDTOOL, 0, (LPARAM)&tinf);
            tinf.hwnd = hShow;
            tinf.lpszText = "dwuklik LPM";
            SendMessage(hToolTip, TTM_ADDTOOL, 0, (LPARAM)&tinf);
            tinf.hwnd = hInfo;
            tinf.lpszText = "F1";
            SendMessage(hToolTip, TTM_ADDTOOL, 0, (LPARAM)&tinf);

            /*-----ustawianie ikony programu w zasobniku systemowym-----*/
            nim.cbSize = sizeof(NOTIFYICONDATA);
            nim.hWnd = hWnd;
            nim.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
            nim.hIcon = ikona;
            nim.uCallbackMessage = WM_TRAY;
            nim.uID = 7;
            lstrcpyn(nim.szTip, "Manipulator by FeniX", sizeof(nim.szTip));
            Shell_NotifyIcon(NIM_ADD, &nim);
            refresh();
            break;
        case WM_KEYUP:  //obs³uga skrótów klawiaturowych dla konkretnych akcji
            switch (wParam)
            {
                case VK_F5:
                    SendMessage(hRefresh, BM_CLICK, 0, 0);
                    break;
                case VK_F1:
                    SendMessage(hInfo, BM_CLICK, 0, 0);
                    break;
                case VK_SPACE:
                    SendMessage(hShow, BM_CLICK, 0, 0);
                    break;
                case VK_ESCAPE:
                    ShowWindow(hWnd, SW_MINIMIZE);
                    break;
            }
            break;
        case WM_COMMAND:    //obs³uga zdarzeñ okien pochodnych okna g³ównego
            switch (HIWORD(wParam))
            {
                case BN_CLICKED:    //komunikat klikniêcia przycisku
                    if ((HWND)lParam == hRefresh)
                    {
                        refresh();
                        SetFocus(hList);
                    }
                    else if ((HWND)lParam == hInfo)
                    {
                        MessageBox(hWnd, "Autor:\nKonrad Gadzina <fenix.b3@gmail.com>\n"
                                         "Grupa 1131, Informatyka, rok akad. 2008/2009"
                                         "\nPracownia In¿ynierii Oprogramowania M-74\n"
                                         "Instytut Informatyki Stosowanej\n"
                                         "Wydzia³ Mechaniczny, Politechnika Krakowska\nJêzyki i Techniki Programowania",
                                         "O programie", MB_OK);
                    }
                    else if ((HWND)lParam == hShow)
                    {
                        int poz;
                        char buf[10];
                        bool hidden;
                        TBBUTTON *ptbb, tbb;
                        NOTIFYICONDATA fake;  //potrzebne do "odswiezania" paska zadan i dopasowywania szerokosci przyciskow

                        ptbb = (TBBUTTON*)VirtualAllocEx(proces, NULL, sizeof(TBBUTTON), MEM_COMMIT, PAGE_READWRITE);
                        poz = ListView_GetNextItem(hList, (WPARAM)-1, LVNI_SELECTED);

                        SendMessage(hPasek, TB_GETBUTTON, (WPARAM)poz, (LPARAM)ptbb);
                        ReadProcessMemory(proces, (void*)ptbb, (void*)&tbb, sizeof(TBBUTTON), NULL);

                        hidden = SendMessage(hPasek, TB_ISBUTTONHIDDEN, (WPARAM)tbb.idCommand, 0);
                        SendMessage(hPasek, TB_HIDEBUTTON, (WPARAM)tbb.idCommand, (LPARAM)!hidden);

                        if (hidden)
                            SendMessage(hShow, WM_SETTEXT, 0, (LPARAM)"Ukryj przycisk");
                        else
                            SendMessage(hShow, WM_SETTEXT, 0, (LPARAM)"Poka¿ przycisk");

                        /*---dodawanie i usuwanie ikony do zasobnika, by pasek zadañ siê "odœwie¿y³"---*/
                        fake.cbSize = sizeof(NOTIFYICONDATA);
                        fake.hWnd = hWnd;
                        fake.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
                        fake.hIcon = ikona;
                        fake.uCallbackMessage = WM_TRAY;
                        fake.uID = 19;
                        lstrcpyn(fake.szTip, "Manipulator by FeniX", sizeof(fake.szTip));
                        Shell_NotifyIcon(NIM_ADD, &fake);
                        Shell_NotifyIcon(NIM_DELETE, &fake);

                        for (int i = 0; i<ile; i++)
                        {
                            SendMessage(hPasek, TB_GETBUTTON, (WPARAM)i, (LPARAM)ptbb);
                            ReadProcessMemory(proces, (void*)ptbb, (void*)&tbb, sizeof(TBBUTTON), NULL);
                            hidden = SendMessage(hPasek, TB_ISBUTTONHIDDEN, (WPARAM)tbb.idCommand, 0);

                            if (!hidden)
                                strcpy(buf, "[widoczny]");
                            else
                                strcpy(buf, "[ukryty]");
                            ListView_SetItemText(hList, i, 1, buf);
                        }
                        SetFocus(hList);

                        VirtualFreeEx(proces, ptbb, 0, MEM_RELEASE);
                    }
                    break;
            }
            break;
        case WM_TRAY:   //obs³uga klikniêcia na ikonkê programu w zasobniku systemowym
            switch ((UINT)lParam)
            {
                case WM_LBUTTONDOWN:
                    if (!IsWindowVisible(hWnd))
                    {
                        if (IsZoomed(hWnd))
                            ShowWindow(hWnd, SW_MAXIMIZE);
                        else
                            ShowWindow(hWnd, SW_RESTORE);
                        SetForegroundWindow(hWnd);
                    }
                    else if (GetForegroundWindow() == hWnd)
                        ShowWindow(hWnd, SW_HIDE);
                    else
                        SetForegroundWindow(hWnd);
                    break;
            }
            break;
        case WM_ACTIVATE:
            if (LOWORD(wParam) != WA_INACTIVE)
                refresh();
            break;
        case WM_SIZE:   //zdarzenie zmienienia rozmiaru
            int szer, wys;

            if (wParam & SIZE_MINIMIZED)
                ShowWindow(hWnd, SW_HIDE);

            szer = (int)LOWORD(lParam);
            wys = (int)HIWORD(lParam );

            /*
               Poni¿ze linijki pozwalaj¹ przestawiaæ kontrolki tak, by
               dopasowa ich pozycjê/rozmiar do rozmiaru okna g³ównego
            */
            MoveWindow(hList, 0, 0, szer, wys-23, false);
            MoveWindow(hShow, 20, wys-21, 120, 20, false);
            MoveWindow(hRefresh, szer-310, wys-21, 120, 20, false);
            MoveWindow(hInfo, szer-140, wys-21, 120, 20, false);
            break;
        case WM_SIZING: //zdarzenie zmieniania rozmiaru
            RECT* rc;
            rc = (RECT*)lParam;
            /*
               Poni¿sze linie pozwalaj¹ ograniczyc rozmiar okna - jako, ¿e to zdarzenie jest
               wywo³ywane w trakcie zmiany rozmiaru, warunki s¹ sprawdzane nie po puszczeniu
               przycisku myszy, a jeszcze w trakcie jego trzymania. Dziêki temu okno nie
               nigdy nie bêdzie mia³o rozmiarów mniejszych ni¿ 500x100 pikseli.
            */
            if (rc->right - rc->left <= 500)
                rc->right = rc->left + 500;
            if (rc->bottom - rc->top <= 100)
                rc->bottom = rc->top + 100;
            break;
        case WM_DESTROY:    //zdarzenie niszczenia okna
            CloseHandle(proces);
            Shell_NotifyIcon(NIM_DELETE, &nim);
            PostQuitMessage(0);
            break;

        case WM_NOTIFY: //obs³uga komunikatów od kontrolki ListView
            if (((LPNMHDR)lParam)->hwndFrom == hList && ((LPNMHDR)lParam)->code == (unsigned int)NM_DBLCLK)
                SendMessage(hShow, BM_CLICK, 0, 0);
            else if (((LPNMHDR)lParam)->hwndFrom == hList && ((LPNMHDR)lParam)->code == (unsigned int)NM_CLICK)
            {
                char buf[10];
                int poz = ListView_GetNextItem(hList, (WPARAM)-1, LVNI_SELECTED);

                ListView_GetItemText(hList, poz, 1, buf, 50);
                if (!strcmp(buf, "[ukryty]"))
                    SendMessage(hShow, WM_SETTEXT, 0, (LPARAM)"Poka¿ przycisk");
                else
                    SendMessage(hShow, WM_SETTEXT, 0, (LPARAM)"Ukryj przycisk");
            }
            else if (((LPNMHDR)lParam)->hwndFrom == hList && ((LPNMHDR)lParam)->code == LVN_KEYDOWN)
            {
                switch (((LV_KEYDOWN*)lParam)->wVKey)
                {
                case VK_F5:
                    SendMessage(hRefresh, BM_CLICK, 0, 0);
                    break;
                case VK_F1:
                    SendMessage(hInfo, BM_CLICK, 0, 0);
                    break;
                case VK_SPACE:
                    SendMessage(hShow, BM_CLICK, 0, 0);
                    break;
                case VK_ESCAPE:
                    ShowWindow(hWnd, SW_MINIMIZE);
                    break;
                }
            }
            /*------------------- obs³uga drag & drop na liœcie z przyciskami -------------------*/
            else if (((LPNMHDR)lParam)->hwndFrom == hList && ((LPNMHDR)lParam)->code == LVN_BEGINDRAG)
            {
                //wiadomoœæ wysy³ana do okna g³ównego, gdy rozpoczynamy drag & drop na liœcie
                //czyli ³apiemy element lewym przyciskiem myszy
                HIMAGELIST hOneImageList, hTempImageList;
                IMAGEINFO imginf;
                int x = 0, wys;
                pkt.x = 1;
                pkt.y = 1;

                poz = ListView_GetNextItem(hList, (WPARAM)-1, LVNI_SELECTED);  //pobieranie indexu zaznaczonego elementu
                item.iItem = poz;
                item.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM; //ustawienie maski elementu listy do pobrania
                ListView_GetItem(hList, &item);    //pobieranie danego elementu do zmiennej item

                if (item.iIndent) //element z wciêciem nas nie interesuje
                    break;

                hDragImgList = ListView_CreateDragImage(hList, poz, &pkt); //tworzenie "duszka" do d&d
                ImageList_GetImageInfo(hDragImgList, 0, &imginf);
                wys = imginf.rcImage.bottom;

                while(true)    //dodawanie elementów danej grupy do "duszka" w pêtli
                {
                    if (++item.iItem >= ile)
                        break;
                    item.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM; //ustawianie maski pobierania danych elementu
                    ListView_GetItem(hList, &item);
                    if(item.iIndent == 0) //je¿eli napotkano kolejny przycisk grupowy trzeba przerwaæ
                        break;

                    hOneImageList = ListView_CreateDragImage(hList, item.iItem, &pkt);
                    hTempImageList = ImageList_Merge(hDragImgList, 0, hOneImageList, 0, 0, wys);
                    ImageList_Destroy(hDragImgList);
                    ImageList_Destroy(hOneImageList);
                    hDragImgList = hTempImageList;
                    ImageList_GetImageInfo(hDragImgList, 0, &imginf);
                    wys = imginf.rcImage.bottom;
                }

                item.iItem = poz;
                item.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM;
                ListView_GetItem(hList, &item);

                ImageList_BeginDrag(hDragImgList, 0, x, 0);
                pkt = ((NM_LISTVIEW*) ((LPNMHDR)lParam))->ptAction;
                ClientToScreen(hList, &pkt);
                ImageList_DragEnter(GetDesktopWindow(), pkt.x, pkt.y);
                bDragging = true;

                SetCapture(hWnd);
            }
            break;
        case WM_MOUSEMOVE:  //zdarzenie ruchu kursora myszy
            if (!bDragging) //je¿eli nie obs³gujemy akurat d&d na liœcie to nie trzeba nic robiæ
                break;

            pkt.x = LOWORD(lParam);
            pkt.y = HIWORD(lParam);
            ClientToScreen(hWnd, &pkt);
            ImageList_DragMove(pkt.x, pkt.y);
            break;
        case WM_LBUTTONUP:  //zdarzenie puszczenia lewego przycisku myszy
            if (!bDragging) //je¿eli nie obs³gujemy akurat d&d na liœcie to nie trzeba nic robiæ
                break;

            LVHITTESTINFO lvhti;
            char buf[256], sub[12];

            /*---puszczamy przycisk, wiêc przeci¹ganie siê koñczy---*/
            bDragging = false;
            ImageList_DragLeave(hList);
            ImageList_EndDrag();
            //ImageList_Destroy(hDragImgList);  //nie wiem czemu, ale ta linia wywo³uje b³¹d segfault przy uruchomieniu debuggera
            ReleaseCapture();


            /*---sprawdzanie, czy przeci¹gany element zosta³ upuszczony na inny element listy---*/
            lvhti.pt.x = LOWORD(lParam);
            lvhti.pt.y = HIWORD(lParam);
            ClientToScreen(hWnd, &lvhti.pt);
            ScreenToClient(hList, &lvhti.pt);
            ListView_HitTest(hList, &lvhti);
            if (lvhti.iItem == -1)
                break;
            if ((lvhti.flags & LVHT_ONITEMLABEL == 0) && (lvhti.flags & LVHT_ONITEMSTATEICON == 0))
                break;

            if (!item.iIndent) //je¿eli wciêcie = 0, to znaczy ¿e zajmujemy siê przyciskiem "grupowym"
            {
                poz = ListView_GetNextItem(hList, (WPARAM)-1, LVNI_SELECTED);

                if (lvhti.iItem > poz)
                    lvhti.iItem++;

                while (true)
                {
                    if (lvhti.iItem == poz || lvhti.iItem > ile-1 || lvhti.iItem < 0)
                        break;
                    item.iItem = lvhti.iItem;
                    item.iSubItem = 0;
                    item.mask = LVIF_STATE | LVIF_IMAGE | LVIF_INDENT | LVIF_TEXT | LVIF_PARAM;
                    item.stateMask = LVIS_SELECTED;
                    ListView_GetItem(hList, &item);
                    if (item.iIndent)
                    {
                        if (lvhti.iItem > poz)
                            lvhti.iItem++;
                        else if (lvhti.iItem < poz)
                            lvhti.iItem--;
                    }
                    else
                    {
                        if (lvhti.iItem > poz)
                            lvhti.iItem--;
                        break;
                    }
                }

                ListView_GetItemText(hList, poz, 0, buf, 256); //pobieranie tekstu danego elementu listy
                while (true)
                {
                    int tmp = poz;

                    item.iItem = poz;
                    item.iSubItem = 0;
                    item.cchTextMax = 256;
                    item.pszText = buf;
                    item.stateMask = ~LVIS_SELECTED;
                    item.mask = LVIF_STATE | LVIF_IMAGE | LVIF_INDENT | LVIF_TEXT | LVIF_PARAM;
                    ListView_GetItem(hList, &item);
                    ListView_GetItemText(hList, poz, 1, sub, 12);

                    SendMessage(hPasek, TB_MOVEBUTTON, (WPARAM)poz, (LPARAM)lvhti.iItem);
                    if (lvhti.iItem > poz && lvhti.iItem < ile)
                        lvhti.iItem++;
                    item.iItem = lvhti.iItem;

                    ListView_InsertItem(hList, &item);
                    ListView_SetItemText(hList, item.iItem, 1, sub);
                    if (lvhti.iItem < poz)
                        poz++;
                    ListView_DeleteItem(hList, poz);
                    if (lvhti.iItem > tmp)
                        lvhti.iItem--;
                    if (lvhti.iItem < tmp)
                        lvhti.iItem++;
                    ListView_GetItemText(hList, poz, 0, buf, 256);
                    item.iItem = poz;
                    item.iSubItem = 0;
                    item.cchTextMax = 256;
                    item.pszText = buf;
                    ListView_GetItem(hList, &item);
                    if (!item.iIndent || poz > ile-1)  //je¿eli
                        break;
                }
            }
            break;
        default:
            return DefWindowProc(hWnd, komunikat, wParam, lParam);
    }
    return 0;
}
