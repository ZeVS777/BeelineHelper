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

//������������ ������ ������������ �������� CheckFieldsForError
enum Error
{
  ERROR_NO,
  ERROR_IP,     //������ � ���� ip ����� (�� ������������ �������� ip)
  ERROR_ADRESS, //������ � ���� ����� (�������� ����������� �������)
  ERROR_BAR,    //������ � ���� ���-��� (�������� ����������� �������)
  ERROR_PORTS   //������ � ���� tag-�����  (�������� ����������� �������)
};

//�������� ������ �� ���������� �� ��������� ��������
//return: true(���� ��������) � false �����
bool NotOnlyLatinAndNumbersChars(UnicodeString txt);

//�������� ������ �� ���������� �� �������� �������� + ',' � '-'
//return: true(���� ��������) � false �����
bool NotOnlyNumbersAndSpecialChars(UnicodeString txt);

//�������� ����� �� ����������� ����������
//returns: Error � ����� ������ (������������ Error ������� ����)
Error CheckFieldsForError(TEdit* EditIP, TEdit* EditBarCode,
						  TComboBox* ComboBoxAdress, TEdit* EditPorts);

//���������� �� ������ � ip ������� ��������� ��� ����� � ���������� ��
//� ����� �������. [0]-pass, [1]-tag, [2]-route
std::vector<std::wstring> GivePassTagAndRoute(std::wstring ip);

//���������� ������������ ��� ����� �� ������ �����
std::wstring MakeConfiguration(TEdit *EditIP,
				  TEdit *EditBarCode,
				  TComboBox *ComboBoxAdress,
				  TEdit *EditPorts);

//���������� ��������� � ����
bool WriteToFile(std::wstring configuration);

//��������� ��������� ��������� �� ����� config.ini
bool ReadConfigsFromIniFile(std::wstring & comPort, std::vector<std::wstring> & addresses);

//���� �� ������� ���� ��������� ���������, ��������� ������� ���
bool CreateConfigFile(std::wstring comPort, TStrings *items);
#endif
