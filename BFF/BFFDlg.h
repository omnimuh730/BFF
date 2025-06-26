// BFFDlg.h : header file
//

#pragma once

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
	afx_msg void OnDestroy(); // <--- ADD THIS LINE
	DECLARE_MESSAGE_MAP()

private:
	CWinThread* m_pCaptureThread = nullptr;
	CWinThread* m_p100msThread = nullptr;

	Gdiplus::Bitmap* m_pCaptureBitmap = nullptr;

	CCriticalSection m_csCaptureBitmap;

	volatile bool m_bStopCaptureThread;
	volatile bool m_bStop100msThread;

	static UINT CaptureThreadProc(LPVOID pParam);
	static UINT _100msThreadProc(LPVOID pParam);

	void StartCaptureThread();
	void StopCaptureThread();

	void Start100msThread();
	void Stop100msThread();

	void UpdateAreaRectEdits(const CRect);
public:
	void OnFindLiveCaptions_Windows();
	void OnFindLiveCaptions_Google();
protected:
	CRect LiveCaptionRect_Windows;
	CRect LiveCaptionRect_Google;
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CEdit v_CaptureRect_Left;
	CEdit v_CaptureRect_Top;
	CEdit v_CaptureRect_Width;
	CEdit v_CaptureRect_Height;
	afx_msg void OnBnClickedRadioWindows();
	afx_msg void OnBnClickedRadioGoogle();
	afx_msg void OnBnClickedRadioArea();
};
