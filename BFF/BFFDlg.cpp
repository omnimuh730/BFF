// BFFDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "BFF.h"
#include "BFFDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CBFFDlg dialog

HHOOK CBFFDlg::m_hKeyboardHook = NULL;
CBFFDlg* CBFFDlg::m_pThis = nullptr;

CBFFDlg::CBFFDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BFF_DIALOG, pParent)
	, m_bShowGreenCircle(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBFFDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

UINT HideCircleThreadProc(LPVOID pParam)
{
	CBFFDlg* pDlg = reinterpret_cast<CBFFDlg*>(pParam);
	::Sleep(100); // 100 ms
	if (pDlg && ::IsWindow(pDlg->GetSafeHwnd()))
	{
		::PostMessage(pDlg->GetSafeHwnd(), WM_HIDE_CIRCLE, 0, 0);
	}
	return 0;
}

BEGIN_MESSAGE_MAP(CBFFDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_HIDE_CIRCLE, &CBFFDlg::OnHideCircle)
END_MESSAGE_MAP()


// CBFFDlg message handlers

BOOL CBFFDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_bShowGreenCircle = TRUE; // Show green circle at startup
	m_pThis = this;
	m_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);

	InvalidateRect(NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBFFDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CBFFDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint(); // Call the base class first to draw the background

		/*
		if (m_bShowGreenCircle == true)
			MessageBox(L"true", L"", NULL);
		else
			MessageBox(L"false", L"", NULL);
		*/
		// Check if we should draw the circle
		if (m_bShowGreenCircle)
		{
			// Use a CClientDC for drawing outside of the original paint message scope
			// Or simply perform drawing after the base call within the same handler.
			// For simplicity and correctness, we get the DC for the client area.
			CClientDC dc(this);

			// Define the circle's properties
			int circleCenterX = 100; // X-coordinate of the circle's center
			int circleCenterY = 100; // Y-coordinate of the circle's center
			int circleRadius = 50;   // Radius of the circle

			// Create a green brush
			CBrush greenBrush(RGB(0, 255, 0)); // RGB for green

			// Select the green brush into the device context
			CBrush* pOldBrush = dc.SelectObject(&greenBrush);

			// Create a pen for the circle's outline
			CPen blackPen(PS_SOLID, 1, RGB(0, 0, 0)); // Black solid pen with 1 pixel width
			CPen* pOldPen = dc.SelectObject(&blackPen);

			// Calculate the bounding rectangle for the circle
			CRect circleRect(circleCenterX - circleRadius,
				circleCenterY - circleRadius,
				circleCenterX + circleRadius,
				circleCenterY + circleRadius);

			// Draw the ellipse (which is a circle in this case)
			dc.Ellipse(circleRect);

			// Restore the original brush and pen
			dc.SelectObject(pOldBrush);
			dc.SelectObject(pOldPen);
		}
	}
}
// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CBFFDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CALLBACK CBFFDlg::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION && wParam == WM_KEYDOWN)
	{
		if (m_pThis)
		{
			m_pThis->m_bShowGreenCircle = TRUE;
//			CWnd* pStatus = m_pThis->GetDlgItem(IDC_STATIC_STATUS);
			m_pThis->Invalidate();
			AfxBeginThread(HideCircleThreadProc, m_pThis);
		}
	}
	return CallNextHookEx(m_hKeyboardHook, nCode, wParam, lParam);
}

LRESULT CBFFDlg::OnHideCircle(WPARAM, LPARAM)
{
    m_bShowGreenCircle = FALSE;
    Invalidate();
    return 0;
}