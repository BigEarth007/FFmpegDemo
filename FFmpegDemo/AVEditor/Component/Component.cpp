#include "pch.h"
#include "Component.h"

namespace aveditor
{

	IComponent::~IComponent()
	{
		//Release();
	}

	void IComponent::Init()
	{
		ThrowExceptionExpr(!m_Editor, "Call function SetEditor please!\n");

		m_OutputContext = &m_Editor->GetOutputContext()->GetContext();
	}

	void IComponent::Init(int n_nContextIndex)
	{
		Init();
	}

	int IComponent::Run()
	{
		return 0;
	}

	int IComponent::Run(EStreamType n_eStreamType)
	{
		return 0;
	}

	void IComponent::Release()
	{
		IAVIOHandle::Release();

		m_Callback = nullptr;

		m_bIsEnd = false;
	}

	int IComponent::WriteData(const EStreamType n_eStreamType, 
		void* n_Data, EDataType n_eType, int n_nIndex /*= 0*/)
	{
		int ret = 0;

		if (m_Callback)
			ret = m_Callback(n_eStreamType,
				n_Data, n_eType, n_nIndex);
		else if (m_AVIOHandle)
			ret = m_AVIOHandle->ReceiveData(n_eStreamType, 
				n_Data, n_eType, n_nIndex);
		else
		{
			switch (n_eType)
			{
			case aveditor::EDataType::DT_Packet:
				av_packet_free((AVPacket**)&n_Data);
				break;
			case aveditor::EDataType::DT_Frame:
				av_frame_free((AVFrame**)&n_Data);
				break;
			}
		}

		return ret;
	}

	void IComponent::SetEditor(CEditor* n_Editor)
	{
		m_Editor = n_Editor;
	}

	const CEditor* IComponent::GetEditor() const
	{
		return m_Editor;
	}

	void IComponent::SetCompID(const ECompID& n_eCompID)
	{
		m_eCompID = n_eCompID;
	}

	const ECompID IComponent::GetCompID() const
	{
		return m_eCompID;
	}

	void IComponent::SetEndFlag(const bool n_bEndFlag)
	{
		m_bIsEnd = n_bEndFlag;
	}

	const bool IComponent::GetEndFlag() const
	{
		return m_bIsEnd;
	}

	void IComponent::SetIOHandle(IAVIOHandle* n_Handle)
	{
		m_AVIOHandle = n_Handle;
	}

	void IComponent::SetCompCallback(CompCallback func)
	{
		m_Callback = func;
	}

	bool IComponent::LimitBufferSize()
	{
		return GetEndFlag();
	}

}