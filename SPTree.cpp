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

CustomTree::CustomTree(LPCWSTR caption, BOOL isCategory)
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

// Calculates percent value for category
// and returns it, or just returns the value for plain records
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

			// do not count empty categories
			if (node->checkCategory() && !node->getFirstChild())
			{
				count--;
			}
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

// Draws TREEVIEW item hierarchy from scratch
void CustomTree::renderTreeView(HWND hWndTv, HTREEITEM parentItem)
{
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

	// Set proper images and captions
	this->prepareItem(&(tvis.item));

	tvis.hInsertAfter = TVI_LAST;
	tvis.hParent = parentItem ? parentItem : TVI_ROOT;

	this->handle = TreeView_InsertItem(hWndTv, (LPTVINSERTSTRUCT)&tvis);

	for (node = this->getFirstChild(); node; node = node->getNext())
	{
		node->renderTreeView(hWndTv, this->handle);
	}

	if (sel)
	{
		TreeView_Select(hWndTv, sel->getHandle(), TVGN_CARET);
	}
}

// Writes a string to the end of an ALREADY OPENED file
static void FileAppendString(HANDLE hFile, LPCWSTR text)
{
	UINT sz = wcslen(text) * 4; // for sure
	char *buf = (char *) malloc(sz);
	memset(buf, 0, sz);
	DWORD bytesWritten;
	SetFilePointer(hFile, 0, NULL, FILE_END);
	WideCharToMultiByte(CP_UTF8, 0, text, wcslen(text), (LPSTR)buf, sz, NULL, NULL);
	WriteFile(hFile, (LPCVOID)buf, strlen(buf), (LPDWORD)&bytesWritten, NULL);
	free(buf);
}

BOOL CustomTree::saveToFile(LPCWSTR fileName)
{
	HANDLE hFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	this->renderJSON(hFile);

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

	this->parseJSON(buffer);

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

// Refreshes TREEVIEW according to the model
void CustomTree::updateTreeView(HWND hWndTv)
{
	TVITEM item;
	CustomTree *node;

	memset(&item, 0, sizeof(item));

	item.hItem = this->getHandle();
	item.mask = TVIF_TEXT;
	TreeView_GetItem(hWndTv, &item);
	this->prepareItem(&item);
	TreeView_SetItem(hWndTv, &item);

	for (node = this->getFirstChild(); node; node = node->getNext())
	{
		node->updateTreeView(hWndTv);
	}
}

// Fills in TVITEM structure for TREEVIEW
// using internal model information
void CustomTree::prepareItem(TVITEM *item)
{
	if (item->pszText == NULL)
	{
		item->pszText = (LPWSTR) malloc(64 * sizeof(TCHAR));
	}

	item->mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_STATE;

	if (this->checkCategory() && this->getFirstChild())
	{
		wsprintf(item->pszText, L"%s (%d%%)", this->caption, this->getPercent());
	}
	else
	{
		wsprintf(item->pszText, L"%s", this->caption);
	}

	item->mask = TVIF_TEXT |  TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
	item->stateMask = TVIS_EXPANDED;
	if (this->checkCategory())
	{
		if (this->isExpanded)
		{
			item->state = TVIS_EXPANDED;
			item->iImage = 0; // folder picture
			item->iSelectedImage = 0;
		}
		else
		{
			item->state = 0;
			item->iImage = 1; // folder picture
			item->iSelectedImage = 1;
		}
	}
	else
	{
		if (this->getPercent() == 100)
		{
			item->iImage = 3; // check box marked
			item->iSelectedImage = 3;
		}
		else
		{
			item->iImage = 2; // empty check box
			item->iSelectedImage = 2;
		}
	}
}

void CustomTree::renderJSON(HANDLE hFile)
{
	TCHAR buf[10];
	CustomTree *node = NULL;
	FileAppendString(hFile, L"{");
	FileAppendString(hFile, L"'caption':'");
	FileAppendString(hFile, this->getCaptionP());
	FileAppendString(hFile, L"','type':'");
	FileAppendString(hFile, this->checkCategory() ? L"category" : L"record");
	FileAppendString(hFile, L"','percent':'");
	wsprintf((LPWSTR)buf, L"%d", this->getPercent());
	FileAppendString(hFile, (LPCWSTR)&buf);
	FileAppendString(hFile, L"','expanded':'");
	FileAppendString(hFile, this->isExpanded ? L"yes" : L"no");
	FileAppendString(hFile, L"','children':{");
	for (node = this->getFirstChild(); node; node = node->getNext())
	{
		if (node != this->getFirstChild())
		{
			FileAppendString(hFile, L",");
		}
		node->renderJSON(hFile);
	}
	FileAppendString(hFile, L"}}");
}

// This method will parse a given string for object fields
// and, presumably, children.
// RETURNS the number of characters parsed
UINT CustomTree::parseJSON(LPCWSTR buffer)
{
	LPCWSTR start = buffer;
	TCHAR tmp[64];
	TCHAR tmp2[64];
	UINT i;

	// do nothing if no JSON met
	if (*buffer != '{') return 0;

	buffer++;
	while (*buffer != '}')
	{
		// option is met, read into temp
		if (*buffer == '\'')
		{
			buffer++;
			i = 0;
			while (*buffer != '\'')
			{
				tmp[i] = *buffer;
				buffer++;
				i++;
			}
			tmp[i] = '\0';

			// expect for semicolon
			buffer++;
			if (*buffer != ':') return 0;

			// if there is another string, read it
			buffer++;
			i = 0;
			if (*buffer == '\'')
			{
				while (*buffer != '\'')
				{
					tmp2[i] = *buffer;
					i++;
					buffer++;
				}
			}
			tmp2[i] = '\0';

			// now check the buffer value
			if (!wcscmp(tmp, L"caption"))
			{
				wcscpy_s(this->caption, wcslen(tmp2), tmp2);
			}
			else if (!wcscmp(tmp, L"type"))
			{
				this->isCategory = !wcscmp(tmp2, L"category");
			}
			else if (!wcscmp(tmp, L"percent"))
			{
				this->percentFilled = _wtoi(tmp2);
			}
			else if (!wcscmp(tmp, L"expanded"))
			{
				this->setExpanded(!wcscmp(tmp2, L"yes"));
			}
			else if (!wcscmp(tmp, L"children"))
			{
				// no list as value, fail
				if (*buffer != '{') return 0;

				// parse list
				while (1)
				{
					buffer++;

					// allocate node
					CustomTree *node = new CustomTree();
					if (node->parseJSON(buffer) != 0)
					{
						this->addChild(node);
					}
					else
					{
						delete node;
						break;
					}
				}

				if (*buffer != '}') return 0;
			}
			else
			{
				// unknown option, fail
				return 0;
			}
		}
		buffer++;
	}

	return (buffer-start);
}