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
	, m_pCaptureThread(nullptr)
	, m_pCaptureBitmap(nullptr)
	, m_bStopCaptureThread(false) // <--- INITIALIZE THE FLAG HERE
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CBFFDlg::~CBFFDlg()
{
	// Clean up the GDI+ bitmap
	if (m_pCaptureBitmap) {
		delete m_pCaptureBitmap;
		m_pCaptureBitmap = nullptr;
	}

	// Also unhook the keyboard hook
	if (m_hKeyboardHook)
	{
		UnhookWindowsHookEx(m_hKeyboardHook);
		m_hKeyboardHook = NULL;
	}
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
	ON_WM_DESTROY() // <--- ADD THIS LINE
	ON_BN_CLICKED(IDOK, &CBFFDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CBFFDlg::OnBnClickedCancel)
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

	StartCaptureThread();

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
		CDialogEx::OnPaint();

		// Create one Graphics object for all drawing
		Graphics graphics(dc.GetSafeHdc());
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);


		// Use a higher quality interpolation mode for better scaling results
		graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);

		// 1. Draw the captured desktop image, scaled to fit the window
		{
			CSingleLock lock(&m_csCaptureBitmap, TRUE);
			if (m_pCaptureBitmap)
			{
				// Get the client area rectangle of our dialog
				CRect clientRect;
				GetClientRect(&clientRect);

				// Create a GDI+ destination rectangle from our client rect
				Gdiplus::Rect destRect(clientRect.left, clientRect.top, clientRect.Width(), clientRect.Height());

				// Draw the image, scaling it from its original size to fit destRect
				graphics.DrawImage(m_pCaptureBitmap, destRect);
			}
		}

		// Draw the green circle as before
		if (m_bShowGreenCircle)
		{
			int circleCenterX = 100;
			int circleCenterY = 100;
			int circleRadius = 50;
			Rect circleRect(circleCenterX - circleRadius,
				circleCenterY - circleRadius,
				circleRadius * 2,
				circleRadius * 2);
			SolidBrush greenBrush(Color(255,36,155,209));
			graphics.FillEllipse(&greenBrush, circleRect);
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

// Add these methods to CBFFDlg

void CBFFDlg::StartCaptureThread()
{
	if (!m_pCaptureThread)
	{
		// Reset the flag before starting the new thread
		m_bStopCaptureThread = false;
		m_pCaptureThread = AfxBeginThread(CaptureThreadProc, this);
	}
}

// BFFDlg.cpp

void CBFFDlg::StopCaptureThread()
{
	if (m_pCaptureThread)
	{
		// 1. Signal the thread to exit its loop
		m_bStopCaptureThread = true;

		// 2. Wait for the thread to actually finish
		//    (A timeout is important to prevent the app from hanging)
		DWORD result = WaitForSingleObject(m_pCaptureThread->m_hThread, 2000); // 2-second timeout

		if (result == WAIT_TIMEOUT)
		{
			// The thread did not stop gracefully. This is a last resort.
			// It can leave resources in an inconsistent state, but it's better than hanging.
			TRACE("Capture thread did not respond, terminating.\n");
			TerminateThread(m_pCaptureThread->m_hThread, -1);
		}

		// 3. The CWinThread object is auto-deleted by MFC after the thread handle closes,
		//    so we just null out our pointer.
		m_pCaptureThread = nullptr;
	}
}

// BFFDlg.cpp

// ADD A NEW MEMBER in BFFDlg.h for the stop flag
// In BFFDlg.h, inside the CBFFDlg class private section:
// volatile bool m_bStopCaptureThread = false;

// In BFFDlg.cpp

UINT CBFFDlg::CaptureThreadProc(LPVOID pParam)
{
	CBFFDlg* pDlg = reinterpret_cast<CBFFDlg*>(pParam);
	if (!pDlg) return 1;

	// Get the DC for the entire screen (the desktop window)
	CWnd* pDesktopWnd = CWnd::GetDesktopWindow();
	CDC* pDesktopDC = pDesktopWnd->GetDC();
	if (!pDesktopDC) return 1;

	// Get the dimensions of the entire virtual screen (for multi-monitor support)
	int desktopLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int desktopTop = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int desktopWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int desktopHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	// Create a compatible DC for our in-memory capture
	CDC memDC;
	memDC.CreateCompatibleDC(pDesktopDC);

	// Make sure you have a thread stop flag in your BFFDlg.h and cpp!
	// volatile bool m_bStopCaptureThread;
	while (!pDlg->m_bStopCaptureThread)
	{
		// Create a bitmap with the full desktop's dimensions
		CBitmap bmp;
		bmp.CreateCompatibleBitmap(pDesktopDC, desktopWidth, desktopHeight);
		CBitmap* pOldBmp = memDC.SelectObject(&bmp);

		// Copy the entire screen content into our bitmap
		memDC.BitBlt(0, 0, desktopWidth, desktopHeight, pDesktopDC, desktopLeft, desktopTop, SRCCOPY);

		// Convert the CBitmap to a GDI+ Bitmap
		Gdiplus::Bitmap* pNewBitmap = Gdiplus::Bitmap::FromHBITMAP((HBITMAP)bmp.GetSafeHandle(), NULL);

		// Store the new bitmap thread-safely
		{
			CSingleLock lock(&pDlg->m_csCaptureBitmap, TRUE);
			if (pDlg->m_pCaptureBitmap)
			{
				delete pDlg->m_pCaptureBitmap; // Delete the old one
			}
			pDlg->m_pCaptureBitmap = pNewBitmap; // Assign the new one
		}

		// Clean up the CBitmap object for this loop iteration
		memDC.SelectObject(pOldBmp);
		bmp.DeleteObject();

		// Invalidate the dialog to trigger OnPaint, but only if it's visible
		if (pDlg->IsWindowVisible())
		{
			pDlg->Invalidate(FALSE);
		}

		// Sleep to prevent high CPU usage (e.g., target ~30 FPS)
		Sleep(33);
	}

	// Clean up the desktop device context when the thread exits
	pDesktopWnd->ReleaseDC(pDesktopDC);
	return 0;
}

void CBFFDlg::OnDestroy()
{
	// Stop the capture thread if it's running
	StopCaptureThread();
	// Call the base class implementation
	CDialogEx::OnDestroy();
	// Unhook the keyboard hook
	if (m_hKeyboardHook)
	{
		UnhookWindowsHookEx(m_hKeyboardHook);
		m_hKeyboardHook = NULL;
	}
}
void CBFFDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}

void CBFFDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}
