// DriverHunter.cpp : main project file.

#include "stdafx.h"
#include "UIDialog.h"


using namespace System;
using namespace DriverHunter;

int main(array<System::String ^> ^args)
{
	System::Windows::Forms::Application::EnableVisualStyles();
	System::Windows::Forms::Application::SetCompatibleTextRenderingDefault(false);

	auto dialog = gcnew UIDialog();
	System::Windows::Forms::Application::Run(dialog);
	return 0;
}
