// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	

#include "FileHelper.h"
#include "MenuWrapper.h"

#include "extend.ctrls/imre/ImgProvider.h"

#define  MAX_FILESIZE			4*1024*1024			//可发送文件最大限制
#define  MAX_IMAGESIZE			4*1024*1024			//图片最大限制
	
CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited = FALSE;

	m_bSnapshotHideWindow = false;
}

CMainDlg::~CMainDlg()
{
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetMsgHandled(FALSE);
	return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	m_bLayoutInited = TRUE;
	//SShellNotifyIcon *notify = FindChildByID2<SShellNotifyIcon>(110);
	//notify->ShowNotify(L"Hello SOUI",L"这可能是一个提示");

	if (!ImageProvider::IsExist(L"default_portrait"))
	{
		SAntialiasSkin* pSkin = new SAntialiasSkin();
		pSkin->SetRound(TRUE);

		if (pSkin->LoadFromFile(L"default_res\\default_portrait.png"))
			ImageProvider::Insert(L"default_portrait", pSkin);
		else
			delete pSkin;
	}

	::DragAcceptFiles(m_hWnd, TRUE);
	::RegisterDragDrop(m_hWnd, GetDropTarget());

	SImageButton* pBtnMessage = FindChildByName2<SImageButton>(L"btn_message");
	SImageButton* pBtnContact = FindChildByName2<SImageButton>(L"btn_contact");
	SImageButton* pBtnFavorites = FindChildByName2<SImageButton>(L"btn_favorites");
	SASSERT(pBtnMessage);
	SASSERT(pBtnContact);
	SASSERT(pBtnFavorites);

	pBtnMessage->SetCheck(TRUE);
	pBtnContact->SetCheck(FALSE);
	pBtnFavorites->SetCheck(FALSE);

	SListView* pLasttalkList = FindChildByName2<SListView>(L"lv_list_lasttalk");
	SASSERT(pLasttalkList);
	pLasttalkList->EnableScrollBar(SSB_HORZ, FALSE);

	m_pAdapterLasttalk = new CAdapter_MessageList(pLasttalkList, this);
	pLasttalkList->SetAdapter(m_pAdapterLasttalk);
	m_pAdapterLasttalk->Release();


	STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
	SASSERT(pChatTab);
	//添加测试数据
	m_pAdapterLasttalk->AddItem(0, "1001");
	//添加聊天页面
	{
		SStringW sstrPage;
		sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_COMMON_FILEHELPER'/></page>", L"1001");
		pChatTab->InsertItem(sstrPage);

		SWindow* pPage = pChatTab->GetPage(L"1001", TRUE);
		SASSERT(pPage);
		SImRichEdit* pRecvRichedit = pPage->FindChildByName2<SImRichEdit>(L"recv_richedit");
		SImRichEdit* pSendRichedit = pPage->FindChildByName2<SImRichEdit>(L"send_richedit");
		SASSERT(pRecvRichedit);
		SASSERT(pSendRichedit);

		SUBSCRIBE(pSendRichedit, EVT_RE_OBJ, CMainDlg::OnSendRichEditObjEvent);
		SUBSCRIBE(pSendRichedit, EVT_RE_NOTIFY, CMainDlg::OnSendRichEditEditorChange);
		SUBSCRIBE(pSendRichedit, EVT_RE_QUERY_ACCEPT, CMainDlg::OnSendRichEditAcceptData);
		SUBSCRIBE(pSendRichedit, EVT_CTXMENU, CMainDlg::OnSendRichEditMenu);

		SUBSCRIBE(pRecvRichedit, EVT_RE_OBJ, CMainDlg::OnRecvRichEditObjEvent);
		SUBSCRIBE(pRecvRichedit, EVT_RE_SCROLLBAR, CMainDlg::OnRecvRichEditScrollEvent);
		SUBSCRIBE(pRecvRichedit, EVT_RE_QUERY_ACCEPT, CMainDlg::OnRecvRichEditAcceptData);
		SUBSCRIBE(pRecvRichedit, EVT_CTXMENU, CMainDlg::OnRecvRichEditMenu);
	}

	m_pAdapterLasttalk->AddItem(1, "2001");
	//添加聊天页面
	{
		SStringW sstrPage;
		sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_COMMON_PERSONAL'/></page>", L"2001");
		pChatTab->InsertItem(sstrPage);
	}

	m_pAdapterLasttalk->AddItem(2, "3001");
	//添加聊天页面
	{
		SStringW sstrPage;
		sstrPage.Format(L"<page title='%s'><include src='layout:XML_PAGE_COMMON_GROUP'/></page>", L"3001");
		pChatTab->InsertItem(sstrPage);
	}
	m_pAdapterLasttalk->AddItem(3, "4001");
	m_pAdapterLasttalk->AddItem(4, "5001");
	m_pAdapterLasttalk->AddItem(5, "6001");


	SStatic* pCurName = FindChildByName2<SStatic>(L"page_name");
	SASSERT(pCurName);
	pCurName->SetVisible(FALSE);

	SImageButton* pImgBtnMore = FindChildByName2<SImageButton>(L"btn_more");
	SASSERT(pImgBtnMore);
	pImgBtnMore->SetVisible(FALSE);


	STreeView * pTreeView = FindChildByName2<STreeView>("tv_Friend");
	if (pTreeView)
	{
		m_pTreeViewAdapter = new CFriendTreeViewAdapter();
		pTreeView->SetAdapter(m_pTreeViewAdapter);
		m_pTreeViewAdapter->Release();
	}
	return 0;
}
//TODO:消息映射
void CMainDlg::OnClose()
{
	CSimpleWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	if (!m_bLayoutInited) return;
	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}


//演示如何响应菜单事件
void CMainDlg::OnCommand(UINT uNotifyCode, int nID, HWND wndCtl)
{
	if (uNotifyCode == 0)
	{
		switch (nID)
		{
		case 6:
			PostMessage(WM_CLOSE);
			break;
		default:
			break;
		}
	}
}


void CMainDlg::OnBnClickMessage()
{
	SImageButton* pBtnMessage = FindChildByName2<SImageButton>(L"btn_message");
	SImageButton* pBtnContact = FindChildByName2<SImageButton>(L"btn_contact");
	SImageButton* pBtnFavorites = FindChildByName2<SImageButton>(L"btn_favorites");
	SASSERT(pBtnMessage);
	SASSERT(pBtnContact);
	SASSERT(pBtnFavorites);

	pBtnMessage->SetCheck(TRUE);
	pBtnContact->SetCheck(FALSE);
	pBtnFavorites->SetCheck(FALSE);

	STabCtrl* pLeftListTab = FindChildByName2<STabCtrl>(L"leftlist_tabctrl");
	SASSERT(pLeftListTab);
	pLeftListTab->SetCurSel(L"lasttalk_page", TRUE);

	if ("" != m_LasttalkCurSel.m_strID)
	{
		SStatic* pCurName = FindChildByName2<SStatic>(L"page_name");
		SASSERT(pCurName);
		pCurName->SetVisible(TRUE);
		pCurName->Invalidate();
	}

	if (3 == m_LasttalkCurSel.m_nType ||
		4 == m_LasttalkCurSel.m_nType ||
		5 == m_LasttalkCurSel.m_nType)
	{

		SImageButton* pImgBtnMore = FindChildByName2<SImageButton>(L"btn_more");
		SASSERT(pImgBtnMore);
		pImgBtnMore->SetVisible(FALSE);
		pImgBtnMore->Invalidate();
	}
}

void CMainDlg::OnBnClickContact()
{
	SImageButton* pBtnMessage = FindChildByName2<SImageButton>(L"btn_message");
	SImageButton* pBtnContact = FindChildByName2<SImageButton>(L"btn_contact");
	SImageButton* pBtnFavorites = FindChildByName2<SImageButton>(L"btn_favorites");
	SASSERT(pBtnMessage);
	SASSERT(pBtnContact);
	SASSERT(pBtnFavorites);

	pBtnMessage->SetCheck(FALSE);
	pBtnContact->SetCheck(TRUE);
	pBtnFavorites->SetCheck(FALSE);

	STabCtrl* pLeftListTab = FindChildByName2<STabCtrl>(L"leftlist_tabctrl");
	SASSERT(pLeftListTab);
	pLeftListTab->SetCurSel(L"contact_page", TRUE);

	STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
	SASSERT(pChatTab);
	pChatTab->SetCurSel(0);

	SStatic* pCurName = FindChildByName2<SStatic>(L"page_name");
	SASSERT(pCurName);
	pCurName->SetVisible(FALSE);
	pCurName->Invalidate();

	SImageButton* pImgBtnMore = FindChildByName2<SImageButton>(L"btn_more");
	SASSERT(pImgBtnMore);
	pImgBtnMore->SetVisible(FALSE);
	pImgBtnMore->Invalidate();
}

void CMainDlg::OnBnClickFavorites()
{
	SImageButton* pBtnMessage = FindChildByName2<SImageButton>(L"btn_message");
	SImageButton* pBtnContact = FindChildByName2<SImageButton>(L"btn_contact");
	SImageButton* pBtnFavorites = FindChildByName2<SImageButton>(L"btn_favorites");
	SASSERT(pBtnMessage);
	SASSERT(pBtnContact);
	SASSERT(pBtnFavorites);

	pBtnMessage->SetCheck(FALSE);
	pBtnContact->SetCheck(FALSE);
	pBtnFavorites->SetCheck(TRUE);

	STabCtrl* pLeftListTab = FindChildByName2<STabCtrl>(L"leftlist_tabctrl");
	SASSERT(pLeftListTab);
	pLeftListTab->SetCurSel(L"favorites_page", TRUE);

	STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
	SASSERT(pChatTab);
	pChatTab->SetCurSel(0);

	SStatic* pCurName = FindChildByName2<SStatic>(L"page_name");
	SASSERT(pCurName);
	pCurName->SetVisible(FALSE);
	pCurName->Invalidate();

	SImageButton* pImgBtnMore = FindChildByName2<SImageButton>(L"btn_more");
	SASSERT(pImgBtnMore);
	pImgBtnMore->SetVisible(FALSE);
	pImgBtnMore->Invalidate();
}

void CMainDlg::OnBnClickMenu()
{
	//TODO:
}

void CMainDlg::MessageListItemClick(int nType, const std::string& strID)
{
	STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
	SASSERT(pChatTab);

	SStringW sstrID = S_CA2W(strID.c_str());
	pChatTab->SetCurSel(sstrID, TRUE);

	SStatic* pCurName = FindChildByName2<SStatic>(L"page_name");
	SASSERT(pCurName);

	SStringW sstrName = L"";
	switch (nType)
	{
	case 0://filehelper
		sstrName = L"SOUI文件传输助手";
		break;
	case 1://personal
		sstrName = L"SOUI个人好友名";
		break;
	case 2://group
		sstrName = L"SOUI群聊名";
		break;
	case 3://订阅号
		sstrName = L"SOUI订阅号";
		break;
	case 4://新闻
		sstrName = L"SOUI新闻";
		break;
	case 5://公众号
		sstrName = L"SOUI公众号";
		break;
	default:
		break;
	}
	pCurName->SetVisible(TRUE);
	pCurName->SetWindowText(sstrName);
	pCurName->Invalidate();

	SImageButton* pImgBtnMore = FindChildByName2<SImageButton>(L"btn_more");
	SASSERT(pImgBtnMore);
	pImgBtnMore->SetVisible(TRUE);
	pImgBtnMore->Invalidate();

	m_LasttalkCurSel.m_nType = nType;
	m_LasttalkCurSel.m_strID = strID;

	if (3 == m_LasttalkCurSel.m_nType ||
		4 == m_LasttalkCurSel.m_nType ||
		5 == m_LasttalkCurSel.m_nType)
	{

		SImageButton* pImgBtnMore = FindChildByName2<SImageButton>(L"btn_more");
		SASSERT(pImgBtnMore);
		pImgBtnMore->SetVisible(FALSE);
		pImgBtnMore->Invalidate();
	}
}

void CMainDlg::MessageListItemRClick(int nType, const std::string& strID)
{
	//Show R_Menu
}


void CMainDlg::OnBnClickSend()
{
	STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
	SASSERT(pChatTab);

	SStringW sstrID = S_CA2W(m_LasttalkCurSel.m_strID.c_str());
	SWindow* pPage = pChatTab->GetPage(sstrID, TRUE);
	SASSERT(pPage);
	SImRichEdit* pRecvRichedit = pPage->FindChildByName2<SImRichEdit>(L"recv_richedit");
	SImRichEdit* pSendRichedit = pPage->FindChildByName2<SImRichEdit>(L"send_richedit");
	SASSERT(pRecvRichedit);
	SASSERT(pSendRichedit);

	CHARRANGE chr = {0, -1};
	SStringT strContent = pSendRichedit->GetSelectedContent(&chr);
	pugi::xml_document doc;
	if (!doc.load_buffer(strContent, strContent.GetLength() * sizeof(WCHAR)))
		return;
	strContent.Empty();

	std::vector<SStringW> vecContent;
	pugi::xml_node node = doc.child(L"RichEditContent").first_child();
	for (; node; node = node.next_sibling())
	{
		const wchar_t* pNodeName = node.name();
		if (wcscmp(RichEditText::GetClassName(), pNodeName) == 0)			//文本
		{
			SStringW sstrContent = node.text().get();
			SStringW sstrText;
			sstrText.Format(L"<text font-size=\"10\" font-face=\"微软雅黑\" color=\"#000000\"><![CDATA[%s]]></text>", sstrContent);
			vecContent.push_back(sstrText);
		}
		else if (wcscmp(RichEditImageOle::GetClassName(), pNodeName) == 0)	//图片
		{
			SStringW sstrImgPath = node.attribute(L"path").as_string();
			SStringW sstrImg;
			sstrImg.Format(L"<img subid=\"%s\" id=\"%s\" type=\"normal_img\" encoding=\"\" show-magnifier=\"1\" path=\"%s\"/>", L"", L"", sstrImgPath);
			vecContent.push_back(sstrImg);
		}
		else if (wcscmp(RichEditMetaFileOle::GetClassName(), pNodeName) == 0)	//文件
		{
			SStringW sstrFilePath = node.attribute(L"file").as_string();
		}
		else
		{
			//其他未知消息类型
		}
	}

	AddBackGeneralMessage(pRecvRichedit, vecContent);
	pSendRichedit->Clear();
}

bool CMainDlg::OnSendRichEditAcceptData(SOUI::EventArgs* pEvt)
{
	EventQueryAccept * pev = (EventQueryAccept*)pEvt;
	if (pev->Conv->GetAvtiveFormat() == CF_HDROP)
	{
		::SetForegroundWindow(m_hWnd);
		RichFormatConv::DropFiles files = pev->Conv->GetDropFiles();
		DragDropFiles(files);
		return true;
	}
	return true;
}

bool CMainDlg::OnSendRichEditEditorChange(SOUI::EventArgs* pEvt)
{
	return true;
}

bool CMainDlg::OnSendRichEditMenu(SOUI::EventArgs* pEvt)
{
	return true;
}

bool CMainDlg::OnSendRichEditObjEvent(SOUI::EventArgs* pEvt)
{
	return true;
}

bool CMainDlg::OnRecvRichEditAcceptData(SOUI::EventArgs *pEvt)
{
	return true;
}

bool CMainDlg::OnRecvRichEditMenu(SOUI::EventArgs *pEvt)
{
	return true;
}

bool CMainDlg::OnRecvRichEditObjEvent(SOUI::EventArgs *pEvt)
{
	return true;
}

bool CMainDlg::OnRecvRichEditScrollEvent(SOUI::EventArgs *pEvt)
{
	return true;
}

void CMainDlg::DragDropFiles(RichFormatConv::DropFiles& files)
{
	SStringW sstrID = S_CA2W(m_LasttalkCurSel.m_strID.c_str());

	STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
	SASSERT(pChatTab);
	SWindow* pPage = pChatTab->GetPage(sstrID, TRUE);
	SASSERT(pPage);
	SImRichEdit* pSendRichEdit = pPage->FindChildByName2<SImRichEdit>(L"send_richedit");
	SASSERT(pSendRichEdit);

	RichFormatConv::DropFiles::iterator iter = files.begin();
	for (; iter != files.end(); ++iter)
	{
		SStringW strFile = *iter;
		int nFileSize;
		FILE* fp = _wfopen(strFile, L"rb");
		if (fp)
		{
			fseek(fp, 0L, SEEK_END);
			nFileSize = ftell(fp);
			rewind(fp);
			fclose(fp);
		}
		else
			return;

		//可对发送的文件大小做个限制
// 		if(nFileSize >= MAX_FILESIZE)
// 		{
// 			::MessageBox(this->m_hWnd, L"仅支持4M以下的文件，该文件大于4M", L"提示", MB_OK);
// 			return;
// 		}

		SStringW sstrContent;
		sstrContent.Format(L"<RichEditContent><metafile file=\"%s\" /></RichEditContent>", *iter);
		pSendRichEdit->InsertContent(sstrContent, RECONTENT_CARET);
	}

	::SetFocus(m_hWnd);
	pSendRichEdit->SetFocus();
	pSendRichEdit->GetContainer()->OnShowCaret(TRUE);
}

void CMainDlg::OnBnClickChatEmotion()
{
	//
}

void CMainDlg::OnBnClickChatImage()
{
	SStringW strFile;
	CFileDialogEx openDlg(TRUE,_T("图片"),0,6,
		_T("图片文件\0*.gif;*.bmp;*.jpg;*.png\0\0"));
	if (openDlg.DoModal() == IDOK)
	{
		strFile = openDlg.m_szFileName;

		int nFileSize;
		FILE* fp = _wfopen(strFile, L"rb");
		if (fp)
		{
			fseek(fp, 0L, SEEK_END);
			nFileSize = ftell(fp);
			rewind(fp);
			fclose(fp);
		}
		else
			return;

		//发送图片限制大小
// 		if(nFileSize >= MAX_FILESIZE)
// 		{
// 			::MessageBox(this->m_hWnd, L"仅支持4M以下的图片，该图片大于4M", L"提示", MB_OK);
// 			return;
// 		}

		SStringW str;
		str.Format(L"<RichEditContent>"
			L"<para break=\"0\" disable-layout=\"1\">"
			L"<img type=\"normal_img\" path=\"%s\" id=\"zzz\" max-size=\"\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
			L"</para>"
			L"</RichEditContent>", strFile);

		STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
		SASSERT(pChatTab);

		SStringW sstrID = S_CA2W(m_LasttalkCurSel.m_strID.c_str());

		SWindow* pPage = pChatTab->GetPage(sstrID, TRUE);
		SImRichEdit* pSendRichedit = pPage->FindChildByName2<SImRichEdit>(L"send_richedit");
		pSendRichedit->InsertContent(str, RECONTENT_CARET);
	}
}

void CMainDlg::OnBnClickChatFile()
{
	SStringW strFile;
	CFileDialogEx openDlg(TRUE,_T("文件"),0,6,
		_T("文件\0*.*\0\0"));
	if (openDlg.DoModal() == IDOK)
	{
		strFile = openDlg.m_szFileName;
		int nFileSize;
		FILE* fp = _wfopen(strFile, L"rb");
		if (fp)
		{
			fseek(fp, 0L, SEEK_END);
			nFileSize = ftell(fp);
			rewind(fp);
			fclose(fp);
		}
		else
			return;

		//发送图片限制大小
// 		if(nFileSize >= MAX_FILESIZE)
// 		{
// 			::MessageBox(this->m_hWnd, L"仅支持4M以下的文件，该文件大于4M", L"提示", MB_OK);
// 			return;
// 		}

		SStringW str;
		str.Format(L"<RichEditContent>"
			L"<metafile file=\"%s\" />"
			L"</RichEditContent>", strFile);

		STabCtrl* pChatTab = FindChildByName2<STabCtrl>(L"chattab");
		SASSERT(pChatTab);

		SStringW sstrID = S_CA2W(m_LasttalkCurSel.m_strID.c_str());

		SWindow* pPage = pChatTab->GetPage(sstrID, TRUE);
		SImRichEdit* pSendRichedit = pPage->FindChildByName2<SImRichEdit>(L"send_richedit");
		pSendRichedit->InsertContent(str, RECONTENT_CARET);
	}
}

void CMainDlg::OnBnClickChatCapture()
{
	//
}

void CMainDlg::OnBnClickSettingCapture()
{
	//点击截图旁的箭头
	MenuWrapper menu(L"menu_snapshot_arrow", L"SMENU");
	menu.AddMenu(L"屏幕截图", 2001, TRUE, FALSE);
	menu.AddMenu(L"截图隐藏当前窗口", 2002, TRUE, FALSE);

	MenuItemWrapper* pItemCapture = menu.GetMenuItemById(2001);
	MenuItemWrapper* pItemHideWindow = menu.GetMenuItemById(2002);

	if (m_bSnapshotHideWindow)
		pItemHideWindow->SetCheck(TRUE);

	int ret = 0;
	POINT pt;
	::GetCursorPos(&pt);
	ret = menu.ShowMenu(TPM_RETURNCMD, pt.x, pt.y, m_hWnd);
	switch (ret)
	{
	case 2001:
			OnBnClickChatCapture();
		break;

	case 2002:
		{
			if (m_bSnapshotHideWindow)
				m_bSnapshotHideWindow = false;
			else
				m_bSnapshotHideWindow = true;
		}
		break;

	default:
		break;
	}
}

void CMainDlg::OnBnClickChatHistory()
{
	//
}

void CMainDlg::AddBackGeneralMessage(SImRichEdit* pRecvRichEdit, const std::vector<SStringW>& vecContents)
{
	//统一使用右侧布局
	LPCWSTR pEmpty;
	pEmpty = L"<para id=\"msgbody\" margin=\"0,0,0,0\" break=\"1\" simulate-align=\"1\">"		
		L""
		L"</para>";

	SStringW sstrResend;
	sstrResend = L"<bkele id=\"resend\" name=\"BkEleSendFail\" data=\"resend\" right-skin=\"skin.richedit_resend\" right-pos=\"{-15,[-16,@12,@12\" cursor=\"hand\" interactive=\"1\"/>";

	SStringW sstrMsg;
	for (int i = 0; i < vecContents.size(); i++)
	{
		sstrMsg += vecContents[i];
	}

	SStringW sstrContent;
	sstrContent.Format(
		L"<RichEditContent  type=\"ContentRight\" align=\"right\" auto-layout=\"1\">"
		L"<para break=\"1\" align=\"left\" />"
		L"<bkele data=\"avatar\" id=\"%s\" skin=\"%s\" left-pos=\"0,]-6,@40,@40\" right-pos=\"-50,]-9,@40,@40\" cursor=\"hand\" interactive=\"1\"/>"
		L"<para id=\"msgbody\" margin=\"65,0,35,0\" break=\"1\" simulate-align=\"1\">"		
		L"%s"
		L"</para>"
		L"<bkele data=\"bubble\" left-skin=\"skin_left_bubble\" right-skin=\"skin_right_otherbubble\" left-pos=\"50,{-9,[10,[10\" right-pos=\"{-10,{-9,-55,[10\" />"
		L"%s"
		L"</RichEditContent>",
		L"default_portrait", L"default_portrait", sstrMsg, pEmpty);

	pRecvRichEdit->InsertContent(sstrContent, RECONTENT_LAST);
	pRecvRichEdit->ScrollToBottom();

	RichEditBkElement* pResendEleObj = sobj_cast<RichEditBkElement>(pRecvRichEdit->GetElementById(L"resend"));
	if (pResendEleObj)
	{
		pResendEleObj->SetVisible(FALSE);
		pRecvRichEdit->Invalidate();
	}
}

void CMainDlg::AddBackFileMessage(SImRichEdit* pRecvRichEdit, const SStringW& sstrFilePath)
{
	//
}

void CMainDlg::AddBackSysMessage(SImRichEdit* pRecvRichEdit, const SStringW& sstrContent)
{
	//
}