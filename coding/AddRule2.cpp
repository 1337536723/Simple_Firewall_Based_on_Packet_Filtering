// AddRuleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "fire.h"
#include "AddRuleDlg.h"
//********************************************************
#include <winsock2.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddRuleDlg dialog


CAddRuleDlg::CAddRuleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddRuleDlg::IDD, pParent)
{
	m_sdadd = _T("");
	m_sdport = _T("");
	m_ssadd = _T("");
	m_ssport = _T("");
	ipFltDrv.LoadDriver("DrvFltIp", NULL, NULL, TRUE);	
}


void CAddRuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddRuleDlg)
	DDX_Control(pDX, IDC_COMBO2, m_protocol);
	DDX_Control(pDX, IDC_COMBO1, m_action);
	DDX_Text(pDX, IDC_DADD, m_sdadd);
	DDV_MaxChars(pDX, m_sdadd, 15);
	DDX_Text(pDX, IDC_DPORT, m_sdport);
	DDX_Text(pDX, IDC_SADD, m_ssadd);
	DDV_MaxChars(pDX, m_ssadd, 15);
	DDX_Text(pDX, IDC_SPORT, m_ssport);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddRuleDlg, CDialog)
	//{{AFX_MSG_MAP(CAddRuleDlg)
	//ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_EN_KILLFOCUS(IDC_SADD, OnKillfocusSadd)
	ON_EN_KILLFOCUS(IDC_DADD, OnKillfocusDadd)
	ON_BN_CLICKED(IDC_ADDSAVE, OnAddsave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

//**************************************************************************
//������IPFilter������ָ��������
DWORD CAddRuleDlg::AddFilter(IPFilter pf)
{

	DWORD result = ipFltDrv.WriteIo(ADD_FILTER, &pf, sizeof(pf));
	if (result != DRV_SUCCESS) 
	{
		AfxMessageBox("Unable to add rule to the driver");
		return FALSE;
	}

	else
		return TRUE;
}

//*************************************************************************
//��������IP��ַ�Ƿ�Ƿ�

BOOL CAddRuleDlg::Verify(CString str)
{
    int pos = 0, prevpos = -1;      // ��ʶ���ַ����еĵ�ǰλ�ú���һλ��
    CString    str1, str2;

    // ����ַ������Ҳ��� . ���IP��ַ�Ƿ�
    if(str.Find('.') == -1)
        return FALSE;

    // ����ַ����а�����ĸ�����IP��ַ�Ƿ�
    if(str.FindOneOf("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") != -1)
        return FALSE;

    // ����ַ����а���������ţ����IP��ַ�Ƿ�
    if(str.FindOneOf("!@#$%^&*()_+|-;:'\"/?><,") != -1)
        return FALSE;

    // �����ĸ���а��� . ��������ȷ��λ�ã����IP��ַ�Ƿ�
    // ��һ�� . �������ַ����±�1~3��
    int _pos = 0;
    _pos = str.Find('.');
    if( (0 >= _pos) || (_pos > 3))
        return FALSE;

    // �ڶ��� . ����ǰһ�� . ��λ�ò�Ӧ����3
    int oldpos = _pos;
    _pos = str.Find('.',_pos+1);
    if( ( oldpos+1 >= _pos) || ( _pos > oldpos + 4) )
        return FALSE;

    // ������ . ����ǰһ�� . ��λ��Ҳ��Ӧ����3
    oldpos = _pos;
    _pos = str.Find('.',_pos+1);
    if( (oldpos+1 >= _pos) || (_pos > oldpos + 4) )
        return FALSE;

    // ������滹�� . ��IP��ַ�Ƿ�
    if(str.Find('.',_pos+1) != -1)
        return FALSE;

    // ������һλ�������֣���IP��ַ�Ƿ�
    str2 = str[str.GetLength()-1];
    if(str2.FindOneOf("0123456789") == -1)
        return FALSE;

    //IP��ַ������Ч��Χ0.0.0.0~255.255.255.255����Ƿ�
    for(int cnt = 0 ; cnt <= 3; cnt++)
    {
        if(cnt < 3)
            pos = str.Find('.',pos+1); //�Ϸ�IP��ַ����3�� . ��
        else
            pos = str.GetLength(); //���һ��û�� . ֱ�����ַ���ĩβ

        str1 = str.Left(pos); // �����ַ�����࿪ʼ��ָ���������ַ�
        str1 = str1.Right(pos-(prevpos+1)); // ��ȡ���ʮ���Ƶ�ǰ�����ʮ������
        unsigned int a = atoi(LPCTSTR(str1));
        if((0 > a)||(a > 255))
            return FALSE;

        prevpos = pos;
    }
    return TRUE;
}

//*****************************************************************

void CAddRuleDlg::OnKillfocusSadd() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	BOOL bresult = Verify(m_ssadd);
	if(bresult == FALSE)
		MessageBox("Invalid IP Address");	
}

void CAddRuleDlg::OnKillfocusDadd() 
{
	// TODO: Add your control notification handler code here
	// This will check wether the IP address you had given
	// corresponds to a valid IP address or not. If not it
	// will prompt you for a valid IP address.

	UpdateData();
    BOOL bresult = Verify(m_sdadd);
	if(bresult == FALSE)
		MessageBox("Invalid IP Address");
	
}

//�������Ĺ���д���ļ�
void CAddRuleDlg::OnAddsave()
{
    UpdateData(); //���ؼ���ֵ��������Ա����
    BOOL    setact;
    int     setproto;
    char ch[30];

    int action = m_action.GetCurSel(); //��ȡAction��ѡ���ֵ
    if(action == 0 ) // 0��ʾAllow
         setact = FALSE;
    else             // 1��ʾDeny
        setact = TRUE;

    int proto = m_protocol.GetCurSel(); //��ȡProtocol��ѡ���ֵ
    if(proto == 0) // 0��ʾICMP
        setproto = 1;
    if(proto == 1) // 1��ʾTCP
        setproto = 6;
    if(proto == 2) // 2��ʾUDP
        setproto = 17;

    wsprintf(ch,"Action: %d, Protocol %d",action,proto);
    MessageBox(ch);

    CString _str;
    char   ch1[100]; // �齨���˹����ַ���
    wsprintf(ch1,"%s,%s,%s,%s,%s,%s,%d,%d\n",
             m_sdadd,
             "255.255.255.255",
             m_sdport,
             m_ssadd,
             "255.255.255.255",
             m_ssport,
             setproto,
             setact);

    if( NewFile()== FALSE)
        MessageBox("unable to create file");

     GotoEnd(); // ���ļ�ָ���ƶ����ļ�β��
    SaveFile(ch1); // ���µĹ��˹������ӵ��ļ�β��

    if(CloseFile() == FALSE)
        MessageBox("Unalbe to close the file");

    IPFilter   ip; // �������˹��򣬽ṹ��Ķ�����DrvFltIp.h��
    ip.destinationIp = inet_addr((LPCTSTR)m_sdadd);
    ip.destinationMask = inet_addr("255.255.255.255");
    ip.destinationPort = htons(atoi((LPCTSTR)m_sdport));
    ip.sourceIp = inet_addr((LPCTSTR)m_ssadd);
    ip.sourceMask = inet_addr("255.255.255.255");
    ip.sourcePort = htons(atoi((LPCTSTR)m_ssport));
    ip.protocol = setproto;
    ip.drop = setact;

    DWORD result = AddFilter(ip); // �ѹ��˹�����������
}

BOOL CAddRuleDlg::NewFile(void)
{
/* This will open an existing file or open a new file if the file
   doesnot exists
*/
	_hFile = CreateFile("saved.rul",
						GENERIC_WRITE | GENERIC_READ,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_ALWAYS,
						NULL,
						NULL);

	/* If unable to obtain the handle values*/
	if(_hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	
	return TRUE;		// File has been opened succesfully
}
//******************************************************
// This will move the file pointer to the end of the file so that 
// it can be easily added to the file

DWORD CAddRuleDlg::GotoEnd(void)
{
	DWORD val;

	DWORD size = GetFileSize(_hFile,NULL);
	if(size == 0)
		return size;
	val = SetFilePointer(_hFile,0,NULL,FILE_END);

	/* If unable to set the file pointer to the end of the file */
 	if(val == 0)
	{
		MessageBox("Unable to set file pointer");
		return GetLastError();
	}
	return val;
}

/* This code will save the data into the file which is given by the parameter*/

DWORD CAddRuleDlg::SaveFile(char*  str)
{
	DWORD   bytesWritten;
	/* Try to write the string passed as parameter to the file and if any 
		error occurs return the appropriate values
	*/
	DWORD	_len =  strlen(str);
	if(WriteFile(_hFile, str, _len, &bytesWritten, NULL) == 0)
	{
		MessageBox("Unalbe to write to the file\n");
		return FALSE;
	}
	return TRUE;
}

/* This function will close the existing file */
BOOL CAddRuleDlg::CloseFile()
{
	if(!_hFile)
	{
	//	MessageBox("File handle does not exist");
		return FALSE;
	}

// if there is an appropriate handle then close it and return app values
    else{
		if(CloseHandle(_hFile) != 0)
		{
			return TRUE;
		}
		else 
			return FALSE;
	}
}