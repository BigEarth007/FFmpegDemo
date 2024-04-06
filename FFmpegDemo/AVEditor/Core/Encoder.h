#pragma once


namespace aveditor
{
	class AVEDITOR_API CEncoder : public IStage
	{
	public:
		CEncoder(CEditor* n_Editor, EStreamType n_eStreamType);
		~CEncoder();

		virtual void Run();

		void Release();

	protected:
		EStreamType m_eStreamType = EStreamType::ST_Size;
	};

}
