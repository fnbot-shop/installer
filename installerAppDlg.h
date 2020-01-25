// installerAppDlg.h : header file
//

#pragma once
#include <filesystem>

#include "backend/installer.h"

namespace fs = std::filesystem;

enum class INSTALL_STATE : unsigned char {
	WELCOME,
	LICENSE,
	DEST_LOCATION,
	INSTALL,
	COMPLETE
};

// CinstallerAppDlg dialog
class CinstallerAppDlg : public CDialogEx
{
// Construction
public:
	CinstallerAppDlg(CWnd* pParent = nullptr);	// standard constructor

private:
	CBrush m_brush_bg;
	COLORREF m_cr_statictext;
	CBrush m_brush_transparent;
	CFont m_font_installtitle;
	CFont m_font_license;
	INSTALL_STATE m_installstate;
	fs::path m_installpath;
	installer::InstallManager* m_installmanager;

	void UnloadState();
	void LoadState();
	void ChangeState(INSTALL_STATE state);
	void UpdateLicenseCheck();
	void RunInstaller();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INSTALLERAPP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedNext();
	afx_msg void OnBnClickedBack();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnStnClickedStaticAgreementN();
	afx_msg void OnStnClickedStaticAgreementY();
	afx_msg void OnBnClickedRadioAgreementY();
	afx_msg void OnBnClickedRadioAgreementN();
	afx_msg void OnBnClickedInstallBrowse();
	afx_msg LRESULT OnProgressUpdate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnProgressError(WPARAM wParam, LPARAM lParam);
};
