#ifndef UNICODE
#define UNICODE
#endif 

#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "Ws2_32.lib")

#include <windows.h>  
#include <iostream>


#define AUTH_BUTTON 1
#define AUTH_TEXTBOX 2
#define AUTH_TEXTBOX2 3
#define MAIN_TEXTBOX 101
#define MAIN_SENDBUTTON 102
#define MAIN_SETTINGSBUTTON 103
#define MAIN_STATICBOX 104
#define AUTH_TEXTBOX3 4
#define BUFFER_SIZE 2048
#define IP_TARGET "127.0.0.1"

using namespace std;

BOOL bEnd = FALSE;
WSADATA wsaData = { 0 };
SOCKET sock;                    // Сокет сервера
WORD wSrcport, wDstPort;        // Для портов


HWND hAuthwnd;                  // Окно входа
HWND hMainwnd;                  // Окно чата

HWND hAuthButton;               // Кнопка войти
HWND hAuthNameTextBox;          // Инпут для имени
HWND hAuthPortTextBox;          // Инпут для порта 
HWND hMainMesageTextBox;        // Инпут для сообщений
HWND hMainMesagesBox;           // Окно для сообщений
HWND hMainSendButton;           // Кнопка войти
HWND hAuthPort2TextBox;         // Инпут для второго порта

char* Name_convert = (char*)malloc(BUFFER_SIZE); // Для конвертирования имени из LPWSTR в CHAR
char* port1_convert = (char*)malloc(BUFFER_SIZE);// Для конвертирования порта из LPWSTR в CHAR
char* port2_convert = (char*)malloc(BUFFER_SIZE);// Для конвертирования порта из LPWSTR в CHAR

LPWSTR name = (LPWSTR)VirtualAlloc((LPVOID)NULL, // Выделение памяти для имени 
    (DWORD)(256), MEM_COMMIT,
    PAGE_READWRITE);
LPWSTR port1 = (LPWSTR)VirtualAlloc((LPVOID)NULL,
    (DWORD)(256), MEM_COMMIT,
    PAGE_READWRITE);
LPWSTR port2 = (LPWSTR)VirtualAlloc((LPVOID)NULL,
    (DWORD)(256), MEM_COMMIT,
    PAGE_READWRITE);



const wchar_t MAIN_CLASS_NAME[] = L"ChatMainWindowClass";
const wchar_t AUTH_CLASS_NAME[] = L"ChatAuthWindowClass";

LRESULT CALLBACK AuthWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
wchar_t* getwc(const char* c) {
    size_t iSize = strlen(c) + 1;
    wchar_t* wc = (wchar_t*)calloc(iSize, sizeof(wchar_t));
    mbstowcs(wc, c, iSize);
    return wc;
}
SOCKET MakeSocket(WORD wPort)
{
    SOCKET sock = (SOCKET)NULL;
    SOCKADDR_IN Addr = { 0 };   // адрес сокета

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
    {
        return (SOCKET)NULL;
    }

    Addr.sin_family = AF_INET;   //биндим порт 
    Addr.sin_port = htons(wPort);
    Addr.sin_addr.s_addr = inet_addr(IP_TARGET);

    if (bind(sock, (SOCKADDR*)&Addr, sizeof(Addr)) == SOCKET_ERROR)  // проверка на правильность привязки 
    {
        closesocket(sock);
        return(SOCKET)NULL;
    }

    return sock;
}




BOOL SendData(SOCKET sock, WORD wDstPort)            // Отправка сообщений
{
    SOCKADDR_IN SendAddr = { 0 };
    SendAddr.sin_family = AF_INET;
    SendAddr.sin_port = htons(wDstPort);             // Привязывание порта собеседника к сокету
    SendAddr.sin_addr.s_addr = inet_addr(IP_TARGET); // Привязывание IP к сокету

    size_t   i;
    char* buff = (char*)malloc(BUFFER_SIZE);         // Для хранения сообщения
    wchar_t* Mes_wc = (wchar_t*)malloc(BUFFER_SIZE); // Для конвертирования сообщения
    GetWindowText(hMainMesageTextBox, Mes_wc, 1024);
    wcstombs_s(&i, buff, (size_t)BUFFER_SIZE,        // Конвертирование wchar в char
        Mes_wc, (size_t)BUFFER_SIZE);

    if (buff[0] == 'q')
        return FALSE;
    sendto(sock, Name_convert, strlen(Name_convert), 0, (SOCKADDR*)&SendAddr, sizeof(SendAddr));    // отправляем имя
    sendto(sock, ": ", strlen(": "), 0, (SOCKADDR*)&SendAddr, sizeof(SendAddr));                    // отправляем двоеточие 
    sendto(sock, buff, strlen(buff), 0, (SOCKADDR*)&SendAddr, sizeof(SendAddr));  // отправка сообщения собеседнику
    return TRUE;
}
int k = 0;
DWORD WINAPI RecvThread(LPVOID pParam)  // Поток приёмка сообщения с клиента
{
    SOCKET sock = (SOCKET)pParam;    // открывается поток
    SOCKADDR_IN RecvAddr = { 0 };
    int iRet, iRecvSize;
    char buf[1024];
    while (1)        // в бесконенчом цикле принимаем сообщение 
    {
        iRecvSize = sizeof(RecvAddr);
        iRet = recvfrom(sock, buf, 1024, 0, (SOCKADDR*)&RecvAddr, &iRecvSize);
        if (iRet == SOCKET_ERROR)
            continue;
        buf[iRet] = '\0';    
        SendDlgItemMessage(hMainwnd, MAIN_STATICBOX, EM_REPLACESEL, 0, LPARAM(getwc(buf))); // Отображение сообщения в окне
        if (k==2)
        {
            SendDlgItemMessage(hMainwnd, MAIN_STATICBOX, EM_REPLACESEL, 0, (LPARAM)L"\r\n"); // Перевод каретки
            k = 0;
        }
        else
        {
            k++;
        }
    }
    return 0;
}
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    setlocale(LC_ALL, "Russian");
    WNDCLASS authWc = { };
    WNDCLASS mainWc = { };
    authWc.lpfnWndProc = AuthWindowProc;
    authWc.hInstance = hInstance;
    authWc.lpszClassName = AUTH_CLASS_NAME;
    mainWc.lpfnWndProc = MainWindowProc;
    mainWc.hInstance = hInstance;
    mainWc.lpszClassName = MAIN_CLASS_NAME;
    mainWc.hbrBackground = CreateSolidBrush(RGB(14,22,33));
    RegisterClass(&authWc);
    RegisterClass(&mainWc);

    // Окна входа и чата
    hAuthwnd = CreateWindowEx(0, AUTH_CLASS_NAME, L"P2P Чат", WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 
        420, 300,  NULL, NULL, hInstance, NULL);
    hMainwnd = CreateWindowEx(0, MAIN_CLASS_NAME, L"Чат", WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500, NULL, NULL, hInstance, NULL);
    if (hAuthwnd == NULL)
    {
        return 0;
    }
    ShowWindow(hAuthwnd, nCmdShow);
    ShowWindow(hMainwnd, SW_HIDE);

    // Запуск цикла сообщений по окнам 
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);   // переводит сообщения виртуальных клавиш в символьные сообщения
        DispatchMessage(&msg);
    }
    return 0;
}
LRESULT CALLBACK AuthWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) // принимает сообщение от окна авторизации 
{
    switch (uMsg)
    {
    case WM_CREATE:
        hAuthButton = CreateWindowEx(0, L"BUTTON", L"Войти", WS_VISIBLE | WS_CHILD | BS_FLAT, 100, 217, 200, 40, hwnd,
            (HMENU)AUTH_BUTTON, NULL, NULL);
        hAuthNameTextBox = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"Введите имя", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 100, 97, 200, 40, hwnd,
            (HMENU)AUTH_TEXTBOX, NULL, NULL);
        hAuthPortTextBox = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"Введите свой порт", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 100, 137, 200, 40, hwnd,
            (HMENU)AUTH_TEXTBOX2, NULL, NULL);
        hAuthPort2TextBox = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"Введите порт собеседника", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 100, 177, 200, 40, hwnd,
            (HMENU)AUTH_TEXTBOX3, NULL, NULL);
        break;

    case WM_PAINT:       
        PAINTSTRUCT ps;
        HFONT hFont;
        HDC hdc;
        hFont = CreateFont(48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"Arial");
        hdc = BeginPaint(hwnd, &ps);
        SelectObject(hdc, hFont);
        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkMode(hdc, 1);
        TextOut(hdc, 100, 38, L"Вход", strlen("Вход"));
        ReleaseDC(hwnd, hdc);
        EndPaint(hwnd, &ps);
        break;                                                         

    case WM_DESTROY:         // закрытие окна 
        PostQuitMessage(0);
        return 0;

    case WM_COMMAND:
        switch (wParam)
        {
        case AUTH_BUTTON:           
            //char name[256];
            GetWindowText(hAuthNameTextBox, name, 256); //прием имени пользователя из окна
            GetWindowText(hAuthPortTextBox, port1, 256); // прием порта 
            GetWindowText(hAuthPort2TextBox , port2, 256);
            size_t   i;
            wcstombs_s(&i, Name_convert, (size_t)BUFFER_SIZE, //конвертирование w_char в char
                name, (size_t)BUFFER_SIZE);
            wcstombs_s(&i, port2_convert, (size_t)BUFFER_SIZE,
                port1, (size_t)BUFFER_SIZE);
            wcstombs_s(&i, port1_convert, (size_t)BUFFER_SIZE,
                port2, (size_t)BUFFER_SIZE);
            int intport1 = atoi(port1_convert); // конвертирование char в int
            int intport2 = atoi(port2_convert);
            wSrcport = (WORD)(intport1);
            wDstPort = (WORD)(intport2);
           
            WSAStartup(MAKEWORD(2, 2), &wsaData);
            sock = MakeSocket(wSrcport); // старт сервера
            HANDLE hThread = CreateThread(NULL, 0, RecvThread, (PVOID)sock, 0, NULL); // создание потока для приема сообщений
            
            ShowWindow(hMainwnd, SW_SHOW); //показать главное окно чата
            ShowWindow(hAuthwnd, SW_HIDE); //скрыть окно авторизации
            break;
        }
    return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)   // дескриптор второго окна 
{
    switch (uMsg)   // обработчик 
    {
    case WM_DESTROY:
       
        PostQuitMessage(0);
        return 0;
    
    case WM_CREATE:
        hMainMesageTextBox = CreateWindowEx(0, L"EDIT", L"Введите сообщение", WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE,
            5, 415, 340, 40, hwnd, (HMENU)MAIN_TEXTBOX, NULL, NULL);       //Инпут сообщения
        hMainMesagesBox = CreateWindowEx(0, L"EDIT", L"", WS_VSCROLL | WS_VISIBLE | WS_CHILD | ES_MULTILINE    /* | ES_READONLY*/,
            0, 0, 480, 370, hwnd, (HMENU)MAIN_STATICBOX, NULL, NULL);        //Окно сообщений
        hMainSendButton = CreateWindowEx(0, L"BUTTON", L"Отправить", WS_VISIBLE | WS_CHILD | BS_FLAT, 
            360, 415, 100,40, hwnd, (HMENU)MAIN_SENDBUTTON, NULL, NULL);                                //Кнопка отправить
        SendMessage(hMainMesageTextBox, EM_SETLIMITTEXT, 256, 0);           //Ограничение на ввод символов

    case WM_PAINT:
        HDC hdc;
        HFONT hFont;
        PAINTSTRUCT ps;
        RECT mainHeader;
        RECT mainFooter;
        hdc = BeginPaint(hwnd, &ps);

        //Закраска футера и хедера
        mainHeader.left = 0;
        mainHeader.right = 500;
        mainHeader.top = 0;
        mainHeader.bottom = 80;
        mainFooter.left = 0;
        mainFooter.right = 500;
        mainFooter.top = 920;
        mainFooter.bottom = 1000;
        FillRect(hdc, &mainHeader, CreateSolidBrush(RGB(23, 33, 43)));
        FillRect(hdc, &mainFooter, CreateSolidBrush(RGB(23, 33, 43)));

        //Надпись названия чата
        hFont = CreateFont(36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"Arial");
        SelectObject(hdc, hFont);
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, 1);
        ReleaseDC(hwnd, hdc);
        EndPaint(hwnd, &ps);
        break;

    case WM_CTLCOLOREDIT:
        SelectObject((HDC)wParam, CreateFont(20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"Arial"));

        //Красим и задаем стиль инпутам
        if (hMainMesagesBox == (HWND)lParam)        //Окно вывода сообщений
        {
            SetTextColor((HDC)wParam, RGB(255, 255, 255));
            SetBkColor((HDC)wParam, RGB(14, 22, 33));
            return (LRESULT)CreateSolidBrush(RGB(14, 22, 33));
        }
        else if (hMainMesageTextBox == (HWND)lParam) {      //Инпут нового сообщения
            SetTextColor((HDC)wParam, RGB(255, 255, 255));
            SetBkColor((HDC)wParam, RGB(23, 33, 43));
            return (LRESULT)CreateSolidBrush(RGB(23, 33, 43));
        }
        else break;

    case WM_COMMAND:
        switch (wParam)
        {
        case MAIN_SENDBUTTON:
            LPWSTR msg = (LPWSTR)VirtualAlloc((LPVOID)NULL,
                (DWORD)(256), MEM_COMMIT,
                PAGE_READWRITE);
            GetWindowText(hMainMesageTextBox, msg, 1024);  // берем текст сообщения 
            SendData(sock, wDstPort);
            SendDlgItemMessage(hMainwnd, MAIN_STATICBOX, EM_REPLACESEL, 0, LPARAM(name));
            SendDlgItemMessage(hMainwnd, MAIN_STATICBOX, EM_REPLACESEL, 0, LPARAM(getwc(": ")));
            SendDlgItemMessage(hMainwnd, MAIN_STATICBOX, EM_REPLACESEL, 0, (LPARAM)msg);
            SendDlgItemMessage(hMainwnd, MAIN_STATICBOX, EM_REPLACESEL, 0, (LPARAM)L"\r\n");
            break;
        }
    return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}