//---------------------------------------------------------------------------

#include <vcl.h>
#include <boost\regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <clipbrd.hpp>
#include <vector>
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


enum Error
{
  ERROR_NO,
  ERROR_IP,
  ERROR_ADRESS,
  ERROR_BAR,
  ERROR_PORTS
};

bool NotOnlyLatinAndNumbersChars(UnicodeString txt)
{
	wchar_t * text = txt.w_str();
	for (unsigned int i = 0; i < wcslen(text); i++) {
	  wchar_t ch = text[i];
	  if (ch >= L'\u0021' && ch <= L'\u00a0'
	   || ch == ' ')
	  {}
	  else
	  {
		return true;
	  }
	}

	return false;
}

bool NotOnlyNumbersAndSpecialChars(UnicodeString txt)
{
	std::wstring t = txt.w_str();
	boost::algorithm::trim(t);
	wchar_t * text = (wchar_t*)t.c_str();

	for (unsigned int i = 0; i < wcslen(text); i++) {
	  wchar_t ch = text[i];
	  if (ch >= L'0' && ch <= L'9'
	  //TODO: Проверять, что перед и после тире стоит цифра. То же для запятой
	   || ch == '-' || ch == ',')
	  {}
	  else
	  {
		return true;
	  }
	}

	return false;
}


Error CheckFieldsForError(TEdit* EditIP, TEdit* EditBarCode, TComboBox* ComboBoxAdress, TEdit* EditPorts)
{
   Error errorCode = ERROR_NO;
   //Проверка поля IP
   std::wstring str = EditIP->Text.w_str();
   std::string st(str.begin(),str.end());
   boost::algorithm::trim(st);
   const boost::regex rgx("^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$");
   if (!boost::regex_match(st, rgx))
   {
	   errorCode = ERROR_IP ;
   }
   //Проверка поля Адрес (только латинские буквы и цифры + 'пробел')
   else if (NotOnlyLatinAndNumbersChars(ComboBoxAdress->Text))
   {
	   errorCode = ERROR_ADRESS;
   }
   //Проверка поля Бар-код (только латинские буквы и цифры + 'пробел')
   else if (NotOnlyLatinAndNumbersChars(EditBarCode->Text))
   {
	   errorCode = ERROR_BAR;
   }
   else if (NotOnlyNumbersAndSpecialChars(EditPorts->Text))
   {
	   errorCode = ERROR_PORTS;
   }

   return errorCode;
}

std::vector<std::wstring> GivePassTagAndRoute(std::wstring ip)
{
	 std::vector<std::wstring> passTagAndRoute;
	 std::vector<std::wstring> ipSplit;
	 split(ipSplit, ip, boost::is_any_of(L"."));


	 int thrdIP = boost::lexical_cast<int>(ipSplit[2]);
	 if (ipSplit[1]==L"6") {
		  if (thrdIP<100) {
			  passTagAndRoute.push_back(L"54");
			  passTagAndRoute.push_back(L"400");
		  }
		  else {
			  passTagAndRoute.push_back(L"51");
			  passTagAndRoute.push_back(L"401");
          }
	 }
	 else {
		  if (thrdIP<200) {
			 passTagAndRoute.push_back(L"59");
			 passTagAndRoute.push_back(L"400");
		  }
		  else {
			 if (thrdIP>226) {
			   passTagAndRoute.push_back(L"60");
			   passTagAndRoute.push_back(L"408");
			 }
			 else {
			   passTagAndRoute.push_back(L"60");
			   passTagAndRoute.push_back(L"400");
             }
		  }
     }

	 int lastIP =  boost::lexical_cast<int>(ipSplit[3]);
	 if (lastIP > 129) {
	   passTagAndRoute.push_back(ipSplit[0] + L"." + ipSplit[1] + L"." + ipSplit[2] + L".129");
	 }
	 else {
	   passTagAndRoute.push_back(ipSplit[0] + L"." + ipSplit[1] + L"." + ipSplit[2] + L".1");
	 }

	 return passTagAndRoute;
}

std::wstring MakeConfiguration(TEdit *EditIP,
				  TEdit *EditBarCode,
				  TComboBox *ComboBoxAdress,
				  TEdit *EditPorts)
{
   std::wstring ip = EditIP->Text.w_str();
   std::wstring bar = EditBarCode->Text.w_str();
   std::wstring address = ComboBoxAdress->Text.w_str();
   std::wstring ports = EditPorts->Text.w_str();
   boost::algorithm::trim(ip);
   boost::algorithm::trim(bar);
   boost::algorithm::trim(address);
   boost::algorithm::trim(ports);
   std::vector<std::wstring> options = GivePassTagAndRoute(ip);
   std::wstring configuration = L"create account admin admin\n";
   configuration += options[0];
   configuration += L" pass\n" ;
   configuration += options[0];
   configuration += L" pass\n";
   configuration += L"create vlan management tag ";
   configuration += options[1];
   configuration += L"\n";
   configuration += L"config vlan management add tagged ";
   configuration += ports;
   configuration += L"\n";
   configuration += L"create iproute default ";
   configuration += options[2];
   configuration += L"\n";
   configuration += L"config ipif System vlan management ipaddress ";
   configuration += ip;
   configuration += L"/21 state enable\n";
   configuration += L"config snmp system_location ";
   configuration += address;
   configuration += L"\n";
   configuration += L"config snmp system_name ";
   configuration += bar;
   configuration += L"\n";
   configuration += L"enable snmp\n";
   configuration += L"save";
   return configuration;
}

bool WriteToFile(std::wstring configuration)
{
   const wchar_t* fileName = L"base.txt";

   std::time_t now = time(0);
   std::tm *ltm = localtime(&now);
   std::wstringstream ss;
   ss  << (ltm->tm_mday < 10 ?  0 :
			ltm->tm_mday < 20 ?  1 :
			ltm->tm_mday < 30 ?  2 : 3)
		<< ltm->tm_mday % 10
		<< '.'
		<< (ltm->tm_mon < 10 ?  0 : 1 )
		<< 1 + ltm->tm_mon % 10
		<< '.'
		<< 1900 + ltm->tm_year;
   std::wstring strDate = L"-------------------------------"
						  + ss.str()
						  + L"-------------------------------";


   std::wifstream inf(fileName);
   if (!inf) {
	   std::wofstream otf(fileName);
	   if (!otf) {
		   return false;
	   }
	   otf << (strDate + L"\n" + configuration + L"\n");
	   otf.close();
   }
   else
   {
	   std::wstring strOutput;

		while (inf)
		{
			std::wstring strInput;
			// read stuff from the file into a string and print it
			getline(inf, strInput);
			strOutput += (L"\n" + strInput);
		}
	   inf.close();
	   std::wofstream otf(fileName);
	   if (!otf) {
		   return false;
	   }

	   otf << (strDate + L"\n" + configuration + L"\n" + strOutput);
	   otf.close();
   }

   return true;
}

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
   if (RadioButton115200->Checked)
	 ShellExecute(GetDesktopWindow(), L"open", L"putty_zmodem.exe", L"-serial COM3 -sercfg 115200,8,n,1,X", NULL, SW_SHOWNORMAL);
   else 
	 ShellExecute(GetDesktopWindow(), L"open", L"putty_zmodem.exe", L"-serial COM3 -sercfg 9600,8,n,1,X", NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------

