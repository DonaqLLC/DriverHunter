#pragma once
#include "DriverInfo.h"

namespace DriverHunter
{
	using namespace System;
	using namespace System::Collections::Generic;

	ref class DeviceInfo
	{
	public:
		DeviceInfo(String^ instanceID)
		{
			InstanceID = instanceID;
			Drivers = gcnew List<DriverHunter::DriverInfo^>();
		}

		property String^ InstanceID;
		property String^ Description;
		property String^ Mfg;
		property String^ Class;
		property String^ ClassGuid;
		property UInt32 DeviceType;

		property List<DriverInfo^>^ Drivers;

		virtual String^ ToString() override
		{
			return Description;
		}
	};
}