// BFFDlg.h : header file
//

#pragma once

#define WM_HIDE_CIRCLE (WM_USER + 1)

// CBFFDlg dialog
class CBFFDlg : public CDialogEx
{
// Construction
public:
	CBFFDlg(CWnd* pParent = nullptr);	// standard constructor
	~CBFFDlg();	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BFF_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	BOOL m_bShowGreenCircle;

	static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
	static HHOOK m_hKeyboardHook;
	static CBFFDlg* m_pThis;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnHideCircle(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy(); // <--- ADD THIS LINE
	DECLARE_MESSAGE_MAP()

private:
	CWinThread* m_pCaptureThread = nullptr;
	Gdiplus::Bitmap* m_pCaptureBitmap = nullptr;
	CCriticalSection m_csCaptureBitmap;
	volatile bool m_bStopCaptureThread; // <--- DECLARE THE FLAG HERE
	static UINT CaptureThreadProc(LPVOID pParam);
	void StartCaptureThread();
	void StopCaptureThread();
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
