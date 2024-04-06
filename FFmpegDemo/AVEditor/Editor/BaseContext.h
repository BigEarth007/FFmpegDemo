#pragma once


namespace aveditor
{
	class CEditor;
	class AVEDITOR_API CBaseContext
	{
	public:
		CBaseContext(CEditor& n_Editor, const int n_nContextIndex);
		virtual ~CBaseContext();

		FFormatContext& GetContext();

		// Is this context valid
		const bool IsValid() const;

		const int GetContextIndex() const;

		virtual void Release();

		void SetContextHandle(IContextHandle* n_CtxHandle);

		void SetAVIOHandle(IAVIOHandle* n_AVIOHandle);

	protected:
		FFormatContext			m_Context;
		CEditor*				m_Editor = nullptr;

		IContextHandle*			m_CtxHandle = nullptr;
		IAVIOHandle*			m_AVIOHandle = nullptr;

		// Index of this format context
		int						m_nContextIndex = 0;
	};
}
