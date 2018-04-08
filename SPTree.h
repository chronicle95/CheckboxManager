#pragma once
#include <windows.h>

class CustomTree
{
public:
	CustomTree(void);
	CustomTree(LPWSTR caption, BOOL isCategory);
	~CustomTree(void);
	CustomTree *getFirstChild();
	CustomTree *getNext();
	HTREEITEM getHandle();
	void setParent(CustomTree *node);
	void addChild(CustomTree *node);
	void render(HWND hWndTv, HTREEITEM parentItem);
	void setExpanded(BOOL state);
	void setPercent(UINT percent);
	UINT getPercent();
	LPCWSTR getCaptionP();
	BOOL checkCategory();
	BOOL saveToFile(LPCWSTR fileName);
	BOOL loadFromFile(LPCWSTR fileName);
	CustomTree *findNodeByHandle (HTREEITEM handle);

private:
	BOOL isCategory;
	HTREEITEM handle;
	CustomTree *firstChild;
	CustomTree *next;
	CustomTree *parent;
	TCHAR caption[64];
	BOOL isExpanded;
	UINT percentFilled; // this is set to 100% for TRUE and to 0% for FALSE
};
