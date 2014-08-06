#pragma once

namespace DriverHunter
{
	using namespace System;

	ref class DriverInfo
	{
	public:
		DriverInfo()
		{
		}

		property String^ InfFile;
		property String^ Description;
		property String^ Manufacturer;
		property String^ HardwareIds;
		property String^ Provider;
		property Version^ Version;
		
		virtual String^ ToString() override
		{
			return Description + " v" + Version + " (" + InfFile + ")";
		}
	};
}