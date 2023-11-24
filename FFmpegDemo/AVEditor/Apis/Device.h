#pragma once


namespace aveditor
{
	struct AVEDITOR_API FDevice
	{
		FDevice() = default;
		~FDevice();

		// Get all video and audio device on you PC
		AVDeviceInfoList* Alloc(const char* n_szShortName, 
			const char* n_szDeviceName = nullptr);

		// Get name from device list by index
		std::string GetDeviceDescription(const unsigned int& n_nIndex);

		// Print all device information
		void DebugDevices();

		void Release();

		AVDeviceInfoList*	m_Context = nullptr;
		const AVInputFormat* m_InputFormat = nullptr;
	};
}
