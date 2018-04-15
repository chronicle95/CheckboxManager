#pragma once
#include <windows.h>

class CustomTree
{
public:
	CustomTree(void);
	CustomTree(LPCWSTR caption, BOOL isCategory);
	~CustomTree(void);
	CustomTree *getFirstChild();
	CustomTree *getNext();
	HTREEITEM getHandle();
	void setParent(CustomTree *node);
	void addChild(CustomTree *node);
	void updateTreeView(HWND hWndTv);
	void renderTreeView(HWND hWndTv, HTREEITEM parentItem);
	void renderJSON(HANDLE hFile);
	UINT parseJSON(LPCWSTR buffer, UINT maxSz);
	void setExpanded(BOOL state);
	void setPercent(UINT percent);
	UINT getPercent();
	void setCaption(LPCWSTR caption);
	LPCWSTR getCaptionP();
	BOOL checkCategory();
	BOOL saveToFile(LPCWSTR fileName);
	BOOL loadFromFile(LPCWSTR fileName);
	CustomTree *findNodeByHandle (HTREEITEM handle);
	void toggleCheckBox();

private:
	BOOL isCategory;
	HTREEITEM handle;
	CustomTree *firstChild;
	CustomTree *next;
	CustomTree *parent;
	TCHAR caption[64];
	BOOL isExpanded;
	UINT percentFilled; // this is set to 100% for TRUE and to 0% for FALSE

	void prepareItem(TVITEM *item);
};
