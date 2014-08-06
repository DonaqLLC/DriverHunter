#pragma once

#include "DeviceInfo.h"

namespace DriverHunter
{
	using namespace System;
	using namespace System::Windows::Forms;
	using namespace System::Collections::Generic;

	ref class UIDialog 
		: Form
	{
	private:
		ListBox^ lstDevices;
		ListBox^ lstDrivers;
		Button^ btnRemoveDevice;
		Button^ btnRemoveDrivers;

		List<DeviceInfo^>^ usbDevices;

		void InitControls(){
			auto lblInfo = gcnew Label();
			lstDevices = gcnew ListBox();
			lstDrivers = gcnew ListBox();
			btnRemoveDevice = gcnew Button();
			btnRemoveDrivers = gcnew Button();

			Reflection::Assembly^ pxAssembly = Reflection::Assembly::GetExecutingAssembly();

			this->Text = String::Format("Driver Hunter - v.{0}.{1} (c) Donaq LLC", pxAssembly->GetName()->Version->Major, pxAssembly->GetName()->Version->Minor);

			String^ pxResName = pxAssembly->GetName()->Name + ".Resources";
			auto s_pxResourceManager = (gcnew Resources::ResourceManager(pxResName,pxAssembly));

			Object^ iconResource = s_pxResourceManager->GetObject(L"App.Icon");
			if(iconResource != nullptr)
				this->Icon = safe_cast<System::Drawing::Icon^>(iconResource);

			this->Width = 640;
			this->Height = 480;



			this->PerformLayout();
			this->SuspendLayout();

			SplitContainer^ devicesDriversSplit = gcnew SplitContainer();
			
			lblInfo->Text = "Select a device on the left to view driver details.";
			lblInfo->Top = 5;
			lblInfo->Left = 5;
			lblInfo->AutoSize = true;
			lblInfo->Visible = true;

			lstDevices->Top = 0;
			lstDevices->Left = 0;
			lstDevices->Width = 100;
			lstDevices->Height = 100;
			lstDevices->Visible = true;
			lstDevices->Dock = DockStyle::Fill;
			lstDrivers->Top = 0;
			lstDrivers->Left = 100;
			lstDrivers->Width = 100;
			lstDrivers->Height = 100;
			lstDrivers->Visible = true;
			lstDrivers->Dock = DockStyle::Fill;

			devicesDriversSplit->SuspendLayout();
			devicesDriversSplit->Left = 5;
			devicesDriversSplit->Top = 25;
			devicesDriversSplit->Width = Width - 25;
			devicesDriversSplit->Height = Height - 110;
			devicesDriversSplit->Visible = true;
			devicesDriversSplit->Anchor = AnchorStyles::Top | AnchorStyles::Right | AnchorStyles::Left | AnchorStyles::Bottom;
			devicesDriversSplit->Panel1->Controls->Add(lstDevices);
			devicesDriversSplit->Panel2->Controls->Add(lstDrivers);
			devicesDriversSplit->Panel1MinSize = 200;
			devicesDriversSplit->Panel2MinSize = 200;
			devicesDriversSplit->ResumeLayout();

			btnRemoveDevice->Top = Height - 75;
			btnRemoveDevice->Left = 5;
			btnRemoveDevice->Height = 30;
			btnRemoveDevice->Width = 100;
			btnRemoveDevice->AutoSize = false;
			btnRemoveDevice->Text = "Remove Device";
			btnRemoveDevice->Visible = true;
			btnRemoveDevice->Enabled = false;
			btnRemoveDevice->Anchor = AnchorStyles::Left | AnchorStyles::Bottom;

			btnRemoveDrivers->Top = Height - 75;
			btnRemoveDrivers->Left = 5 + btnRemoveDevice->Width + 10;
			btnRemoveDrivers->Height = 30;
			btnRemoveDrivers->Width = 100;
			btnRemoveDrivers->AutoSize = false;
			btnRemoveDrivers->Text = "Uninstall Driver";
			btnRemoveDrivers->Visible = true;
			btnRemoveDrivers->Enabled = false;
			btnRemoveDrivers->Anchor = AnchorStyles::Left | AnchorStyles::Bottom;

			this->Controls->Add(lblInfo);
			this->Controls->Add(devicesDriversSplit);
			this->Controls->Add(btnRemoveDevice);
			this->Controls->Add(btnRemoveDrivers);

			this->ResumeLayout();

			btnRemoveDevice->Click += gcnew EventHandler(this, &DriverHunter::UIDialog::btnRemoveDevice_Click);
			btnRemoveDrivers->Click += gcnew EventHandler(this, &UIDialog::btnRemoveDrivers_Click);

			lstDevices->SelectedIndexChanged += gcnew EventHandler(this, &UIDialog::lstDevices_IndexChanged);
			lstDrivers->SelectedIndexChanged += gcnew EventHandler(this, &UIDialog::lstDrivers_IndexChanged);
		}

		bool InitDeviceList()
		{
			usbDevices = gcnew List<DeviceInfo^>();

			// enumerate all devices in the dev_class USB Video

			HDEVINFO hDevInfo;
			SP_DEVINFO_DATA DeviceInfoData;
			DWORD i;

			GUID usbclass = GUID_DEVCLASS_USB;
			// Create a HDEVINFO with all present devices.
			hDevInfo = SetupDiGetClassDevs(NULL, 0, 0, DIGCF_ALLCLASSES);
       
			if (hDevInfo == INVALID_HANDLE_VALUE)
			{
				// Insert error handling here.
				return false;
			}
       
			// Enumerate through all devices in Set.
			Console::WriteLine("Enumerating all devices..");

			DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
			for (i=0; SetupDiEnumDeviceInfo(hDevInfo,i,&DeviceInfoData); i++)
			{ 
				String^ devDescription = GetDeviceStringProp(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC);
				String^ devMfg = GetDeviceStringProp(hDevInfo, &DeviceInfoData, SPDRP_MFG);
				String^ devSetupClass = GetDeviceStringProp(hDevInfo, &DeviceInfoData, SPDRP_CLASS);
				String^ devClassGuid = GetDeviceStringProp(hDevInfo, &DeviceInfoData, SPDRP_CLASSGUID);
				UInt32 devType = GetDeviceNumericProp(hDevInfo, &DeviceInfoData, SPDRP_DEVTYPE);

				if(devDescription != nullptr && devMfg != nullptr)
				{
					if (devSetupClass == nullptr)
						continue;

					if(String::Compare(devSetupClass, "USB", true) != 0 
						&& String::Compare(devSetupClass, "IMAGE", true) != 0
						&& String::Compare(devSetupClass, "MEDIA", true) != 0)
					continue;

					Console::WriteLine(" * found interesting device: " + devSetupClass + " - " + devDescription);

					DeviceInfo^ dInfo = gcnew DeviceInfo(GetDeviceInstanceID(hDevInfo, &DeviceInfoData));
					dInfo->Description = devDescription;
					dInfo->Mfg = devMfg;
					dInfo->DeviceType = devType;
					dInfo->Class = devSetupClass;
					dInfo->ClassGuid = devClassGuid;
					
				    lstDevices->Items->Add(dInfo);
		
					Console::WriteLine("    building drivers list..");
					if(SetupDiBuildDriverInfoList(hDevInfo, &DeviceInfoData, SPDIT_COMPATDRIVER))
					{
						SP_DRVINFO_DATA drvData;
						   
						drvData.cbSize = sizeof(SP_DRVINFO_DATA);

						for(int drvIdx = 0; SetupDiEnumDriverInfo(hDevInfo, &DeviceInfoData, SPDIT_COMPATDRIVER, drvIdx, &drvData); drvIdx++)
						{
							DriverInfo^ drvInfo = gcnew DriverInfo();

							drvInfo->Description = gcnew String(drvData.Description);
							drvInfo->Manufacturer = gcnew String(drvData.MfgName);
							drvInfo->Provider = gcnew String(drvData.ProviderName);

							drvInfo->Version = gcnew Version((drvData.DriverVersion >> 48) & 0xFFFF, (drvData.DriverVersion >> 32) & 0xFFFF, (drvData.DriverVersion >> 16) & 0xFFFF, drvData.DriverVersion & 0xFFFF);

							DWORD requiredSize = 0;
							SetupDiGetDriverInfoDetail(hDevInfo, &DeviceInfoData, &drvData, NULL, NULL, &requiredSize);

							PSP_DRVINFO_DETAIL_DATA pDrvDataDetail = (PSP_DRVINFO_DETAIL_DATA)malloc(requiredSize);
							pDrvDataDetail->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

							if(SetupDiGetDriverInfoDetail(hDevInfo, &DeviceInfoData, &drvData, pDrvDataDetail, requiredSize, &requiredSize))
							{
								drvInfo->InfFile = gcnew String(pDrvDataDetail->InfFileName);
								TCHAR* pText = (TCHAR*)(&pDrvDataDetail->HardwareID[0]);
								drvInfo->HardwareIds = gcnew String(pText);
								if(pDrvDataDetail->CompatIDsOffset > 1)
								{	
									while(*pText != '\0')
									{
#ifdef _UNICODE
										size_t len = wcslen(pText);
#else
										size_t len = strlen(pText);
#endif
										pText = (TCHAR*)(pText + len + 1);
										if(*pText != '\0')
										{
											String^ compatibleId = gcnew String(pText);
										}
									}
								}
							}
							
							free(pDrvDataDetail);


							Console::WriteLine("     - adding driver to list");
							dInfo->Drivers->Add(drvInfo);
						}
						   

						SetupDiDestroyDriverInfoList(hDevInfo, &DeviceInfoData, SPDIT_COMPATDRIVER);
					}
				}
			}
       
       
			if ( GetLastError()!=NO_ERROR &&
				GetLastError()!=ERROR_NO_MORE_ITEMS )
			{
				   
				// Insert error handling here.
				return false;
			}
       
			//  Cleanup
			SetupDiDestroyDeviceInfoList(hDevInfo);

			lstDevices->Sorted = true;

			return true;
		}

		String^ GetDeviceStringProp(HDEVINFO devInfo, SP_DEVINFO_DATA* pDevInfoData, DWORD prop)
		{
			DWORD DataT;
			LPTSTR buffer = NULL;
			DWORD buffersize = 0;
			String^ propText = nullptr;

			try{
				//
				// Call function with null to begin with, then use the returned buffer size (doubled) to Alloc the buffer. 
				// Keep calling until success or an unknown failure.
				//
				while (!SetupDiGetDeviceRegistryProperty(devInfo, pDevInfoData, prop, &DataT, (PBYTE)buffer, buffersize, &buffersize))
				{
					if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
					{
						// Change the buffer size.
						if (buffer) LocalFree(buffer);
						// Double the size to avoid problems on W2k MBCS systems per KB 888609. 
						buffer = (LPTSTR)LocalAlloc(LPTR, buffersize * 2);
					}
					else
					{
						return nullptr;
					}
				}

				if(buffer)
				{
					propText = gcnew String(buffer);
				}
			}finally{
				if(buffer) LocalFree(buffer);
			}
			return propText;
		}

		UInt32 GetDeviceNumericProp(HDEVINFO devInfo, SP_DEVINFO_DATA* pDevInfoData, DWORD prop)
		{
			DWORD DataT;
			DWORD value = 0;
			DWORD buffersize = sizeof(DWORD);

			if(SetupDiGetDeviceRegistryProperty(devInfo, pDevInfoData, prop, NULL, (PBYTE)&value, sizeof(DWORD), NULL))
				return value;
			else
				return -1;
		}

		String^ GetDeviceInstanceID(HDEVINFO devInfo, SP_DEVINFO_DATA* pDevInfoData)
		{
			DWORD requiredBufferSize;
			String^ strDeviceInstanceId = nullptr;

			SetupDiGetDeviceInstanceId(devInfo, pDevInfoData, NULL, NULL, &requiredBufferSize);
			LPTSTR devInstanceId = (LPTSTR)LocalAlloc(LPTR, requiredBufferSize * 2);
			if(devInstanceId)
			{
				SetupDiGetDeviceInstanceId(devInfo, pDevInfoData, devInstanceId, requiredBufferSize, &requiredBufferSize);
				strDeviceInstanceId = gcnew String(devInstanceId);
				LocalFree(devInstanceId);
			}

			return strDeviceInstanceId;
		}

		HRESULT RemoveDevice(DeviceInfo^ devInfo)
		{
			HRESULT finalResult = -1;

			HDEVINFO hDevInfo;
			SP_DEVINFO_DATA DeviceInfoData;
			DWORD i;

			hDevInfo = SetupDiGetClassDevs(NULL, 0, 0, DIGCF_ALLCLASSES);
			
			if (hDevInfo == INVALID_HANDLE_VALUE)
			{
				// Insert error handling here.
				return -1;
			}
       
			// Enumerate through all devices in Set.
       
			DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
			for (i=0; SetupDiEnumDeviceInfo(hDevInfo,i,&DeviceInfoData); i++)
			{ 
				String^ instanceID = GetDeviceInstanceID(hDevInfo, &DeviceInfoData);
				if(instanceID == devInfo->InstanceID)
				{
					if(SetupDiCallClassInstaller(DIF_REMOVE, hDevInfo, &DeviceInfoData))
					{
						//found what we were looking for!
						finalResult = 0;
					}else
					{
						finalResult = GetLastError();
					}
					break;
				}
			}
       
       
			//  Cleanup
			SetupDiDestroyDeviceInfoList(hDevInfo);
			return finalResult;
		}

	protected:
		void btnRemoveDevice_Click(System::Object^ sender, System::EventArgs^ e)
		{
			if(lstDevices->SelectedItem != nullptr)
			{
				DeviceInfo^ dev = dynamic_cast<DeviceInfo^>(lstDevices->SelectedItem);
				
				DWORD errCode = RemoveDevice(dev);

				if(errCode == 0)
				{
					MessageBox::Show("The device was successfully removed", "Success", MessageBoxButtons::OK, MessageBoxIcon::Information);
				}else if(errCode = 0xFFFFFFFF)
				{
					MessageBox::Show(String::Format("The device was NOT removed!\r\n\r\nIt looks like it was already removed!"), "Warning", MessageBoxButtons::OK, MessageBoxIcon::Warning);
				
				}else
				{
					MessageBox::Show(String::Format("The device was NOT removed! Error: 0x{0:X8}\r\n\r\nThis tool must be run as Administrator!", errCode), "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
				
				}
			}
		}
		
		void btnRemoveDrivers_Click(System::Object^ sender, System::EventArgs^ e)
		{
			if(lstDrivers->SelectedItem != nullptr)
			{
				DriverInfo^ drv = dynamic_cast<DriverInfo^>(lstDrivers->SelectedItem);

				if(MessageBox::Show("Warning! You are about to completely delete the following drivers:\r\n\r\n" + drv->Description + "\r\n\r\nVersion: " + drv->Version + "\r\nManufacturer: " + drv->Manufacturer + "\r\n\r\nProceed only if you know what you are doing!", "Confirm Delete", MessageBoxButtons::YesNo, MessageBoxIcon::Question) 
					== System::Windows::Forms::DialogResult::Yes)
				{
					String^ infFileName = System::IO::Path::GetFileName(drv->InfFile);

					IntPtr p = System::Runtime::InteropServices::Marshal::StringToHGlobalAuto(infFileName);
					TCHAR* pInf = static_cast<TCHAR*>(p.ToPointer());

					if(SetupUninstallOEMInf(pInf, SUOI_FORCEDELETE, NULL))
					{
						MessageBox::Show("The " + drv->Description + " driver (" + drv->InfFile + ") was successfully removed.", "Success", MessageBoxButtons::OK, MessageBoxIcon::Information);
					}else
					{
						DWORD errCode = GetLastError();
						if(errCode == 0x02)
						{
							MessageBox::Show(String::Format("The {0} driver ({1}) was not found.\r\n\r\nPerhaps it was previously removed?!", drv->Description, drv->InfFile, errCode), "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
						}else
						{
							MessageBox::Show(String::Format("The {0} driver ({1}) could NOT be removed: Error 0x{2:X8}\r\n\r\nThis tool must be run as administrator!", drv->Description, drv->InfFile, errCode), "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
						}
					}

					System::Runtime::InteropServices::Marshal::FreeHGlobal(p);
				}
			}
		}

		void lstDevices_IndexChanged(Object^ sender, EventArgs^ e)
		{
			if(lstDevices->SelectedItem != nullptr)
			{
				DeviceInfo^ device = dynamic_cast<DeviceInfo^>(lstDevices->SelectedItem);

				lstDrivers->Items->Clear();

				lstDrivers->Items->Add(device->Class);
				lstDrivers->Items->Add("Class GUID:" + device->ClassGuid);
				lstDrivers->Items->Add("Instance ID: " + device->InstanceID);
				lstDrivers->Items->Add("------------------------------------------------------");

				for each (DriverInfo^ drvInfo in device->Drivers)
				{
					lstDrivers->Items->Add(drvInfo);
				}
				btnRemoveDevice->Enabled = true;
				btnRemoveDrivers->Enabled = false;
			}else{
				btnRemoveDevice->Enabled = false;
				btnRemoveDrivers->Enabled = false;
			}
		}

		void lstDrivers_IndexChanged(Object^ sender, EventArgs^ e)
		{
			DriverInfo^ drv = nullptr;
			if(lstDrivers->SelectedItem != nullptr)
			{
				drv = dynamic_cast<DriverInfo^>(lstDrivers->SelectedItem);
			}
			btnRemoveDrivers->Enabled = drv != nullptr;
		}

	public:
		UIDialog()
			: Form()
		{
			InitControls();

			InitDeviceList();
		}




	};
}