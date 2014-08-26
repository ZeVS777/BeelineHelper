//---------------------------------------------------------------------------

#pragma hdrstop

#include "HelperFunctions.h"
//---------------------------------------------------------------------------

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

bool ReadConfigsFromIniFile(std::wstring & comPort, std::vector<std::wstring> & addresses)
{
	const wchar_t* fileName = L"config.ini";
	std::wifstream inf(fileName);
	if (!inf) {
		return false;
	}
	else
	{
		bool comPortRead = false;
		bool addressesRead = false;
		while (inf)
		{
			std::wstring strInput;
			// read stuff from the file into a string and print it
			getline(inf, strInput);
			if (comPortRead == true) {
			   comPort = strInput;
			   comPortRead = false;
			}
			if (strInput == L"[COM_PORT]") {
				 comPortRead = true;
			}
			if (addressesRead == true) {
				 if (strInput == L"#THE END") {
					addressesRead = false;
				 }
				 else
				 {
					 addresses.push_back(strInput);
				 }
			}
			if (strInput == L"[ADDRESSES]") {
				 addressesRead = true;
			}
		}
		inf.close();
    }
	return true;
}



bool CreateConfigFile(std::wstring comPort, TStrings *items)
{
   const wchar_t* fileName = L"config.ini";
   std::wofstream otf(fileName);
   if (!otf) {
	   return false;
   }
   otf << L"[COM_PORT]\n";
   otf << comPort;
   otf << L"\n[ADDRESSES]\n";
   for (int i = 0; i < items->Count; i++) {
		 otf << items->Strings[i].w_str() << L"\n";
   }
   otf.close();
   return true;
}

#pragma package(smart_init)
