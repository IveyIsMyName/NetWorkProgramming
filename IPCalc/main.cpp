#define _CRT_SECURE_NO_WARNINGS
#include<Windows.h>
#include <CommCtrl.h>
#include<cstdio>
#include"resource.h"

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, INT nCmdShow)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc, 0);
	return 0;
}

BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		HWND hSpin = GetDlgItem(hwnd, IDC_SPIN_PREFIX);
		SendMessage(hSpin, UDM_SETRANGE, 0, MAKELPARAM(30, 1));
		SendMessage(hSpin, UDM_SETPOS, 0, 1);
		//SetDlgItemInt(hwnd, IDC_EDIT_PREFIX, 1, FALSE);
	}
	break;
	case WM_COMMAND:
	{
		HWND hIPaddress = GetDlgItem(hwnd, IDC_IPADDRESS);
		HWND hIPmask = GetDlgItem(hwnd, IDC_IPMASK);
		HWND hIPprefix = GetDlgItem(hwnd, IDC_EDIT_PREFIX);
		DWORD dwIPaddress = 0;
		DWORD dwIPmask = 0;
		switch (LOWORD(wParam))
		{
		case IDC_IPADDRESS:
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				SendMessage(hIPaddress, IPM_GETADDRESS, 0, (LPARAM)&dwIPaddress);
				if (FIRST_IPADDRESS(dwIPaddress) < 128)SendMessage(hIPmask, IPM_SETADDRESS, 0, 0xFF000000);
				else if (FIRST_IPADDRESS(dwIPaddress) < 192)SendMessage(hIPmask, IPM_SETADDRESS, 0, 0xFFFF0000);
				else if (FIRST_IPADDRESS(dwIPaddress) < 224)SendMessage(hIPmask, IPM_SETADDRESS, 0, 0xFFFFFF00);
			}
		}
		break;
		case IDC_IPMASK:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				SendMessage(hIPmask, IPM_GETADDRESS, 0, (LPARAM)&dwIPmask);
				DWORD count = 0;
				for (DWORD i = dwIPmask; 0x80000000 & i; i <<= 1, count++);
				CHAR szIPprefix[3] = "";
				sprintf(szIPprefix, "%i", count);
				SendMessage(hIPprefix, WM_SETTEXT, 0, (LPARAM)szIPprefix);
			}
			break;
		/*case IDC_EDIT_PREFIX:
		{
			if (HIWORD(wParam) == EN_CHANGE)
			{
				DWORD dwIPmask = UINT_MAX;
				CHAR szIPprefix[3];
				SendMessage(hIPprefix, WM_GETTEXT, 3, (LPARAM)szIPprefix);
				DWORD dwIPprefix = atoi(szIPprefix);
				dwIPmask <<= (32 - dwIPprefix);
				SendMessage(hIPmask, IPM_SETADDRESS, 0, dwIPmask);
			}
			break;
		}
		break;*/
		case IDOK:
			break;
		case IDCANCEL:
			EndDialog(hwnd, 0);
		}

	}
	break;
	case WM_NOTIFY:
	{
		NMUPDOWN* pNMUpDown = (NMUPDOWN*)lParam;
		if (pNMUpDown->hdr.idFrom == IDC_SPIN_PREFIX)
		{
			if (pNMUpDown->hdr.code == UDN_DELTAPOS)
			{
				HWND hIPmask = GetDlgItem(hwnd, IDC_IPMASK);
				DWORD dwIPmask = UINT_MAX;
				int iPos = pNMUpDown->iPos + pNMUpDown->iDelta;

				if (iPos < 1) iPos = 1;
				if (iPos > 30) iPos = 30;

				dwIPmask <<= (32 - iPos);
				SendMessage(hIPmask, IPM_SETADDRESS, 0, dwIPmask);

				CHAR szIPprefix[3];
				sprintf(szIPprefix, "%i", iPos);
				SetDlgItemText(hwnd, IDC_EDIT_PREFIX, szIPprefix);

				return TRUE; 
			}
		}
	}
	break;
	case WM_CLOSE:
		EndDialog(hwnd, 0);
	}
	return FALSE;
}