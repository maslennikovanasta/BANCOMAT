#include "framework.h"
#include "BankomatATM4.h"
#include <string>
#include <sstream>
#include <iomanip>
using namespace std;
struct CreditCardData {
    wchar_t Parol[5];
    wchar_t ClientAttributes[100];
    int LimitOfMoney;
    bool CardInserted;
    wchar_t CardNumber[20];
    CreditCardData() {
        wcscpy_s(Parol, L"1234");
        wcscpy_s(ClientAttributes, L"Комалов Максим Иванович");
        wcscpy_s(CardNumber, L"4276 3800 1234 5678");
        LimitOfMoney = 50000;
        CardInserted = false;
    }
};
#define MAX_LOADSTRING 100
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
HWND hDisplay, hStatus, hBalance, hReceipt, hMsg;
HWND hInsert, hEject;
HWND hBtnOps[4];
HWND hDigit[10];
HWND hClear, hEnter, hCancel;
enum class State { WaitingCard, PinEntry, MainMenu, Withdraw, BalanceShown, StatementShown };
State state = State::WaitingCard;
CreditCardData card;
wstring pinInput;
int withdrawSum = 0;
HBRUSH hbrDarkGreen, hbrDarkerGreen, hbrTextBg;
void CreateControls(HWND hWnd);
void UpdateDisplay();
void AddDigit(int d);
void ProcessEnter();
void ProcessClear();
void ShowOperations(bool visible);
void AnimatedMsg(const wchar_t* txt, int ms = 800);
void CreditCartControlModule(int cmd);
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_BANKOMATATM4, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow)) return FALSE;
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex{};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BANKOMATATM4));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = szWindowClass;
    return RegisterClassExW(&wcex);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    HWND hWnd = CreateWindowW(szWindowClass, L"БАНКОМАТ – Система обслуживания клиентов",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 920, 720, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) return FALSE;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}
void CreateControls(HWND hWnd)
{
    hbrDarkGreen = CreateSolidBrush(RGB(0, 100, 60));
    hbrDarkerGreen = CreateSolidBrush(RGB(0, 80, 50));
    hbrTextBg = CreateSolidBrush(RGB(0, 120, 80));
    SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbrDarkGreen);
    CreateWindow(L"STATIC", L"БАНКОВСКАЯ ПОДСИСТЕМА ОБСЛУЖИВАНИЯ КЛИЕНТОВ",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        20, 10, 870, 40, hWnd, 0, hInst, 0);
    hDisplay = CreateWindow(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_READONLY | ES_CENTER | WS_BORDER,
        20, 60, 570, 140, hWnd, 0, hInst, 0);
    hStatus = CreateWindow(L"EDIT", L"СТАТУС: НЕТ КАРТЫ",
        WS_VISIBLE | WS_CHILD | ES_READONLY | ES_CENTER | WS_BORDER,
        610, 60, 270, 35, hWnd, 0, hInst, 0);
    hInsert = CreateWindow(L"BUTTON", L"ВСТАВИТЬ КАРТУ",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        610, 105, 130, 40, hWnd, (HMENU)1001, hInst, 0);
    hEject = CreateWindow(L"BUTTON", L"ИЗВЛЕЧЬ КАРТУ",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
        750, 105, 130, 40, hWnd, (HMENU)1002, hInst, 0);
    CreateWindow(L"STATIC", L"ИНФОРМАЦИЯ О КЛИЕНТЕ:",
        WS_VISIBLE | WS_CHILD,
        20, 210, 570, 25, hWnd, 0, hInst, 0);
    hBalance = CreateWindow(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | ES_READONLY | WS_BORDER,
        20, 240, 570, 35, hWnd, 0, hInst, 0);
    CreateWindow(L"GROUPBOX", L"КЛАВИАТУРА",
        WS_VISIBLE | WS_CHILD,
        20, 290, 370, 300, hWnd, 0, hInst, 0);
    int sz = 65, sp = 12, x0 = 45, y0 = 320;
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            int n = r * 3 + c + 1;
            int x = x0 + c * (sz + sp);
            int y = y0 + r * (sz + sp);
            hDigit[n] = CreateWindow(L"BUTTON", to_wstring(n).c_str(),
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                x, y, sz, sz, hWnd, (HMENU)(2000 + n), hInst, 0);
        }
    }
    hDigit[0] = CreateWindow(L"BUTTON", L"0",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        x0 + sz + sp, y0 + 3 * (sz + sp) - 10,
        sz * 2 + sp, sz,
        hWnd, (HMENU)2000, hInst, 0);
    int fx = x0 + 3 * (sz + sp) + 25;
    int fy = y0;
    hClear = CreateWindow(L"BUTTON", L"CLEAR", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, fx, fy, 110, sz, hWnd, (HMENU)1003, hInst, 0);
    hEnter = CreateWindow(L"BUTTON", L"ENTER", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, fx, fy + sz + sp, 110, sz, hWnd, (HMENU)1004, hInst, 0);
    hCancel = CreateWindow(L"BUTTON", L"CANCEL", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, fx, fy + 2 * (sz + sp), 110, sz, hWnd, (HMENU)1005, hInst, 0);
    CreateWindow(L"GROUPBOX", L"ОПЕРАЦИИ",
        WS_VISIBLE | WS_CHILD,
        410, 290, 290, 300, hWnd, 0, hInst, 0);
    const wchar_t* ops[] = { L"1. ПРОВЕРКА БАЛАНСА", L"2. СНЯТИЕ НАЛИЧНЫХ", L"3. ПЕЧАТЬ ВЫПИСКИ", L"0. ЗАВЕРШИТЬ" };
    int oy = 320;
    for (int i = 0; i < 4; i++) {
        hBtnOps[i] = CreateWindow(L"BUTTON", ops[i],
            WS_CHILD | BS_PUSHBUTTON,
            425, oy, 260, 55, hWnd, (HMENU)(1010 + i), hInst, 0);
        oy += 65;
    }
    CreateWindow(L"GROUPBOX", L"ЧЕК",
        WS_VISIBLE | WS_CHILD,
        720, 290, 180, 300, hWnd, 0, hInst, 0);
    hReceipt = CreateWindow(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_READONLY | WS_BORDER | ES_AUTOVSCROLL,
        730, 320, 160, 260, hWnd, 0, hInst, 0);
    hMsg = CreateWindow(L"EDIT", L"",
        WS_VISIBLE | WS_CHILD | ES_READONLY | WS_BORDER,
        20, 610, 880, 50, hWnd, 0, hInst, 0);
    NONCLIENTMETRICS ncm{ sizeof(ncm) };
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
    HFONT hf = CreateFontIndirect(&ncm.lfMessageFont);
    HWND ctrls[] = { hDisplay, hStatus, hBalance, hReceipt, hMsg, hInsert, hEject, hClear, hEnter, hCancel };
    for (auto c : ctrls) SendMessage(c, WM_SETFONT, (WPARAM)hf, TRUE);
    for (int i = 0; i < 10; i++) if (hDigit[i]) SendMessage(hDigit[i], WM_SETFONT, (WPARAM)hf, TRUE);
    for (auto b : hBtnOps) SendMessage(b, WM_SETFONT, (WPARAM)hf, TRUE);
    UpdateDisplay();
}
void AnimatedMsg(const wchar_t* txt, int ms)
{
    SetWindowText(hMsg, L"");
    wstring s = txt;
    for (wchar_t c : s) {
        wstring t = L">> " + wstring(1, c);
        SetWindowText(hMsg, t.c_str());
        UpdateWindow(hMsg);
        Sleep(ms / max(1, (int)s.length()));
    }
}
void UpdateDisplay()
{
    wstring txt;
    wstring bal = L"";
    switch (state)
    {
    case State::WaitingCard:
        txt = L"";
        EnableWindow(hInsert, TRUE);
        EnableWindow(hEject, FALSE);
        ShowOperations(false);
        SetWindowText(hStatus, L"СТАТУС: НЕТ КАРТЫ");
        SetWindowText(hBalance, L"");
        SetWindowText(hReceipt, L"");
        break;
    case State::PinEntry:
        txt = L"КАРТА ПРИНЯТА\n\nВведите ПИН-код:\n";
        for (size_t i = 0; i < pinInput.size(); ++i) txt += L"*";
        EnableWindow(hInsert, FALSE);
        EnableWindow(hEject, TRUE);
        ShowOperations(false);
        SetWindowText(hStatus, L"СТАТУС: КАРТА ВСТАВЛЕНА");
        SetWindowText(hBalance, L"");
        SetWindowText(hReceipt, L"");
        break;
    case State::MainMenu:
        txt = L"ВЫБЕРИТЕ ОПЕРАЦИЮ:\n\n"
            L"1 – Проверка баланса\n"
            L"2 – Снятие наличных\n"
            L"3 – Печать выписки\n"
            L"0 – Завершить работу";
        ShowOperations(true);
        SetWindowText(hBalance, L"");
        SetWindowText(hReceipt, L"");
        break;
    case State::Withdraw:
        txt = L"СУММА ДЛЯ СНЯТИЯ:\n\n"
            L"Доступно: " + to_wstring(card.LimitOfMoney) + L" ₽\n\n"
            L"Сумма: " + pinInput + L" ₽";
        ShowOperations(false);
        SetWindowText(hBalance, L"");
        SetWindowText(hReceipt, L"");
        break;
    case State::BalanceShown:
        txt = L"БАЛАНС:\n\n" + to_wstring(card.LimitOfMoney) + L" ₽";
        ShowOperations(true);
        bal = L"Клиент: " + wstring(card.ClientAttributes) +
            L"\nБаланс: " + to_wstring(card.LimitOfMoney) + L" ₽";
        SetWindowText(hBalance, bal.c_str());
        SetWindowText(hReceipt, L"");
        break;
    case State::StatementShown:
        txt = L"ВЫПИСКА ПО СЧЁТУ\n\n"
            L"Клиент:   " + wstring(card.ClientAttributes) + L"\n"
            L"Карта:    " + wstring(card.CardNumber) + L"\n"
            L"Баланс:   " + to_wstring(card.LimitOfMoney) + L" ₽\n"
            L"\nДата:     18.03.2026";
        ShowOperations(true);
        SetWindowText(hBalance, L"");
        SetWindowText(hReceipt, txt.c_str());
        break;
    }
    SetWindowText(hDisplay, txt.c_str());
}
void ShowOperations(bool visible)
{
    for (int i = 0; i < 4; i++) {
        ShowWindow(hBtnOps[i], visible ? SW_SHOW : SW_HIDE);
        EnableWindow(hBtnOps[i], visible ? TRUE : FALSE);
    }
}
void AddDigit(int d)
{
    if (state == State::PinEntry || state == State::Withdraw) {
        size_t maxLen = (state == State::PinEntry) ? 4 : 7;
        if (pinInput.length() < maxLen) {
            pinInput += to_wstring(d);
            UpdateDisplay();
        }
    }
}
void ProcessEnter()
{
    if (state == State::PinEntry) {
        if (pinInput.length() == 4) {
            if (pinInput == card.Parol) {
                AnimatedMsg(L"ПИН-код верен", 900);
                state = State::MainMenu;
            }
            else {
                AnimatedMsg(L"Неверный ПИН-код", 1200);
                pinInput.clear();
                CreditCartControlModule(2);
                state = State::WaitingCard;
            }
            UpdateDisplay();
            pinInput.clear();
        }
        return;
    }
    if (state == State::Withdraw) {
        try {
            withdrawSum = stoi(pinInput);
            if (withdrawSum <= 0 || withdrawSum % 100 != 0 || withdrawSum > card.LimitOfMoney) {
                AnimatedMsg(L"Неверная сумма или недостаточно средств", 1400);
                pinInput.clear();
                UpdateDisplay();
                return;
            }
            card.LimitOfMoney -= withdrawSum;
            wstringstream receipt;
            receipt << L"============================\r\n";
            receipt << L"      ЧЕК\r\n";
            receipt << L"============================\r\n";
            receipt << L"ОПЕРАЦИЯ: СНЯТИЕ НАЛИЧНЫХ\r\n";
            receipt << L"СУММА:     " << withdrawSum << L" ₽\r\n";
            receipt << L"ОСТАТОК:   " << card.LimitOfMoney << L" ₽\r\n";
            receipt << L"============================\r\n";
            receipt << L"Дата: 18.03.2026\r\n";
            SetWindowText(hReceipt, receipt.str().c_str());
            AnimatedMsg(L"Наличные выданы", 1200);
            state = State::MainMenu;
            pinInput.clear();
            UpdateDisplay();
        }
        catch (...) {
            AnimatedMsg(L"Ошибка ввода суммы", 1000);
            pinInput.clear();
            UpdateDisplay();
        }
        return;
    }
}
void ProcessClear()
{
    pinInput.clear();
    UpdateDisplay();
}
void CreditCartControlModule(int cmd)
{
    if (cmd == 1) {
        AnimatedMsg(L"Карта вставлена", 800);
        card.CardInserted = true;
        state = State::PinEntry;
        SetWindowText(hStatus, L"СТАТУС: КАРТА ВСТАВЛЕНА");
    }
    else {
        AnimatedMsg(L"Карта извлечена", 800);
        card.CardInserted = false;
        state = State::WaitingCard;
        SetWindowText(hStatus, L"СТАТУС: НЕТ КАРТЫ");
        pinInput.clear();
    }
    UpdateDisplay();
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        CreateControls(hWnd);
        break;

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wParam;
        SetBkMode(hdc, OPAQUE);
        SetTextColor(hdc, RGB(240, 255, 240));
        SetBkColor(hdc, RGB(0, 80, 50));
        return (LRESULT)hbrDarkerGreen;
    }
    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        if (id >= 2000 && id <= 2009) {
            AddDigit(id - 2000);
            break;
        }
        switch (id)
        {
        case 1001: CreditCartControlModule(1); break;
        case 1002: CreditCartControlModule(2); break;
        case 1003: ProcessClear(); break;
        case 1004: ProcessEnter(); break;
        case 1010:
            state = State::BalanceShown;
            AnimatedMsg(L"Ваш баланс", 1200);
            UpdateDisplay();
            break;
        case 1011: state = State::Withdraw; pinInput.clear(); UpdateDisplay(); break;
        case 1012:
            state = State::StatementShown;
            AnimatedMsg(L"Выписка сформирована", 1400);
            UpdateDisplay();
            break;
        case 1013: CreditCartControlModule(2); break;
        }
    }
    break;

    case WM_DESTROY:
        DeleteObject(hbrDarkGreen);
        DeleteObject(hbrDarkerGreen);
        DeleteObject(hbrTextBg);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}