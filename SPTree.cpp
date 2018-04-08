#include "StdAfx.h"

CustomTree::CustomTree(void)
{
	isCategory = false;
	isExpanded = true;
	percentFilled = 0;
	parent = NULL;
	handle = NULL;
	next = NULL;
	firstChild = NULL;
	wcscpy_s(this->caption, sizeof(this->caption), L"Dummy");
}

CustomTree::CustomTree(LPWSTR caption, BOOL isCategory)
{
	this->isCategory = isCategory;
	this->isExpanded = true;
	this->percentFilled = 0;
	this->parent = NULL;
	this->handle = NULL;
	this->next = NULL;
	this->firstChild = NULL;
	wcscpy_s(this->caption, sizeof(this->caption), caption);
}

CustomTree::~CustomTree(void)
{
	CustomTree *child, *prev_child = NULL;
	for (child = this->getFirstChild(); child; prev_child = child, child = child->getNext())
	{
		if (prev_child)
		{
			delete prev_child;
		}
	}
}

void CustomTree::addChild(CustomTree *node)
{
	CustomTree *ctp;

	if (!this->checkCategory())
	{
		return;
	}

	ctp = this->getFirstChild();
	if (!ctp)
	{
		this->firstChild = node;
	}
	else
	{
		while (ctp->getNext())
		{
			ctp = ctp->getNext();
		}
		ctp->next = node;
	}
	node->setParent(this);
}

void CustomTree::setParent(CustomTree *node)
{
	this->parent = node;
}

void CustomTree::setExpanded(BOOL state)
{
	this->isExpanded = state;
}

CustomTree* CustomTree::getNext()
{
	return this->next;
}

CustomTree* CustomTree::getFirstChild()
{
	return this->firstChild;
}

HTREEITEM CustomTree::getHandle()
{
	return this->handle;
}

CustomTree* CustomTree::findNodeByHandle (HTREEITEM handle)
{
	CustomTree *node = this;

	while (node)
	{
		if (node->getHandle() == handle)
		{
			return node;
		}
		else
		{
			CustomTree *node2 = node->getFirstChild();
			if (node2)
			{
				node2 = node2->findNodeByHandle (handle);
				if (node2)
				{
					return node2;
				}
			}
		}
		node = node->getNext();
	}

	return NULL;
}

UINT CustomTree::getPercent()
{
	CustomTree *node = NULL;
	UINT count = 0, total = 0;

	if (this->checkCategory())
	{
		for (node = this->getFirstChild(); node; node = node->getNext())
		{
			count++;
			total += node->getPercent();
		}

		if (count)
		{
			this->percentFilled = total / count;
		}
	}

	return this->percentFilled;
}

void CustomTree::setPercent(UINT percent)
{
	if (!this->getFirstChild())
	{
		this->percentFilled = percent;
	}
}

void CustomTree::render(HWND hWndTv, HTREEITEM parentItem)
{
	TCHAR tmp[64];
	TVINSERTSTRUCT tvis = {0};
	CustomTree *node;
	CustomTree *sel = NULL;

	// for root of the tree perform calculation and saving of the current selected node
	if (parentItem == NULL)
	{
		sel = this->findNodeByHandle(TreeView_GetSelection(hWndTv));

		// and only then purge the view
		TreeView_DeleteAllItems(hWndTv);
	}

	if (this->checkCategory() && this->getFirstChild())
	{
		wsprintf(tmp, L"%s (%d%%)", this->caption, this->getPercent());
	}
	else
	{
		wsprintf(tmp, L"%s", this->caption);
	}

	tvis.item.mask = TVIF_TEXT |  TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_PARAM;
	tvis.item.stateMask = TVIS_EXPANDED;
	if (this->checkCategory())
	{
		if (this->isExpanded)
		{
			tvis.item.state = TVIS_EXPANDED;
			tvis.item.iImage = 0; // folder picture
			tvis.item.iSelectedImage = 0;
		}
		else
		{
			tvis.item.state = 0;
			tvis.item.iImage = 1; // folder picture
			tvis.item.iSelectedImage = 1;
		}
	}
	else
	{
		if (this->getPercent() == 100)
		{
			tvis.item.iImage = 3; // check box marked
			tvis.item.iSelectedImage = 3;
		}
		else
		{
			tvis.item.iImage = 2; // empty check box
			tvis.item.iSelectedImage = 2;
		}
	}
	tvis.item.pszText = (LPWSTR) tmp;
	tvis.hInsertAfter = TVI_LAST;
	tvis.hParent = parentItem ? parentItem : TVI_ROOT;

	this->handle = TreeView_InsertItem(hWndTv, (LPTVINSERTSTRUCT)&tvis);

	for (node = this->getFirstChild(); node; node = node->getNext())
	{
		node->render(hWndTv, this->handle);
	}

	if (sel)
	{
		TreeView_Select(hWndTv, sel->getHandle(), TVGN_CARET);
	}
}

BOOL CustomTree::saveToFile(LPCWSTR fileName)
{
	HANDLE hFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	// TODO: serialize to file

	CloseHandle(hFile);
	return true;
}

BOOL CustomTree::loadFromFile(LPCWSTR fileName)
{
	HANDLE hFile = CreateFile(fileName, GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	DWORD fileSize = GetFileSize(hFile, NULL);
	TCHAR *buffer = (TCHAR*) malloc(fileSize);

	if (!buffer)
	{
		CloseHandle(hFile);
		return false;
	}

	if (ReadFile(hFile, (LPVOID) buffer, fileSize, NULL, NULL) == false)
	{
		CloseHandle(hFile);
		return false;
	}
	CloseHandle(hFile);

	// TODO: deserialize buffer

	free (buffer);
	return true;
}

BOOL CustomTree::checkCategory()
{
	return this->isCategory;
}

LPCWSTR CustomTree::getCaptionP()
{
	return (LPCWSTR) &this->caption;
}