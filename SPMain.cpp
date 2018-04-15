// SmartProj.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SmartProj.h"

#define MAX_LOADSTRING 100

#define CM_ADD_ITEM 1
#define CM_RENAME	2
#define CM_REMOVE	3

// Global Variables:
HINSTANCE			g_hInst;			// current instance
HWND				g_hWndMenuBar;		// menu bar handle
HWND				hwndTV;				// tree view handle
HWND				mainWin;			// main window handle
TCHAR				addedItem[64];		// name of item to add			
HTREEITEM			currentItem = NULL;	// selected item
CustomTree			*CTRoot;			// root tree item

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
HWND TreeViewCreate(HWND);
void TreeViewAddItem(HWND, LPWSTR, BOOL);
void ProcessContextMenu(UINT, CustomTree *);

int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPTSTR    lpCmdLine,
				   int       nCmdShow)
{
	MSG msg;

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SMARTPROJ));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMARTPROJ));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	TCHAR szTitle[MAX_LOADSTRING];		// title bar text
	TCHAR szWindowClass[MAX_LOADSTRING];	// main window class name

	g_hInst = hInstance; // Store instance handle in our global variable

	// SHInitExtraControls should be called once during your application's initialization to initialize any
	// of the device specific controls such as CAPEDIT and SIPPREF.
	SHInitExtraControls();

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
	LoadString(hInstance, IDC_SMARTPROJ, szWindowClass, MAX_LOADSTRING);

	//If it is already running, then focus on the window, and exit
	hWnd = FindWindow(szWindowClass, szTitle);	
	if (hWnd) 
	{
		// set focus to foremost child window
		// The "| 0x00000001" is used to bring any owned windows to the foreground and
		// activate them.
		SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));
		return 0;
	}

	if (!MyRegisterClass(hInstance, szWindowClass))
	{
		return FALSE;
	}

	hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	mainWin = hWnd;

	// When the main window is created using CW_USEDEFAULT the height of the menubar (if one
	// is created is not taken into account). So we resize the window after creating it
	// if a menubar is present
	if (g_hWndMenuBar)
	{
		RECT rc;
		RECT rcMenuBar;

		GetWindowRect(hWnd, &rc);
		GetWindowRect(g_hWndMenuBar, &rcMenuBar);
		rc.bottom -= (rcMenuBar.bottom - rcMenuBar.top);

		MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
	}

	// Work on a tree view
	hwndTV = TreeViewCreate(hWnd);
	CTRoot = new CustomTree(L"Project Root", true);
	CTRoot->renderTreeView(hwndTV, NULL);
	TreeView_Select(hwndTV, CTRoot->getHandle(), TVGN_CARET);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);


	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	static SHACTIVATEINFO s_sai;

	OPENFILENAME ofn;
	TCHAR name[256];

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = mainWin;
	ofn.lpstrFile = name;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(name)/sizeof(TCHAR);
	ofn.lpstrFilter = L"JSON file\0*.JSON\0Text file\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;

	switch (message) 
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_HELP_ABOUT:
			DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
			break;
		case IDM_ADD_ITEM:
			DialogBox(g_hInst, (LPCTSTR)IDD_ADDBOX, hWnd, Prompt);
			TreeViewAddItem(hwndTV, (LPWSTR) &addedItem, false);
			break;
		case IDM_ADD_CATEGORY:
			DialogBox(g_hInst, (LPCTSTR)IDD_ADDBOX, hWnd, Prompt);
			TreeViewAddItem(hwndTV, (LPWSTR) &addedItem, true);
			break;
		case ID_OPTIONS_SAVEAS:
			if (GetSaveFileName(&ofn) == TRUE)
			{
				CTRoot->saveToFile(ofn.lpstrFile);
			}
			break;
		case ID_OPTIONS_OPEN:
			if (GetOpenFileName(&ofn) == TRUE)
			{
				CustomTree *newCTRoot = new CustomTree();
				if (newCTRoot->loadFromFile(ofn.lpstrFile))
				{
					newCTRoot->renderTreeView(hwndTV, NULL);
					delete CTRoot;
					CTRoot = newCTRoot;
				}
				else
				{
					MessageBox(mainWin, L"Failed to load", L"Error", MB_OK | MB_ICONERROR);
					delete newCTRoot;
				}
			}
			break;
		case IDM_OK:
			delete CTRoot;
			SendMessage (hWnd, WM_CLOSE, 0, 0);				
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_CREATE:
		SHMENUBARINFO mbi;

		memset(&mbi, 0, sizeof(SHMENUBARINFO));
		mbi.cbSize     = sizeof(SHMENUBARINFO);
		mbi.hwndParent = hWnd;
		mbi.nToolBarId = IDR_MENU;
		mbi.hInstRes   = g_hInst;

		if (!SHCreateMenuBar(&mbi)) 
		{
			g_hWndMenuBar = NULL;
		}
		else
		{
			g_hWndMenuBar = mbi.hwndMB;
		}

		// Initialize the shell activate info structure
		memset(&s_sai, 0, sizeof (s_sai));
		s_sai.cbSize = sizeof (s_sai);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		CommandBar_Destroy(g_hWndMenuBar);
		PostQuitMessage(0);
		break;

	case WM_ACTIVATE:
		// Notify shell of our activate message
		SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);
		break;
	case WM_SETTINGCHANGE:
		SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
		break;

	case WM_NOTIFY:
	{
		LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lParam);
		if (pNMHDR->hwndFrom == hwndTV)
		{
			if (pNMHDR->code == TVN_SELCHANGED)
			{
				currentItem = TreeView_GetSelection(hwndTV);
				CustomTree *ct = CTRoot->findNodeByHandle(currentItem);
				if (ct)
				{
					ct->toggleCheckBox();
					CTRoot->updateTreeView(hwndTV);
				}
			}
			else if (pNMHDR->code == TVN_ITEMEXPANDED)
			{
				LPNMTREEVIEW tv;
				tv = (LPNMTREEVIEW) lParam;
				CTRoot->findNodeByHandle(currentItem)->setExpanded(tv->action == TVE_EXPAND);
				CTRoot->updateTreeView(hwndTV);
			}
		}
		break;
	}

	case WM_CONTEXTMENU:
	{
		HTREEITEM hItem = TreeView_GetNextItem(hwndTV, 0, TVGN_CARET);
		CustomTree *ct = CTRoot->findNodeByHandle(hItem);

		if (ct)
		{
			HMENU hPopupMenu = CreatePopupMenu();
			InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, CM_REMOVE, L"Remove");
			InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, CM_RENAME, L"Rename");
			InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, CM_ADD_ITEM, L"Add item");
			SetForegroundWindow(mainWin);
			UINT sel = TrackPopupMenu(hPopupMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RETURNCMD,
									  GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0, mainWin, NULL);

			ProcessContextMenu(sel, ct);
		}
	}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

HWND TreeViewCreate(HWND hwndParent)
{ 
	RECT rcClient;  // dimensions of client area 
	HWND hwndTV;    // handle to tree-view control 

	// Ensure that the common control DLL is loaded. 
	InitCommonControls(); 

	// Get the dimensions of the parent window's client area, and create 
	// the tree-view control. 
	GetClientRect(hwndParent, &rcClient); 
	hwndTV = CreateWindowEx(0,
		WC_TREEVIEW,
		TEXT("Projects"),
		WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES, 
		0, 
		0, 
		rcClient.right, 
		rcClient.bottom,
		hwndParent, 
		(HMENU)ID_TREEVIEW, 
		g_hInst, 
		NULL); 

	HIMAGELIST imageList = ImageList_Create(16, 16, 0, 4, 4);
	DRA::ImageList_AddIcon(imageList, LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_FOLDER)));
	DRA::ImageList_AddIcon(imageList, LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_FOLDERC)));
	DRA::ImageList_AddIcon(imageList, LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_RECORD)));
	DRA::ImageList_AddIcon(imageList, LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_RECORDC)));
	TreeView_SetImageList(hwndTV, imageList, TVSIL_NORMAL);

	return hwndTV;
}

void TreeViewAddItem(HWND hwndTV, LPWSTR s, BOOL cat)
{
	CustomTree *node = CTRoot->findNodeByHandle(currentItem);
	CustomTree *newNode = new CustomTree(s, cat);
	if (node)
	{
		node->addChild(newNode);
	}
	else
	{
		CTRoot->addChild(newNode);
	}
	CTRoot->renderTreeView(hwndTV, NULL);
}

void ProcessContextMenu(UINT option, CustomTree *item)
{
	switch(option)
	{
	case CM_ADD_ITEM:
		DialogBox(g_hInst, (LPCTSTR)IDD_ADDBOX, mainWin, Prompt);
		TreeViewAddItem(hwndTV, (LPWSTR) &addedItem, false);
		break;
	case CM_RENAME:
		DialogBox(g_hInst, (LPCTSTR)IDD_ADDBOX, mainWin, Prompt);
		item->setCaption(addedItem);
		CTRoot->updateTreeView(hwndTV);
		break;
	case CM_REMOVE:
		break;
	}
}