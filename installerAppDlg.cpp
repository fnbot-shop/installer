
// installerAppDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "installerApp.h"
#include "installerAppDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static UINT UWM_UPDATE_PROGRESS = RegisterWindowMessage("fns_Install Progress");
static UINT UWM_ERROR_PROGRESS = RegisterWindowMessage("fns_Install Error");

// CinstallerAppDlg dialog
CinstallerAppDlg::CinstallerAppDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_INSTALLERAPP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_cr_statictext = 0x00DCDCDC;
	VERIFY(m_brush_bg.CreateSolidBrush(0x001E1E1E));
	VERIFY(m_brush_transparent.FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));

	m_installstate = INSTALL_STATE::WELCOME;
	m_installmanager = nullptr;
}

void CinstallerAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CinstallerAppDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDNEXT, &CinstallerAppDlg::OnBnClickedNext)
	ON_BN_CLICKED(IDBACK, &CinstallerAppDlg::OnBnClickedBack)
	ON_BN_CLICKED(IDCANCEL, &CinstallerAppDlg::OnBnClickedCancel)
	ON_STN_CLICKED(IDC_STATIC_AGREEMENT_N, &CinstallerAppDlg::OnStnClickedStaticAgreementN)
	ON_STN_CLICKED(IDC_STATIC_AGREEMENT_Y, &CinstallerAppDlg::OnStnClickedStaticAgreementY)
	ON_BN_CLICKED(IDC_RADIO_AGREEMENT_Y, &CinstallerAppDlg::OnBnClickedRadioAgreementY)
	ON_BN_CLICKED(IDC_RADIO_AGREEMENT_N, &CinstallerAppDlg::OnBnClickedRadioAgreementN)
	ON_BN_CLICKED(IDC_INSTALL_BROWSE, &CinstallerAppDlg::OnBnClickedInstallBrowse)
	ON_REGISTERED_MESSAGE(UWM_UPDATE_PROGRESS, &CinstallerAppDlg::OnProgressUpdate)
	ON_REGISTERED_MESSAGE(UWM_ERROR_PROGRESS, &CinstallerAppDlg::OnProgressError)
END_MESSAGE_MAP()


// CinstallerAppDlg message handlers

BOOL CinstallerAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	if (CFont* font = GetFont())
	{
		LOGFONT lf;

		if (font->GetLogFont(&lf))
		{
			lf.lfHeight = 24;
			lf.lfWeight = FW_BOLD;
			m_font_installtitle.CreateFontIndirect(&lf);
		}
	}
	GetDlgItem(IDC_STATIC_TITLE)->SetFont(&m_font_installtitle);
	GetDlgItem(IDC_STATIC_SUBSECTION)->SetFont(&m_font_installtitle);
	CheckDlgButton(IDC_RADIO_AGREEMENT_N, BST_CHECKED);

	if (CFont* font = GetFont())
	{
		LOGFONT lf;

		if (font->GetLogFont(&lf))
		{
			lf.lfHeight = 15;
			strcpy_s(lf.lfFaceName, "Courier New");
			m_font_license.CreateFontIndirect(&lf);
		}
	}
	{
		HGLOBAL hMemory = LoadResource(nullptr, FindResource(nullptr, MAKEINTRESOURCE(IDR_LICENSE), "TEXT"));
		LPVOID ptr = LockResource(hMemory);
		GetDlgItem(IDC_EDIT_LICENSE)->SetWindowText((char*)ptr);
		FreeResource(hMemory);
	}
	GetDlgItem(IDC_EDIT_LICENSE)->SetFont(&m_font_license);
	char* progFiles;
	if (_dupenv_s(&progFiles, nullptr, "PROGRAMFILES")) {
		// couldn't get the env variable
	}
	m_installpath = fs::path(progFiles) / "fnbot.shop";

	{
		CPngImage img;
		img.Load(IDB_PNG1, AfxGetResourceHandle());
		((CStatic*)GetDlgItem(IDC_STATIC_PICTURE))->SetBitmap(img);
	}

	{
		CPngImage img;
		img.Load(IDB_PNG2, AfxGetResourceHandle());
		((CStatic*)GetDlgItem(IDC_STATIC_ICON))->SetBitmap(img);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

HBRUSH CinstallerAppDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch (nCtlColor)
	{
	case CTLCOLOR_STATIC:
	{
		// set text color, transparent back node then 
		pDC->SetTextColor(m_cr_statictext);
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)m_brush_bg;
	}
	case CTLCOLOR_EDIT:
		return (HBRUSH)CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	default:
		return (HBRUSH)m_brush_bg;
	}
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CinstallerAppDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CinstallerAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CinstallerAppDlg::ChangeState(INSTALL_STATE state) {
	UnloadState();
	m_installstate = state;
	LoadState();
}

#define HIDE_DLG(id) GetDlgItem(id)->ShowWindow(SW_HIDE)
#define SHOW_DLG(id) GetDlgItem(id)->ShowWindow(SW_NORMAL)
#define TEXT_DLG(id, txt) GetDlgItem(id)->SetWindowText(txt)
#define ENBL_DLG(id) GetDlgItem(id)->EnableWindow(TRUE)
#define DSBL_DLG(id) GetDlgItem(id)->EnableWindow(FALSE)

void CinstallerAppDlg::UnloadState() {
	switch (m_installstate)
	{
	case INSTALL_STATE::WELCOME:
		HIDE_DLG(IDC_STATIC_PICTURE);
		HIDE_DLG(IDC_STATIC_WELCOME_DESC);
		HIDE_DLG(IDC_STATIC_TITLE);
		SHOW_DLG(IDBACK);
		break;
	case INSTALL_STATE::LICENSE:
		HIDE_DLG(IDC_STATIC_SUBSECTION);
		HIDE_DLG(IDC_STATIC_ICON);
		HIDE_DLG(IDC_EDIT_LICENSE);
		HIDE_DLG(IDC_STATIC_DESCRIPTION);
		HIDE_DLG(IDC_RADIO_AGREEMENT_Y);
		HIDE_DLG(IDC_STATIC_AGREEMENT_Y);
		HIDE_DLG(IDC_RADIO_AGREEMENT_N);
		HIDE_DLG(IDC_STATIC_AGREEMENT_N);
		ENBL_DLG(IDNEXT);
		break;
	case INSTALL_STATE::DEST_LOCATION:
		HIDE_DLG(IDC_STATIC_SUBSECTION);
		HIDE_DLG(IDC_STATIC_ICON);
		HIDE_DLG(IDC_STATIC_DESCRIPTION);
		HIDE_DLG(IDC_EDIT_BROWSE);
		HIDE_DLG(IDC_INSTALL_BROWSE);
		TEXT_DLG(IDNEXT, "Next >");
		break;
	case INSTALL_STATE::INSTALL:
		HIDE_DLG(IDC_STATIC_SUBSECTION);
		HIDE_DLG(IDC_STATIC_ICON);
		HIDE_DLG(IDC_STATIC_DESCRIPTION);
		HIDE_DLG(IDC_STATIC_PROGRESS);
		HIDE_DLG(IDC_PROGRESS);
		break;
	case INSTALL_STATE::COMPLETE:
	{
		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;
		if (CreateProcess(NULL, (char*)m_installmanager->startExe.string().c_str(), NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, NULL, &si, &pi)) {
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		DestroyWindow();
		break;
	}
	}
}

void CinstallerAppDlg::LoadState() {
	switch (m_installstate)
	{
	case INSTALL_STATE::WELCOME:
		SHOW_DLG(IDC_STATIC_PICTURE);
		SHOW_DLG(IDC_STATIC_WELCOME_DESC);
		SHOW_DLG(IDC_STATIC_TITLE);
		HIDE_DLG(IDBACK);
		break;
	case INSTALL_STATE::LICENSE:
		SHOW_DLG(IDC_STATIC_SUBSECTION);
		SHOW_DLG(IDC_STATIC_ICON);
		SHOW_DLG(IDC_EDIT_LICENSE);
		SHOW_DLG(IDC_STATIC_DESCRIPTION);
		SHOW_DLG(IDC_RADIO_AGREEMENT_Y);
		SHOW_DLG(IDC_STATIC_AGREEMENT_Y);
		SHOW_DLG(IDC_RADIO_AGREEMENT_N);
		SHOW_DLG(IDC_STATIC_AGREEMENT_N);
		TEXT_DLG(IDC_STATIC_SUBSECTION, "License Agreement");
		TEXT_DLG(IDC_STATIC_DESCRIPTION, "In order to use fnbot.shop, you must agree to the following license.");
		if (IsDlgButtonChecked(IDC_RADIO_AGREEMENT_Y) == BST_CHECKED) {
			ENBL_DLG(IDNEXT);
		}
		else {
			DSBL_DLG(IDNEXT);
		}
		break;
	case INSTALL_STATE::DEST_LOCATION:
		SHOW_DLG(IDC_STATIC_SUBSECTION);
		SHOW_DLG(IDC_STATIC_ICON);
		SHOW_DLG(IDC_STATIC_DESCRIPTION);
		SHOW_DLG(IDC_EDIT_BROWSE);
		SHOW_DLG(IDC_INSTALL_BROWSE);
		TEXT_DLG(IDC_STATIC_SUBSECTION, "Select Install Location");
		TEXT_DLG(IDC_STATIC_DESCRIPTION, "Select the folder you want to install fnbot.shop to.");
		TEXT_DLG(IDC_EDIT_BROWSE, m_installpath.string().c_str());
		TEXT_DLG(IDNEXT, "Install");
		break;
	case INSTALL_STATE::INSTALL:
		SHOW_DLG(IDC_STATIC_SUBSECTION);
		SHOW_DLG(IDC_STATIC_ICON);
		SHOW_DLG(IDC_STATIC_DESCRIPTION);
		SHOW_DLG(IDC_STATIC_PROGRESS);
		SHOW_DLG(IDC_PROGRESS);
		TEXT_DLG(IDC_STATIC_SUBSECTION, "Installing");
		TEXT_DLG(IDC_STATIC_DESCRIPTION, "Installing fnbot.shop for you, please wait...");
		TEXT_DLG(IDNEXT, "Done");
		DSBL_DLG(IDNEXT);
		HIDE_DLG(IDBACK);
		RunInstaller();
		break;
	case INSTALL_STATE::COMPLETE:
		SHOW_DLG(IDC_STATIC_SUBSECTION);
		SHOW_DLG(IDC_STATIC_ICON);
		SHOW_DLG(IDC_STATIC_DESCRIPTION);
		TEXT_DLG(IDC_STATIC_SUBSECTION, "Done");
		TEXT_DLG(IDC_STATIC_DESCRIPTION, "You're done! fnbot.shop will now start and initialize for you.");
		TEXT_DLG(IDNEXT, "Finish");
		break;
	}
}

void CinstallerAppDlg::OnBnClickedNext()
{
	ChangeState((INSTALL_STATE)((unsigned char)m_installstate + 1));
}
void CinstallerAppDlg::OnBnClickedBack()
{
	ChangeState((INSTALL_STATE)((unsigned char)m_installstate - 1));
}


void CinstallerAppDlg::OnBnClickedCancel()
{
	int returnId = MessageBox("fnbot.shop isn't installed yet.\n\nExit installation?", "Exit Setup", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
	if (returnId == IDYES) {
		CDialogEx::OnCancel();
	}
}


void CinstallerAppDlg::OnStnClickedStaticAgreementN()
{
	CheckDlgButton(IDC_RADIO_AGREEMENT_N, BST_CHECKED);
	CheckDlgButton(IDC_RADIO_AGREEMENT_Y, BST_UNCHECKED);
	UpdateLicenseCheck();
}


void CinstallerAppDlg::OnStnClickedStaticAgreementY()
{
	CheckDlgButton(IDC_RADIO_AGREEMENT_Y, BST_CHECKED);
	CheckDlgButton(IDC_RADIO_AGREEMENT_N, BST_UNCHECKED);
	UpdateLicenseCheck();
}

void CinstallerAppDlg::UpdateLicenseCheck() {
	if (IsDlgButtonChecked(IDC_RADIO_AGREEMENT_Y) == BST_CHECKED) {
		ENBL_DLG(IDNEXT);
	}
	else {
		DSBL_DLG(IDNEXT);
	}
}

void CinstallerAppDlg::OnBnClickedRadioAgreementY()
{
	UpdateLicenseCheck();
}

void CinstallerAppDlg::OnBnClickedRadioAgreementN()
{
	UpdateLicenseCheck();
}

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}
	return 0;
}

void CinstallerAppDlg::OnBnClickedInstallBrowse()
{
	fs::path fs_path;
	{
		BROWSEINFO a = { 0 };
		a.hwndOwner = GetSafeHwnd();
		a.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
		a.lpfn = BrowseCallbackProc;
		a.lParam = (LPARAM)m_installpath.c_str();
		LPITEMIDLIST pidl = SHBrowseForFolder(&a);
		if (!pidl) {
			return;
		}

		TCHAR path[MAX_PATH];
		SHGetPathFromIDList(pidl, path);
		fs_path = fs::path(path);

		IMalloc* imalloc;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(pidl);
			imalloc->Release();
		}
	}
	if (fs::is_directory(fs_path)) {
		m_installpath = fs_path / "fnbot.shop";
	}
	GetDlgItem(IDC_EDIT_BROWSE)->SetWindowText(m_installpath.string().c_str());
}

static void download_callback(const installer::INSTALL_INFO* info) {
	PostMessage((HWND)info->lParam, UWM_UPDATE_PROGRESS, 0, 0);
}
LRESULT CinstallerAppDlg::OnProgressUpdate(WPARAM wParam, LPARAM lParam)
{
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetPos(int(m_installmanager->info->progress * 1000));
	LPCTSTR status;
	switch (m_installmanager->info->state)
	{
	case installer::STATE::INIT_PINNING:
		status = "Initializing downloader";
		break;
	case installer::STATE::INIT_STREAMS:
		status = "Initializing streams";
		break;
	case installer::STATE::INIT_DOWNLOAD:
		status = "Initializing download";
		break;
	case installer::STATE::DOWNLOAD:
	{
		status = new char[100];
		sprintf((char*)status, "Downloading... (%s)", m_installmanager->info->state_info);
		break;
	}
	case installer::STATE::REGISTRY:
		status = "Adding registry entries";
		break;
	case installer::STATE::SHORTCUT:
		status = "Creating start menu shortcut";
		break;
	case installer::STATE::DONE:
		status = "Done!";
		ENBL_DLG(IDNEXT);
		break;
	default:
		return 0;
	}
	GetDlgItem(IDC_STATIC_PROGRESS)->SetWindowText(status);
	if (m_installmanager->info->state == installer::STATE::DOWNLOAD) {
		delete[] status;
	}
	return 0;
}

static void error_callback(void* lParam, const char* error) {
	char* err = new char[strlen(error)];
	strcpy(err, error);
	PostMessage((HWND)lParam, UWM_UPDATE_PROGRESS, 0, (LPARAM)error);
}
LRESULT CinstallerAppDlg::OnProgressError(WPARAM wParam, LPARAM lParam) {
	MessageBox((char*)lParam, "fnbot.shop Error", MB_ICONERROR);
	GetDlgItem(IDC_STATIC_PROGRESS)->SetWindowText((char*)lParam);
	delete[](char*)lParam;
	DestroyWindow();
	return 0;
}

void CinstallerAppDlg::RunInstaller() {
	((CProgressCtrl*)GetDlgItem(IDC_PROGRESS))->SetRange(0, 1000);

	installer::INSTALL_INFO* info = new installer::INSTALL_INFO;
	info->install_location = m_installpath;
	info->lParam = GetSafeHwnd();
	info->callback = download_callback;
	info->error_callback = error_callback;
	m_installmanager = new installer::InstallManager(info);
	m_installmanager->Run();
}