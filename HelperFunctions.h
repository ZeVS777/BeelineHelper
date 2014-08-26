//---------------------------------------------------------------------------

#ifndef HelperFunctionsH
#define HelperFunctionsH
#include <vcl.h>
#include <boost\regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
//---------------------------------------------------------------------------

//Перечисление ошибок возвращаемое функцией CheckFieldsForError
enum Error
{
  ERROR_NO,
  ERROR_IP,     //ошибка в поле ip адрес (не соответствие паттерну ip)
  ERROR_ADRESS, //ошибка в поле адрес (содержит запрещающие символы)
  ERROR_BAR,    //ошибка в поле бар-код (содержит запрещающие символы)
  ERROR_PORTS   //ошибка в поле tag-порты  (содержит запрещающие символы)
};

//Проверка строки на содержание не латинских символов
//return: true(если содержит) и false иначе
bool NotOnlyLatinAndNumbersChars(UnicodeString txt);

//Проверка строки на содержание не цифровых символов + ',' и '-'
//return: true(если содержит) и false иначе
bool NotOnlyNumbersAndSpecialChars(UnicodeString txt);

//Проверка полей на првильность заполнения
//returns: Error с кодом ошибки (перечисление Error описано выше)
Error CheckFieldsForError(TEdit* EditIP, TEdit* EditBarCode,
						  TComboBox* ComboBoxAdress, TEdit* EditPorts);

//Определяет из строки с ip адресом настройки для свича и возвращает их
//в ввиде вектора. [0]-pass, [1]-tag, [2]-route
std::vector<std::wstring> GivePassTagAndRoute(std::wstring ip);

//Состовляет конфигурацию для свича по данным полей
std::wstring MakeConfiguration(TEdit *EditIP,
				  TEdit *EditBarCode,
				  TComboBox *ComboBoxAdress,
				  TEdit *EditPorts);

//Записывает настройки в файл
bool WriteToFile(std::wstring configuration);

//Считывает настройки программы из файла config.ini
bool ReadConfigsFromIniFile(std::wstring & comPort, std::vector<std::wstring> & addresses);

//Если не находит файл настройки программы, программа создаст его
bool CreateConfigFile(std::wstring comPort, TStrings *items);
#endif
