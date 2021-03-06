
// MCILGraduationDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "MCILGraduation.h"
#include "MCILGraduationDlg.h"
#include "afxdialogex.h"
#include "PEAlgo.h"
#include "DEAlgo.h"
#include "BitArray.h"
#include <commdlg.h>
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
public:
	afx_msg void OnBnClickedOk();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAboutDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CMCILGraduationDlg dialog



CMCILGraduationDlg::CMCILGraduationDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MCILGRADUATION_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMCILGraduationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMCILGraduationDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMCILGraduationDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMCILGraduationDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CMCILGraduationDlg message handlers


BOOL CMCILGraduationDlg::OnInitDialog()
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

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMCILGraduationDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMCILGraduationDlg::OnPaint()
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
HCURSOR CMCILGraduationDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


CString* fileName;
void CMCILGraduationDlg::OnBnClickedButton1()
{
	
	std::vector<std::string> listImages = { "lena.png","4.1.01.tiff","4.1.05.tiff","4.2.01.tiff","4.2.07.tiff"};
	unsigned int points = 50;
	for each (std::string img in listImages)
	{
		std::string fileP = std::string("C:\\Users\\po4A\\Desktop\\IMPORTANT\\Graduation\\images\\") + img;
		DEAlgo* encode=new DEAlgo(Mat::ones(1,1,CV_8UC1),1);
		PEAlgo* pencode = new PEAlgo(Mat::ones(1, 1, CV_8UC1), 1);
		std::vector<float> Bpp, Delta, PSNR,PE_Bpp,PE_Delta,PE_PSNR;
		for (int i = 0; i <= points; i++)
		{
			float curr = i / (float)points;
			delete encode;
			delete pencode;
			//DE
			encode = new DEAlgo(fileP, curr);
			encode->CalcHighPass();
			encode->DetermineLocations();
			encode->CompressOverFlowMap();
			encode->GetDelta();
			encode->OutterHistogramShift();
			encode->BuildBitStream();
			encode->EmbedBitStream();
			encode->CompileImage();
			Bpp.push_back(encode->getBppRate());
			Delta.push_back(encode->getDeltaValue());
			PSNR.push_back(encode->getPSNR());
			if (curr == 1)
				EEAlgo::toCsv(encode->getMaxCapacity(), img + std::string("MaxCapacity_DE.csv"));
			//PE
			pencode = new PEAlgo(fileP,curr);
			pencode->CalcPE();
			pencode->GetLocations();
			pencode->CompressOverFlowMap();
			pencode->GetDelta();
			pencode->OutterHistogramShift();
			pencode->BuildBitStream();
			pencode->EmbedBitStream();
			pencode->CompileImage();
			PE_Bpp.push_back(pencode->getBppRate());
			PE_Delta.push_back(pencode->getDeltaValue());
			PE_PSNR.push_back(pencode->getPSNR());
			if (curr == 1)
				EEAlgo::toCsv(pencode->getMaxCapacity(), img + std::string("MaxCapacity_PE.csv"));
		}
		EEAlgo::toCsv(Bpp, Delta,img+std::string("_BppToDelta_DE.csv"));
		EEAlgo::toCsv(PSNR, Delta, img + std::string("_PSNRToDelta_DE.csv"));
		EEAlgo::toCsv(PSNR, Bpp, img + std::string("_PSNRToBpp_DE.csv"));

		EEAlgo::toCsv(PE_Bpp, PE_Delta, img + std::string("_BppToDelta_PE.csv"));
		EEAlgo::toCsv(PE_PSNR, PE_Delta, img + std::string("_PSNRToDelta_PE.csv"));
		EEAlgo::toCsv(PE_PSNR, PE_Bpp, img + std::string("_PSNRToBpp_PE.csv"));
	}
}


void CAboutDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}


void CMCILGraduationDlg::OnBnClickedButton2()
{
	CFileDialog* dlg= new CFileDialog(TRUE);
	if (dlg->DoModal() == IDOK)
	{
		fileName=&dlg->GetPathName();
		
	}
}
