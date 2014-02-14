#ifndef __MAINFRM_H__
#define __MAINFRM_H__

#include "resource.h"
#include "docview.h"
#include "psdl.h"
#include "psdl_doc.h"
#include "psdl_view.h"
#include "cpvs_doc.h"
#include "toolwnd.h"
#include "glview.h"
#include "histmgr.h"

#include "include/atldock.h"
#include "include/atldock2.h"

class CMainFrame :
	public CFrameWindowImpl<CMainFrame>,
	public CUpdateUI<CMainFrame>,
	public CMessageFilter,
	public CIdleHandler,
	public HistoryManager
{
public:

	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	virtual BOOL PreTranslateMessage(MSG *pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_FILE_SAVE,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_UNDO,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_REDO,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR,			UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_WINDOWS_CITYBLOCKS,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_WINDOWS_ATTRIBUTES,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_WINDOWS_PROPERTIES,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR,		UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_MODE_PSDL,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MODE_CPVS,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MODE_INST,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MODE_BAI,				UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MODE_PATHSET,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_INSERT_DUPLICATE_BLOCKS,	UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_TOOLS_OPTIMIZE,	UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE,		OnCreate)
		MESSAGE_HANDLER(WM_DESTROY,		OnDestroy)
		MESSAGE_HANDLER(WM_PAINT_DESCENDANTS, OnPaintDescendants);

		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_RANGE_HANDLER(ID_FILE_NEW_PSDL, ID_FILE_NEW_WORKSPACE, OnFileNew)
		COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
		COMMAND_ID_HANDLER(ID_FILE_OPENCONTAININGFOLDER, OnOpenContainingFolder)
		COMMAND_ID_HANDLER(ID_FILE_SAVE, OnFileSave)
		COMMAND_ID_HANDLER(ID_FILE_SAVE_AS, OnFileSaveAs)
		COMMAND_ID_HANDLER(ID_APP_EXIT,	OnFileExit)

		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_RANGE_HANDLER(ID_WINDOWS_CITYBLOCKS, ID_WINDOWS_BAI_CULLING, OnViewBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_RANGE_HANDLER(ID_MODE_PSDL, ID_MODE_PATHSET, OnSetEditingMode)

		COMMAND_ID_HANDLER(ID_INSERT_CITYBLOCK, OnInsertBlock)
		COMMAND_ID_HANDLER(ID_INSERT_DUPLICATE_BLOCKS, OnDuplicateBlock)

		COMMAND_ID_HANDLER(ID_TOOLS_OPTIMIZE, OnOptimizePSDL)
		COMMAND_ID_HANDLER(ID_TOOLS_MM2, OnLaunchMM2)
		COMMAND_ID_HANDLER(ID_TOOLS_OPTIONS, OnOptions)

		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)

		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	LRESULT OnDestroy			(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnCreate			(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnPaintDescendants	(UINT, WPARAM, LPARAM, BOOL&);

	LRESULT OnFileExit				(WORD, WORD, HWND, BOOL&);
	LRESULT OnFileNew				(WORD, WORD, HWND, BOOL&);
	LRESULT OnFileOpen				(WORD, WORD, HWND, BOOL&);
	LRESULT OnOpenContainingFolder	(WORD, WORD, HWND, BOOL&);
	LRESULT OnFileSave				(WORD, WORD, HWND, BOOL&);
	LRESULT OnFileSaveAs			(WORD, WORD, HWND, BOOL&);

	LRESULT OnViewToolBar			(WORD, WORD, HWND, BOOL&);
	LRESULT OnViewStatusBar			(WORD, WORD, HWND, BOOL&);
	LRESULT OnViewBar				(WORD, WORD, HWND, BOOL&);
	LRESULT OnSetEditingMode		(WORD, WORD, HWND, BOOL&);

	LRESULT OnInsertBlock			(WORD, WORD, HWND, BOOL&);
	LRESULT OnDuplicateBlock		(WORD, WORD, HWND, BOOL&);

	LRESULT OnOptimizePSDL			(WORD, WORD, HWND, BOOL&);
	LRESULT OnLaunchMM2				(WORD, WORD, HWND, BOOL&);
	LRESULT OnOptions				(WORD, WORD, HWND, BOOL&);

	LRESULT OnAppAbout				(WORD, WORD, HWND, BOOL&);

	void UpdateCaption(void);
	void SetEditingMode(int iMode);
	void SelectBlock(long iIndex);
	void SelectAttribute(psdl::block* pBlock, long iIndex);

	DocTemplateBase* GetActiveDocument(void);
	BOOL CanSave(void);

	// HistoryManager
	CString GetUndoDescription(void);
	CString GetRedoDescription(void);

private:

	int m_iEditMode;

	PSDLDocTemplate m_psdlDoc;
	CPVSDocTemplate m_cpvsDoc;

	COpenGLView m_view;
	CFlatDockingWindow m_dock;

	CBlocksWindow		m_wndBlocks;
	CPerimeterWindow	m_wndPerimeter;
	CAttributesWindow	m_wndAttribs;
	CPropertiesWindow	m_wndProps;
};

#endif
