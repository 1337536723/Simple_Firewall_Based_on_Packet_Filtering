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
    ON_BN_CLICKED(IDC_DELRULE, OnDelrule)
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
//初始化界面状态
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
    _rows = 0;
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
    SetScrollSizes(MM_TEXT, CSize(1000,260)); // 改变View的大小

    CFormView::OnInitialUpdate();
    GetParentFrame()->RecalcLayout();

    ResizeParentToFit(FALSE); // 窗体不需要适应View的大小

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
    m_Addrule.DoModal ();   // 显示添加规则的对话框
}

void CFireView::OnDelrule()
{
    // TODO: Add your control notification handler code here
    if(MessageBox("Are you sure to delete the last filter?",
                  "Confirm",
                  MB_YESNO) == IDYES)
    {
        HANDLE  _hFile;
        DWORD   error,nbytesRead;
        char    data;
        CString     _buff = "";

        // 打开本地存储文件
        _hFile = CreateFile("saved.rul",                             // filename
                            GENERIC_READ | GENERIC_WRITE,        // open as readable and writeable
                            FILE_SHARE_READ | FILE_SHARE_WRITE,  // shareaable as read only
                            NULL,
                            OPEN_EXISTING,                       // open only if it exist
                            NULL,
                            NULL);

        if(_hFile == INVALID_HANDLE_VALUE)
        {   // 若无法打开文件则返回错误
            error = GetLastError();
            MessageBox("Unable to open the file");
        }

        else
        {   // 否则开始读取文件
            BOOL bResult;
            int countRules = 0;
            CString fullContent[999];
            // 仅当ReadFile成功时bResult不为0
            // 仅当读取到至少1个byte时nbytesRead不为0
            do{
                /* 每次读取一个byte，如果不是换行符，则把它添加到字符串中
                   否则，检查是否读取成功，是即表示读取到换行符，把当前的
                   字符串传递给ParseToIp函数，然后清空。
                */
                bResult = ReadFile(_hFile,&data,1,&nbytesRead,NULL);

                if(data != '\n')
                {
                    _buff  =  _buff + data;
                }
                else if((bResult && nbytesRead) !=0)
                {
                    _buff  =  _buff + '\n';
                    fullContent[countRules] = _buff;
                    countRules ++;
                    _buff = "";
                }
            }while((bResult && nbytesRead) !=0);

            CloseHandle(_hFile); // 关闭文件

            DeleteFile("saved.rul"); // 删除文件

            if( m_Addrule.NewFile()== FALSE) // 建立文件
                MessageBox("unable to create file");

            // 将除最后一条规则以外的所有规则逐条存入本地文件
            int i = 0;
            for(; i<countRules-1; i++)
            {
                m_Addrule.GotoEnd();
                m_Addrule.SaveFile(fullContent[i].GetBuffer(0));
            }

            if(m_Addrule.CloseFile() == FALSE)
                MessageBox("Unalbe to close the file");
            else
                MessageBox("Delete OK. Now click Allow All to clean the list and click View rules to see the changes.");
        }
    }
}

void CFireView::OnStart()
{
    CString     _text;
    m_cstart.GetWindowText(_text);

    //Start响应事件
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

    //Stop响应事件
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

//禁用所有的ICMP包
void CFireView::OnBlockping()
{
    if(MessageBox("Are you sure to block all Incomming Ping Messages",
                  "Confirm",
                  MB_YESNO) == IDYES)
    {
        //构建一条过滤所有ICMP包的规则
        IPFilter     IPflt;
        IPflt.protocol          =1;                 // ICMP协议
        IPflt.destinationIp     =0;                 // 到达任何IP
        IPflt.destinationMask   =0;
        IPflt.destinationPort   =0;                 // 到达任何端口
        IPflt.sourceIp          =0;                 // 来自任何IP
        IPflt.sourceMask        =0;
        IPflt.sourcePort        =0;                 // 来自任何端口
        IPflt.drop              =TRUE;              // 动作是丢包

        // 利用AddRuleDlg的AddFilter把过滤规则发送至驱动
        m_Addrule.AddFilter(IPflt);

        // 更新界面
        ping = FALSE;
        allow = TRUE;
        block = TRUE;

        m_cping.EnableWindow(FALSE); // 禁用Block Ping按钮
    }
}

//禁用所有包
void CFireView::OnBlockall()
{
    if(MessageBox("This action will prevent any further transer"
                    "or recieveing of the data to and from your "
                    "computer, Are you sure to proceed with it",
                    "WARNING",
                    MB_YESNO) == IDYES)
    {

        // 构建一条过滤所有IP包的规则
        IPFilter     IPflt;
        IPflt.protocol =        0;                  // 任何协议
        IPflt.destinationIp =   0;                  // 到达任何IP
        IPflt.destinationMask=  0;
        IPflt.destinationPort=  0;                  // 到达任何端口
        IPflt.sourceIp=         0;                  // 来自任何IP
        IPflt.sourceMask=       0;
        IPflt.sourcePort=       0;                  // 来自任何端口
        IPflt.drop=             TRUE;               // 动作是丢包

        // 利用AddRuleDlg的AddFilter把过滤规则发送至驱动
        m_Addrule.AddFilter(IPflt);

        // 更新界面
        block = FALSE;
        ping = FALSE;
        allow = TRUE;

        m_cblockall.EnableWindow(FALSE); // 禁用Block All按钮
    }
}

//启用所有包
void CFireView::OnAllowall()
{
    if(MessageBox("This action will clear all the rules from the firewall",
                  "CONFIRM",
                  MB_YESNO) ==IDYES)
    {   // 向驱动发送清除过滤规则的CTL_CODE
        if(m_ipFltDrv.WriteIo(CLEAR_FILTER,NULL,0) != DRV_ERROR_IO)
        {
            MessageBox("All Rules had been cleared");
            m_cResult.DeleteAllItems(); //清空规则列表

            // 激活主界面Dialog的button控件，允许其接收鼠标输入
            m_cping.EnableWindow();      //Block All按钮
            m_cblockall.EnableWindow();  //Block Ping按钮
            m_cvrules.EnableWindow();    //View Rules按钮

            // 更新界面
            allow = FALSE; // 禁用Allow All的界面项，图标变灰。
            block = TRUE;  // 启用Block All的界面项
            ping = TRUE;   // 启用Block Ping的界面项

            _rows   = 0; //规则列表行数归零
        }
    }
}



//使用指定过滤规则
BOOL CFireView::ImplementRule(void)
{
    HANDLE  _hFile;
    DWORD   error,nbytesRead;
    char    data;
    CString     _buff = "";

    // 打开本地存储文件
    _hFile = CreateFile("saved.rul",                             // filename
                            GENERIC_READ | GENERIC_WRITE,        // open as readable and writeable
                            FILE_SHARE_READ | FILE_SHARE_WRITE,  // shareaable as read only
                            NULL,
                            OPEN_EXISTING,                       // open only if it exist
                            NULL,
                            NULL);

    if(_hFile == INVALID_HANDLE_VALUE)
    {   // 若无法打开文件则返回错误
        error = GetLastError();
        MessageBox("Unable to open the file");
        return FALSE;
    }

    else
    {   // 否则开始读取文件
        BOOL bResult;

        // 仅当ReadFile成功时bResult不为0
        // 仅当读取到至少1个byte时nbytesRead不为0
        do{
            /* 每次读取一个byte，如果不是换行符，则把它添加到字符串中
               否则，检查是否读取成功，是即表示读取到换行符，把当前的
               字符串传递给ParseToIp函数，然后清空。
            */
            bResult = ReadFile(_hFile,&data,1,&nbytesRead,NULL);

            if(data != '\n')
            {
                _buff  =  _buff + data;
            }
            else if((bResult && nbytesRead) !=0)
            {
                ParseToIp(_buff);
                _buff = "";
            }
        }while((bResult && nbytesRead) !=0);

        CloseHandle(_hFile); // 关闭文件
    }

    return TRUE;
}


//将字符串解析为filter特定格式
/* This function will parse a string into the IPFilter form
   It works as a small Lexical Analyzer whose main job is to
   convert the input string into a parsed string such that the
   format of the parsed string is in the IPFilter Format.
   For the decalration of IPFilter structure look into the
   DrvFltIp.h header file
*/
void CFireView:: ParseToIp(CString str)
{
    CString     _str[8];
    int     count = 0;
    int     _pos,_prevpos = 0;
    for(; count  < 8; count++)
    {   // 获取各个字段的值
        if(count < 7)
        {
            _pos    = str.Find(',',_prevpos + 1);
            if(count > 0)
            {
                _str[count] = str.Left(_pos);
                _str[count].Delete(0,_prevpos + 1);
            }
            else if(count == 0)
                    _str[count] = str.Left(_pos);
        }
        else
            _str[count] = str.Right(1); //最后一位表示操作

        _prevpos  = _pos;
    }

    // 把新的规则加载到界面List中
    // 行数_rows从0开始(第一行行号为0)
    AddItem(_rows,0,(LPCTSTR)_str[0]);
    AddItem(_rows,1,(LPCTSTR)_str[1]);
    AddItem(_rows,2,(LPCTSTR)_str[2]);
    AddItem(_rows,3,(LPCTSTR)_str[3]);
    AddItem(_rows,4,(LPCTSTR)_str[4]);
    AddItem(_rows,5,(LPCTSTR)_str[5]);

    // 解析协议内容
    int _proto  =   atoi((LPCTSTR)_str[6]);
    CString     proto;
    if(_proto == 0)
        proto = "ANY";
    if(_proto == 1)
        proto = "ICMP";
    if(_proto == 6)
        proto = "TCP";
    if(_proto == 17)
        proto = "UDP";
    AddItem(_rows,6,((LPCTSTR)proto));

    // 解析动作
    int _drop = atoi((LPCTSTR)_str[7]);
    if(_drop == 0)
        AddItem(_rows,7,"ALLOW");
    if(_drop == 1)
        AddItem(_rows,7,"DENY");

    _rows = _rows + 1; //规则列表行数+1

    // 构造对应的过滤规则
    IPFilter    ip1;
    ip1.destinationIp   = inet_addr((LPCTSTR)_str[0]);
    ip1.destinationMask = inet_addr((LPCTSTR)_str[1]);
    ip1.destinationPort = htons(atoi((LPCTSTR)_str[2]));
    ip1.sourceIp        = inet_addr((LPCTSTR)_str[3]);
    ip1.sourceMask      = inet_addr((LPCTSTR)_str[4]);
    ip1.sourcePort      = htons(atoi((LPCTSTR)_str[5]));
    ip1.protocol        = atoi((LPCTSTR)_str[6]);
    if(_drop == 0)
        ip1.drop        =   FALSE;
    if(_drop == 1)
        ip1.drop        = TRUE;

    //利用AddRuleDlg的AddFilter把过滤规则发送至驱动
    m_Addrule.AddFilter(ip1);

    m_cvrules.EnableWindow(FALSE);    //禁用View Rules按钮

    _rows   = 0; //规则列表行数归零
}


//增加过滤规则表列
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

//增加过滤规则表一个元素
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
