#include "pch.h"
#include "Device.h"


namespace aveditor
{
	FDevice::~FDevice()
	{
		Release();
	}

	AVDeviceInfoList* FDevice::Alloc(const char* n_szShortName, 
		const char* n_szDeviceName /*= nullptr*/)
	{
		Release();

		m_InputFormat = FindInputFormat(n_szShortName);
		if (!m_InputFormat) return m_Context;

		AVDictionary* Options = nullptr;
		// List all the devices
		av_dict_set(&Options, "list_devices", "true", 0);

		int ret = avdevice_list_input_sources(m_InputFormat, n_szDeviceName,
			Options, &m_Context);

		//ThrowExceptionCodeExpr(ret < 0, ret, "Fail to list devices.");

		//DebugDevices();

		return m_Context;
	}

	std::string FDevice::GetDeviceDescription(const unsigned int& n_nIndex)
	{
		if (!m_Context || n_nIndex >= (unsigned int)m_Context->nb_devices)
			return std::string("");

		return std::string(m_Context->devices[n_nIndex]->device_description);
	}

	void FDevice::DebugDevices()
	{
		if (!m_Context) return;

		for (int i = 0;i < m_Context->nb_devices; i++)
		{
			AVDebug("%-16s%s\n", "Name:", m_Context->devices[i]->device_name);
			AVDebug("%-16s%s\n", "Description:", m_Context->devices[i]->device_description);

			for (int j = 0;j < m_Context->devices[i]->nb_media_types; j++)
			{
				const char* szMedaiType = av_get_media_type_string(
					m_Context->devices[i]->media_types[j]);
				AVDebug("%-16s%s\n", "Media Type:", szMedaiType);
			}

			AVDebug("--------------------------------------------------------\n\n");
		}
	}

	void FDevice::Release()
	{
		avdevice_free_list_devices(&m_Context);
		m_Context = nullptr;
	}

}
