// fireView.cpp : implementation of the CFireView class
//

#include "stdafx.h"
#include "fire.h"
#include "fireDoc.h"
#include "fireView.h"
#include "Sockutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFireView

IMPLEMENT_DYNCREATE(CFireView, CFormView)

BEGIN_MESSAGE_MAP(CFireView, CFormView)
	ON_BN_CLICKED(IDC_ADDRULE, OnAddrule)
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_BN_CLICKED(IDC_BLOCKPING, OnBlockping)
	ON_BN_CLICKED(IDC_BLOCKALL, OnBlockall)
	ON_BN_CLICKED(IDC_ALLOWALL, OnAllowall)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_VIEWRULES, OnViewrules)
	ON_WM_SHOWWINDOW()
	ON_UPDATE_COMMAND_UI(ID_Start, OnUpdateStart)
	ON_COMMAND(ID_STOP, OnStop)
	ON_UPDATE_COMMAND_UI(ID_STOP, OnUpdateStop)
	ON_UPDATE_COMMAND_UI(ID_ALLOWALL, OnUpdateAllowall)
	ON_UPDATE_COMMAND_UI(ID_BLOCKALL, OnUpdateBlockall)
	ON_COMMAND(ID_Start, OnStart)
	ON_COMMAND(ID_BLOCKALL, OnBlockall)
	ON_COMMAND(ID_ALLOWALL, OnAllowall)
	ON_COMMAND(ID_BLOCKPING, OnBlockping)
	ON_UPDATE_COMMAND_UI(ID_BLOCKPING, OnUpdateBlockping)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFireView construction/destruction
//��ʼ������״̬
CFireView::CFireView()
	: CFormView(CFireView::IDD)
{
	//********************************************************
	m_pBrush = new CBrush;
	ASSERT(m_pBrush);
	m_clrBk = RGB(0x01,0x0d,0x18);
	m_clrText = RGB(0x1f,0xa8,0x88);
	m_pBrush->CreateSolidBrush(m_clrBk);
	m_pColumns = new CStringList;
	ASSERT(m_pColumns);
	_rows = 1;
	start = TRUE;
	block = TRUE;
	allow = TRUE;
	ping = TRUE ;
}

CFireView::~CFireView()
{
	if (m_pBrush)
		delete m_pBrush;
}

void CFireView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_RESULT, m_cResult);
	DDX_Control(pDX, IDC_VIEWRULES, m_cvrules);
	DDX_Control(pDX, IDC_BLOCKPING, m_cping);
	DDX_Control(pDX, IDC_BLOCKALL, m_cblockall);
	DDX_Control(pDX, IDC_START, m_cstart);
}

BOOL CFireView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	//*****************************************************************
	
	m_filterDriver.LoadDriver("IpFilterDriver", "System32\\Drivers\\IpFltDrv.sys", NULL, TRUE);

	//we don't deregister the driver at destructor
	m_filterDriver.SetRemovable(FALSE);

	//we load the Filter-Hook Driver
	m_ipFltDrv.LoadDriver("DrvFltIp", NULL, NULL, TRUE);
	//****************************************************************
	return CFormView::PreCreateWindow(cs);
}

void CFireView::OnInitialUpdate()
{
	SetScrollSizes(MM_TEXT, CSize(1000,260)); // �ı�View�Ĵ�С

	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();

	ResizeParentToFit(FALSE); // ���岻��Ҫ��ӦView�Ĵ�С
	
	m_parent = (CMainFrame*)GetParent();
	ShowHeaders();
}

/////////////////////////////////////////////////////////////////////////////
// CFireView diagnostics

#ifdef _DEBUG
void CFireView::AssertValid() const
{
	CFormView::AssertValid();
}

void CFireView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFireView message handlers

void CFireView::OnAddrule() 
{
	// TODO: Add your control notification handler code here
	m_Addrule.DoModal ();	
}


void CFireView::OnStart() 
{
	CString		_text;
	m_cstart.GetWindowText(_text);
	
	//Start��Ӧ�¼�
	if(_text != "Stop" )
	{
		if(m_ipFltDrv.WriteIo(START_IP_HOOK, NULL, 0) != DRV_ERROR_IO)
		{
			MessageBox("Firewall Started Sucessfully");
			start = FALSE;
			m_cstart.SetWindowText("Stop");
			m_parent ->SetOnlineLed(TRUE);
			m_parent ->SetOfflineLed(FALSE);
		}
	}

	//Stop��Ӧ�¼�
	else
	{
		if(m_ipFltDrv.WriteIo(STOP_IP_HOOK, NULL, 0) != DRV_ERROR_IO)
		{
			MessageBox("Firewall Stopped Succesfully");
			m_cstart.SetWindowText("Start");
			start = TRUE;
			m_parent ->SetOnlineLed(FALSE);
			m_parent ->SetOfflineLed(TRUE);
		}
	}	
}

//�������е�ICMP��
void CFireView::OnBlockping() 
{
	// MessageBox("�˹�����Ҫ����ʵ�֣�");
	// Your code
	if(MessageBox("Are you sure to block all Incomming Ping Messages",
				  "Confirm",
				  MB_YESNO) == IDYES)
	{
			IPFilter	 IPflt;

		IPflt.protocol          =1;					// ICMP Protocol
		IPflt.destinationIp		=0;//inet_addr("127.0.0.1");	// all destinations
		IPflt.destinationMask	=0;//inet_addr("255.255.255.255");// all destination masks
		IPflt.destinationPort	=0;					// all ports
		IPflt.sourceIp			=0;					// drop all packets irrespective of source
		IPflt.sourceMask		=0;
		IPflt.sourcePort		=0;					// from any source port
		IPflt.drop				=TRUE;

		m_Addrule.AddFilter(IPflt);
		m_cping.EnableWindow(FALSE);
		ping = FALSE;
		allow = TRUE;
		block = TRUE;
		
	}
}

//�������а�
void CFireView::OnBlockall() 
{
	// MessageBox("�˹�����Ҫ����ʵ�֣�");
	// Your code
	if(MessageBox("This action will prevent any further transer"
					"or recieveing of the data to and from your "
					"computer, Are you sure to proceed with it",
					"WARNING",
					MB_YESNO) == IDYES)
	{

		//	CAddRuleDlg     arule;
		IPFilter	 IPflt;

		IPflt.protocol =		0;					// all the protocols
		IPflt.destinationIp =	0;					// all destinations
		IPflt.destinationMask=  0;					// all destination masks
		IPflt.destinationPort=  0;					// all ports
		IPflt.sourceIp=			0;					// drop all packets irrespective of source
		IPflt.sourceMask=		0;
		IPflt.sourcePort=		0;					// from any source port
		IPflt.drop=				TRUE;

		m_Addrule.AddFilter(IPflt);
		// Disable this button till further notice

		//m_cblockall.SetCheck(0);
		block = FALSE;
		ping = FALSE;
		allow = TRUE;
		m_cblockall.EnableWindow(FALSE);
	
	}
}

//�������а�
void CFireView::OnAllowall() 
{
	// MessageBox("�˹�����Ҫ����ʵ�֣�");
	// Your code
	if(MessageBox("This action will clear all the rules from the firewall",
				  "CONFIRM",
				  MB_YESNO) ==IDYES)
	{
		if(m_ipFltDrv.WriteIo(CLEAR_FILTER,NULL,0) != DRV_ERROR_IO)
		{
			MessageBox("All Rules had been cleared");
			m_cResult.DeleteAllItems();
			m_cping.EnableWindow();
			m_cblockall.EnableWindow();
			m_cvrules.EnableWindow();
			allow = FALSE;
			block = TRUE;
			ping = TRUE;
			_rows	= 1;
		}	
	}
}



//ʹ��ָ�����˹���
BOOL CFireView::ImplementRule(void)
{
	// MessageBox("�˹�����Ҫ����ʵ�֣�");
	// Your code	
	HANDLE	_hFile;
	DWORD	error,nbytesRead;
	char	data;
	CString		_buff = "";
	//	IPFilter	ipf;

	_hFile = CreateFile("saved.rul",				// name of the file
							GENERIC_READ | GENERIC_WRITE,// open as readable and writeable
							FILE_SHARE_READ | FILE_SHARE_WRITE,        // shareaable as read only
							NULL,						
							OPEN_EXISTING,			// open only if it exist
							NULL,
							NULL);

	//if the function fails to open the file check it
	if(_hFile == INVALID_HANDLE_VALUE)
	{
		error = GetLastError();
		MessageBox("Unable to open the file");
		return FALSE;
	}
	
	// if the file succeds than interpret read the file 
	// and interprets the rule and assign them to the firewall
	
	else
	{  
		BOOL bResult;
		
		do{
			/* Read a single byte from the file as we had to look out for the 
			endline. Though this is a bit time consuming one but it saves a lot
			of headache that had been caused if we had used more than one byte
			*/
			bResult = ReadFile(_hFile,&data,1,&nbytesRead,NULL);

			if((data != '\n'))
			{
				_buff  =  _buff + data;
			}
			else
			if((bResult && nbytesRead) !=0)
			{
				//_buff = _buff + '\n';
			//	MessageBox((LPCTSTR)_buff);
				_buff.Remove('\n');
				ParseToIp(_buff);
				_buff = "";
			}
			
			//	_str1 = 
			//	m_redit = _buff;

		}while((bResult && nbytesRead) !=0);

		CloseHandle(_hFile);
	}
	// Close the file if all goes well
	

	return TRUE;
}


//���ַ�������Ϊfilter�ض���ʽ
/* This function will parse a string into the IPFilter form
   It works as a small Lexical Analyzer whose main job is to 
   convert the input string into a parsed string such that the 
   format of the parsed string is in the IPFilter Format.
   For the decalration of IPFilter structure look into the 
   DrvFltIp.h header file
*/
void CFireView:: ParseToIp(CString str)
{
	// Your code, please pay attention to the form of IP address and port!
	//	IPFilter	ip;
	CString		_str[8];
	int		count = 0;
	int		_pos,_prevpos = 0;
	for(; count  < 8; count++)
	{
		if(count < 7)
		{
			_pos	= str.Find(',',_prevpos + 1);
			if((count > 0))
			{
				_str[count] = str.Left(_pos);
				_str[count].Delete(0,_prevpos + 1);
			}
			else{
				if(count == 0)
					_str[count] = str.Left(_pos);
			}
		}
		else
		{
			_str[count] = str.Right(1);
		}
	

		_prevpos  = _pos;
	
	}	

	if(_rows == 1)
	{	

	}
	AddItem(0,0,(LPCTSTR)_str[0]);
	AddItem(0,1,(LPCTSTR)_str[1]);
	AddItem(0,2,(LPCTSTR)_str[2]);
	AddItem(0,3,(LPCTSTR)_str[3]);
	AddItem(0,4,(LPCTSTR)_str[4]);
	AddItem(0,5,(LPCTSTR)_str[5]);


	int	_proto	=	atoi((LPCTSTR)_str[6]);
	CString		proto;
	if(_proto == 0)
		proto = "ANY";
	if(_proto == 1)
		proto = "ICMP";
	if(_proto == 6)
		proto = "TCP";
	if(_proto == 17)
		proto = "UDP";
	AddItem(0,6,((LPCTSTR)proto));

	int _drop = atoi((LPCTSTR)_str[7]);
	if(_drop == 0)
		AddItem(0,7,"ALLOW");
	if(_drop == 1)
		AddItem(0,7,"DENY");

	_rows = _rows + 1;	

	IPFilter	ip1;
	ip1.destinationIp = inet_addr((LPCTSTR)_str[0]);
	ip1.destinationMask = inet_addr((LPCTSTR)_str[1]);
	ip1.sourceIp		= inet_addr((LPCTSTR)_str[3]);
	ip1.sourceMask		= inet_addr((LPCTSTR)_str[4]);
	ip1.sourcePort		= htons(atoi((LPCTSTR)_str[5]));
	ip1.protocol			= atoi((LPCTSTR)_str[6]);


	int		drop;
	drop				= atoi((LPCTSTR)_str[7]);

	if(drop == 0)
	{
		ip1.drop		=	FALSE;
	
	}

	if(drop	== 1)
	{
		ip1.drop		= TRUE;
	
	}	
	
	m_Addrule.AddFilter(ip1);
}


//���ӹ��˹������
BOOL CFireView::AddColumn(LPCTSTR strItem,int nItem,int nSubItem,int nMask,int nFmt)
{
	LV_COLUMN lvc;
	lvc.mask = nMask;
	lvc.fmt = nFmt;
	lvc.pszText = (LPTSTR) strItem;

	if(nItem != 2 && nItem < 5)
		lvc.cx = m_cResult.GetStringWidth(lvc.pszText) + 60;
	else
		lvc.cx = m_cResult.GetStringWidth(lvc.pszText) + 25;

	if(nMask & LVCF_SUBITEM)
	{
		if(nSubItem != -1)
			lvc.iSubItem = nSubItem;
		else
			lvc.iSubItem = nItem;
	}
	return m_cResult.InsertColumn(nItem,&lvc);
}

//���ӹ��˹����һ��Ԫ��
BOOL CFireView::AddItem(int nItem,int nSubItem,LPCTSTR strItem ,int nImageIndex)
{
	LV_ITEM lvItem;
	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = nItem;
	lvItem.iSubItem = nSubItem;
	lvItem.pszText = (LPTSTR) strItem;

	if(nImageIndex != -1)
	{
		lvItem.mask |= LVIF_IMAGE;
		lvItem.iImage |= LVIF_IMAGE;
	}
	if(nSubItem == 0)
		return m_cResult.InsertItem(&lvItem);

	return m_cResult.SetItem(&lvItem);
}

void CFireView::AddHeader(LPTSTR hdr)
{
	if (m_pColumns)
		m_pColumns->AddTail(hdr);
}

void CFireView::ShowHeaders()
{
	int nIndex = 0;
	POSITION pos = m_pColumns->GetHeadPosition();
	while (pos)
	{
		CString hdr = (CString)m_pColumns->GetNext(pos);
		AddColumn(hdr,nIndex++);
	}
}

void CFireView::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CFormView::OnShowWindow(bShow, nStatus);
	AddHeader(_T("Dest IP"));
	AddHeader(_T("Dest MASK"));
	AddHeader(_T("Dest PORT"));
	AddHeader(_T("Source IP"));
	AddHeader(_T("Source MASK"));
	AddHeader(_T("Source PORT"));
	AddHeader(_T("PROTOCOL"));
	AddHeader(_T("ACTION"));
}

void CFireView::OnStop() 
{
	OnStart();	
}

void CFireView::OnUpdateStart(CCmdUI* pCmdUI) 
{	
	// TODO: Add your command update UI handler code here
	pCmdUI ->Enable(start);	
}

void CFireView::OnUpdateStop(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI ->Enable(!start);
}

void CFireView::OnUpdateAllowall(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI ->Enable(allow);
}

void CFireView::OnUpdateBlockall(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI ->Enable(block);
}

void CFireView::OnUpdateBlockping(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI ->Enable(ping);	
}

BOOL CFireView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}
//***********************************************************************

HBRUSH CFireView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
	switch(nCtlColor)
	{
	case CTLCOLOR_BTN:
	case CTLCOLOR_STATIC:
		pDC->SetBkColor(m_clrBk);
		pDC->SetTextColor(m_clrText);
	case CTLCOLOR_DLG:
		return static_cast<HBRUSH>(m_pBrush->GetSafeHandle());
	}
	return CFormView::OnCtlColor(pDC,pWnd,nCtlColor);
}

void CFireView::OnViewrules() 
{
	ImplementRule();	
}