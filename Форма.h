//---------------------------------------------------------------------------

#ifndef ФормаH
#define ФормаH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
//---------------------------------------------------------------------------
class TFormMain : public TForm
{
__published:	// IDE-managed Components
	TEdit *EditIP;
	TEdit *EditBarCode;
	TComboBox *ComboBoxAdress;
	TEdit *EditPorts;
	TButton *ButtonCopyToClipboard;
	TButton *ButtonPutty;
	TButton *ButtonTextFile;
	TRadioButton *RadioButton9600;
	TRadioButton *RadioButton115200;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label4;
	TEdit *EditComPort;
	void __fastcall ButtonCopyToClipboardClick(TObject *Sender);
	void __fastcall ButtonTextFileClick(TObject *Sender);
	void __fastcall ButtonPuttyClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TFormMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormMain *FormMain;
//---------------------------------------------------------------------------
#endif
