#ifndef __OPTIONSDLG_H__
#define __OPTIONSDLG_H__

#include "resource.h"
#include "config.h"
#include "mainfrm.h"
#include <sstream>
#include <iterator>

using namespace std;

class COptionsDialog;

class COptionsPage
{
public:
	virtual HWND Create(HWND hWndParent) = 0;
	virtual CString GetTitle(void) = 0;
	virtual BOOL IsCreated(void) = 0;
	virtual void Show(void) = 0;
	virtual void Hide(void) = 0;
	virtual void OnOk(void) = 0;
};

template <class T>
class COptionsPageImpl : public CDialogImpl<T>, public COptionsPage
{
public:
	virtual ~COptionsPageImpl() {}

	HWND Create(HWND hWndParent)
	{
		return CDialogImpl<T>::Create(hWndParent);
	}
	CString GetTitle(void)
	{
		CString strTitle;
		strTitle.LoadString(T::IDD);
		return strTitle;
	}
	BOOL IsCreated(void) { return IsWindow(); }
	void Show(void) { ShowWindow(SW_SHOW); }
	void Hide(void) { ShowWindow(SW_HIDE); }
	virtual void OnOk(void) {}
};

class COptionsPageGeneral :
	public COptionsPageImpl<COptionsPageGeneral>,
	public CWinDataExchange<COptionsPageGeneral>
{
public:
	enum { IDD = IDD_OPTIONS_GENERAL };

	BEGIN_MSG_MAP(COptionsPageGeneral)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DDX_MAP(COptionsPageGeneral)
		DDX_CHECK(IDC_VERTICES, config.general.test)
	END_DDX_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		DoDataExchange(FALSE);
		return 1;
	}

	void OnOk(void)
	{
		DoDataExchange(TRUE);
	}
};

class COptionsPageRendering :
	public COptionsPageImpl<COptionsPageRendering>,
	public CWinDataExchange<COptionsPageRendering>
{
public:
	enum { IDD = IDD_OPTIONS_RENDER };

	BEGIN_MSG_MAP(COptionsPageRendering)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DDX_MAP(COptionsPageRendering)
		DDX_CHECK(IDC_USE_TEXTURES, config.display.bTextures)
		DDX_CHECK(IDC_TEXTURE_FILTER, config.display.bTextureNearest)
	END_DDX_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		DoDataExchange(FALSE);
		return 1;
	}

	void OnOk(void)
	{
		DoDataExchange(TRUE);
		CMainFrame::GetView()->Invalidate();
	}
};

class COptionsPageTools :
	public COptionsPageImpl<COptionsPageTools>,
	public CWinDataExchange<COptionsPageTools>
{
public:
	enum { IDD = IDD_OPTIONS_TOOLS };

	BEGIN_MSG_MAP(COptionsPageTools)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_BROWSE1, OnBrowse)
		COMMAND_ID_HANDLER(IDC_BROWSE2, OnBrowse)
	END_MSG_MAP()

	BEGIN_DDX_MAP(COptionsPageTools)
		DDX_TEXT(IDC_EDIT1, m_strMM2Path)
		DDX_TEXT(IDC_EDIT2, m_strSDLViewPath)
	END_DDX_MAP()

	COptionsPageTools()
	{
		ExpandEnvironmentStrings(config.tools.strMM2Exe.c_str(), m_strMM2Path.GetBuffer(MAX_PATH), MAX_PATH);
		ExpandEnvironmentStrings(config.tools.strSDLViewExe.c_str(), m_strSDLViewPath.GetBuffer(MAX_PATH), MAX_PATH);
	}

	~COptionsPageTools()
	{
		config.tools.strMM2Exe = m_strMM2Path;
		config.tools.strSDLViewExe = m_strSDLViewPath;
	}

	LRESULT OnBrowse(WORD, WORD wID, HWND, BOOL&)
	{
		WORD wIDEdit = wID == IDC_BROWSE1 ? IDC_EDIT1 : IDC_EDIT2;

		DoDataExchange(TRUE, wIDEdit);

		CString strCurrent = wID == IDC_BROWSE1 ? m_strMM2Path : m_strSDLViewPath;

		CFileDialog fDlg(TRUE, NULL, strCurrent, OFN_HIDEREADONLY, _T("Executables\0*.exe\0All Files\0*.*\0"));

		if (IDOK == fDlg.DoModal())
		{
			if (wID == IDC_BROWSE1)
				m_strMM2Path = fDlg.m_szFileName;
			else
				m_strSDLViewPath = fDlg.m_szFileName;

			DoDataExchange(FALSE, wIDEdit);
		}
		return 0;
	}

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		DoDataExchange(FALSE);
		return 1;
	}

	void OnOk(void)
	{
		DoDataExchange(TRUE);
	}

	CString m_strMM2Path;
	CString m_strSDLViewPath;
};

class COptionsPageDirectories :
	public COptionsPageImpl<COptionsPageDirectories>,
	public CWinDataExchange<COptionsPageDirectories>
{
public:
	enum { IDD = IDD_OPTIONS_DIRECTORIES };

	BEGIN_MSG_MAP(COptionsPageDirectories)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DDX_MAP(COptionsPageDirectories)
		DDX_TEXT(IDC_EDIT1, m_strEdit1)
		DDX_TEXT(IDC_EDIT2, m_strEdit2)
	END_DDX_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
/*		const WORD upArrow[] = { 0xef, 0xc7, 0x83, 0x01 };
		const WORD dnArrow[] = { 0x01, 0x83, 0xc7, 0xef };

		HBITMAP hBitmap;

		hBitmap = CreateBitmap(7, 4, 1, 1, upArrow);
		SendDlgItemMessage(IDC_UP, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hBitmap);

		hBitmap = CreateBitmap(7, 4, 1, 1, dnArrow);
		SendDlgItemMessage(IDC_DOWN, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hBitmap);*/

		ostringstream res;
		copy(config.directories.texturePaths.begin(), config.directories.texturePaths.end(), ostream_iterator<string>(res, "\r\n"));

		m_strEdit1 = (CString) res.str().c_str();

		DoDataExchange(FALSE);
		return 1;
	}

	void OnOk(void)
	{
		DoDataExchange(TRUE);

		stringstream ss;
		ss << (LPCTSTR) m_strEdit1;
		string item;
		vector<string> lines;
		while (getline(ss, item))
		{
			lines.push_back(item);
		}
		config.directories.texturePaths.assign(lines.begin(), lines.end());
	}

private:
	CString m_strEdit1, m_strEdit2;
};

struct PageEntry
{
	HTREEITEM     hItem;
	COptionsPage* pPage;
	PageEntry*    pParent;

	PageEntry(COptionsPage* pPage, PageEntry* pParent = NULL) :
	hItem(NULL), pPage(pPage), pParent(pParent) {}
};

class COptionsDialog : public CDialogImpl<COptionsDialog>
{
public:

	COptionsDialog() : m_pCurrentPage(NULL) {}

	enum { IDD = IDD_OPTIONS };

	BEGIN_MSG_MAP(COptionsDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDAPPLY, OnApply)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnItemSelected)
	END_MSG_MAP()

	LRESULT OnItemSelected(int, LPNMHDR pnmh, BOOL&)
	{
		if (m_pCurrentPage && m_pCurrentPage->IsCreated())
			m_pCurrentPage->Hide();

		m_pCurrentPage = (COptionsPage*) m_tree.GetItemData(LPNMTREEVIEW(pnmh)->itemNew.hItem);

		if (m_pCurrentPage)
		{
			if (!m_pCurrentPage->IsCreated())
				m_pCurrentPage->Create(m_hWndHolder);
			m_pCurrentPage->Show();
		}

		return 0;
	}

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		CenterWindow(GetParent());

		m_tree       = GetDlgItem(IDC_TREE);
		m_hWndHolder = GetDlgItem(IDC_PLACEHOLDER);

		HTREEITEM hItem;

		for (int i = 0; i < m_arrPages.GetSize(); i++)
		{
			HTREEITEM hParent = TVI_ROOT;

			if (m_arrPages[i]->pParent && m_arrPages[i]->pParent->hItem)
				hParent = m_arrPages[i]->pParent->hItem;

			m_arrPages[i]->hItem = m_tree.InsertItem(m_arrPages[i]->pPage->GetTitle(), hParent, TVI_LAST);
			m_tree.SetItemData(m_arrPages[i]->hItem, (DWORD_PTR) m_arrPages[i]->pPage);
		}

		HTREEITEM hRoot = m_tree.GetRootItem();
		while (hRoot)
		{
			m_tree.Expand(hRoot);
			if (m_tree.GetItemData(hRoot) == (LPARAM) m_pCurrentPage)
				m_tree.SelectItem(hRoot);

			hItem = m_tree.GetNextItem(hRoot, TVGN_CHILD);
			while (hItem)
			{
				if (m_tree.GetItemData(hItem) == (LPARAM) m_pCurrentPage)
					m_tree.SelectItem(hItem);

				hItem = m_tree.GetNextItem(hRoot, TVGN_NEXT);
			}

			hRoot = m_tree.GetNextItem(hRoot, TVGN_NEXT);
		};

		return 1;
	}

	PageEntry* AddPage(COptionsPage* pPage, PageEntry* pParent = NULL)
	{
		ATLASSERT(pPage != NULL);

		PageEntry* pEntry = new PageEntry(pPage, pParent);
		m_arrPages.Add(pEntry);

		return pEntry;
	}

	void SetInitialPage(COptionsPage* pPage)
	{
		m_pCurrentPage = pPage;
	}

	LRESULT OnApply(WORD, WORD, HWND, BOOL&)
	{
		for (int i = 0; i < m_arrPages.GetSize(); i++)
		{
			if (m_arrPages[i]->pPage->IsCreated())
				m_arrPages[i]->pPage->OnOk();
		}

		return 0;
	}

	LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		if (wID == IDOK)
		{
			for (int i = 0; i < m_arrPages.GetSize(); i++)
			{
				if (m_arrPages[i]->pPage->IsCreated())
					m_arrPages[i]->pPage->OnOk();
			}
		}

		EndDialog(wID);
		return 0;
	}

private:

	HWND m_hWndHolder;
	CTreeViewCtrl m_tree;
	CSimpleArray<PageEntry*> m_arrPages;
	COptionsPage* m_pCurrentPage;
};

#endif
