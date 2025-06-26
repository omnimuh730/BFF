// BFFDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "BFF.h"
#include "BFFDlg.h"
#include "afxdialogex.h"
#include <gdiplus.h> // Good to have it visible here too
using namespace Gdiplus; // Use GDI+ namespace

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
	m_bShowGreenCircle = FALSE; // Show green circle at startup
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
		CPaintDC dc(this);

		CDialogEx::OnPaint(); // Call the base class first to draw the background

		// Check if we should draw the circle
		if (m_bShowGreenCircle)
		{
			// Create a GDI+ Graphics object from the device context
			Graphics graphics(dc.GetSafeHdc());

			// *** THIS IS THE KEY LINE FOR ANTI-ALIASING ***
			graphics.SetSmoothingMode(SmoothingModeAntiAlias);

			// Define the circle's properties
			int circleCenterX = 100;
			int circleCenterY = 100;
			int circleRadius = 50;

			// Define the bounding rectangle for the circle
			// GDI+ uses RectF (float) or Rect (int)
			Rect circleRect(circleCenterX - circleRadius,
				circleCenterY - circleRadius,
				circleRadius * 2, // Width
				circleRadius * 2); // Height

			// Create a GDI+ green brush and black pen
			// Color is defined as (Alpha, Red, Green, Blue)
			SolidBrush greenBrush(Color(255,36,155,209));

			// Draw the filled ellipse (circle)
			graphics.FillEllipse(&greenBrush, circleRect);

			// GDI+ objects (Brush, Pen) are C++ objects and are automatically
			// destroyed when they go out of scope. No need to select them
			// into a DC or restore old objects.
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