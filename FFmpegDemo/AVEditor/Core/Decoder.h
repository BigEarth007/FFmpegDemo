#pragma once


namespace aveditor
{
	class AVEDITOR_API CDecoder : public IStage
	{
	public:
		CDecoder(CEditor* n_Editor, EStreamType n_eStreamType);
		~CDecoder();

		virtual void Run();

		virtual void Release();

	protected:
		EStreamType m_eStreamType = EStreamType::ST_Size;
	};
}
