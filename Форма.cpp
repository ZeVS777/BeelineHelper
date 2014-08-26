//---------------------------------------------------------------------------

#include <vcl.h>
#include "HelperFunctions.h"
#include <clipbrd.hpp>
//#pragma hdrstop

#include "Форма.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormMain *FormMain;
//---------------------------------------------------------------------------
__fastcall TFormMain::TFormMain(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------




void __fastcall TFormMain::ButtonCopyToClipboardClick(TObject *Sender)
{
   Error errorCode = CheckFieldsForError(EditIP, EditBarCode, ComboBoxAdress, EditPorts);

   switch (errorCode) {
	 case ERROR_IP:
		Application->MessageBoxW(
		  L"Не правильный формат IP адреса",
		  L"Внимание",
		  MB_OK);
		EditIP->SetFocus();
		EditIP->SelectAll();
		break;
	 case ERROR_ADRESS:
		Application->MessageBoxW(
		  L"Только цифры или латинские буквы могут быть в поле Адрес",
		  L"Внимание",
		  MB_OK);
		ComboBoxAdress->SetFocus();
		ComboBoxAdress->SelectAll();
		break;
	 case ERROR_BAR:
		Application->MessageBoxW(
		  L"Только цифры или латинские буквы могут быть в поле Бар-код",
		  L"Внимание",
		  MB_OK);
		EditBarCode->SetFocus();
		EditBarCode->SelectAll();
		break;
	 case ERROR_PORTS:
		Application->MessageBoxW(
		  L"Только цифры '-' или ',' могут быть в поле Tag порты",
		  L"Внимание",
		  MB_OK);
		EditPorts->SetFocus();
		EditPorts->SelectAll();
		break;
	 default:
		std::wstring configuration = MakeConfiguration(EditIP,
													  EditBarCode,
													  ComboBoxAdress,
													  EditPorts);
		if(!WriteToFile(configuration))
		{
			Application->MessageBoxW(
		  L"Возникла проблема при записи файла",
		  L"Внимание",
		  MB_OK);
		}
		else
			Clipboard()->AsText =  (configuration + L"\n").c_str();
		break;
   }
}
//---------------------------------------------------------------------------


//bool executeCommandLine(CString cmdLine, DWORD & exitCode)
//{
//   PROCESS_INFORMATION processInformation = {0};
//   STARTUPINFO startupInfo                = {0};
//   startupInfo.cb                         = sizeof(startupInfo);
//   int nStrBuffer                         = cmdLine.GetLength() + 50;
//
//
//   // Create the process
//   BOOL result = CreateProcess(NULL, cmdLine.GetBuffer(nStrBuffer),
//							   NULL, NULL, FALSE,
//							   NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
//							   NULL, NULL, &startupInfo, &processInformation);
//   cmdLine.ReleaseBuffer();
//
//
//   if (!result)
//   {
//	  // CreateProcess() failed
//      // Get the error from the system
//	  LPVOID lpMsgBuf;
//      DWORD dw = GetLastError();
//	  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
//                    NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
//
//	  // Display the error
//      CString strError = (LPTSTR) lpMsgBuf;
//	  TRACE(_T("::executeCommandLine() failed at CreateProcess()\nCommand=%s\nMessage=%s\n\n"), cmdLine, strError);
//
//      // Free resources created by the system
//	  LocalFree(lpMsgBuf);
//
//      // We failed.
//	  return FALSE;
//   }
//   else
//   {
//	  // Successfully created the process.  Wait for it to finish.
//	  WaitForSingleObject( processInformation.hProcess, INFINITE );
//
//      // Get the exit code.
//      result = GetExitCodeProcess(processInformation.hProcess, &exitCode);
//
//      // Close the handles.
//	  CloseHandle( processInformation.hProcess );
//	  CloseHandle( processInformation.hThread );
//
//	  if (!result)
//	  {
//		 // Could not get exit code.
//		 TRACE(_T("Executed command but couldn't get exit code.\nCommand=%s\n"), cmdLine);
//		 return FALSE;
//	  }
//
//
//	  // We succeeded.
//	  return TRUE;
//   }
//}

//void execute(wchar_t * cmd)
//{
//   PROCESS_INFORMATION processInformation = {0};
//   STARTUPINFO startupInfo                = {0};
//   startupInfo.cb                         = sizeof(startupInfo);
//   CreateProcess(NULL, cmd,
//							   NULL, NULL, FALSE,
//							   NORMAL_PRIORITY_CLASS,
//							   NULL, NULL, &startupInfo, &processInformation);
//   WaitForSingleObject( processInformation.hProcess, INFINITE );
//   unsigned long exitCode;
//   GetExitCodeProcess(processInformation.hProcess, &exitCode);
//   CloseHandle( processInformation.hProcess );
//   CloseHandle( processInformation.hThread );
//}


void __fastcall TFormMain::ButtonTextFileClick(TObject *Sender)
{
	 ShellExecute(GetDesktopWindow(), L"open", L"notepad", L"base.txt", NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ButtonPuttyClick(TObject *Sender)
{
   std::wstring cmdLine = L"-serial ";
   cmdLine += EditComPort->Text.w_str();
   cmdLine += L" -sercfg ";
   cmdLine += (RadioButton115200->Checked ? L"115200,8,n,1,X" : L"9600,8,n,1,X");
   if((int)ShellExecute(GetDesktopWindow(), L"open", L"putty_zmodem.exe", cmdLine.c_str(), NULL, SW_SHOWNORMAL)<=32)
	 Application->MessageBoxW(
		  L"                                  Не возможно открыть программу Putty.\n Файл putty_zmodem.exe должен находиться вместе с данной программой",
		  L"Внимание",
		  MB_OK);
}
//---------------------------------------------------------------------------


void __fastcall TFormMain::FormCreate(TObject *Sender)
{
	std::wstring comPort;
	std::vector<std::wstring> addresses;
	if (ReadConfigsFromIniFile(comPort,addresses)) {
		  EditComPort->Text = comPort.c_str();
		  ComboBoxAdress->Items->Clear();
		  for (std::vector<std::wstring>::iterator it = addresses.begin() ; it != addresses.end(); ++it) {
			  ComboBoxAdress->Items->Add(((std::wstring)*it).c_str());
		  }
	}
	else
	{
		if	(!CreateConfigFile(EditComPort->Text.w_str(), ComboBoxAdress->Items))
		{
			 Application->MessageBoxW(
		  L"Возникла проблема при создании файла настроек",
		  L"Внимание",
		  MB_OK);
        }
	}
}
//---------------------------------------------------------------------------


