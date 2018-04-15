#include "stdafx.h"
#include "SmartProj.h"

extern TCHAR				addedItem[64];

// Message handler for node addition box
INT_PTR CALLBACK Prompt(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		{
			// Create a Done button and size it.  
			SHINITDLGINFO shidi;
			shidi.dwMask = SHIDIM_FLAGS;
			shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
			shidi.hDlg = hDlg;
			SHInitDialog(&shidi);

			HWND hwEdit = GetDlgItem(hDlg, IDC_EDIT1);
			SetWindowText(hwEdit, (LPCWSTR)&addedItem);
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			HWND hwEdit = GetDlgItem(hDlg, IDC_EDIT1);
			GetWindowText(hwEdit, (LPWSTR)&addedItem, sizeof(addedItem));
			SipShowIM(SIPF_OFF);
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		if (HIWORD(wParam) == EN_SETFOCUS)
		{
			SHSipPreference(hDlg, SIP_UP);
		}
		break;

	case WM_CLOSE:
		SipShowIM(SIPF_OFF);
		EndDialog(hDlg, message);
		return TRUE;

	}
	return (INT_PTR)FALSE;
}