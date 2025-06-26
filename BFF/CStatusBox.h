#pragma once

#include <afxwin.h>

// Simple status box control for displaying status messages
class CStatusBox : public CStatic
{
public:
    CStatusBox() = default;
    virtual ~CStatusBox() = default;

    // Set the status text
    void SetStatusText(const CString& text)
    {
        SetWindowText(text);
    }
};
