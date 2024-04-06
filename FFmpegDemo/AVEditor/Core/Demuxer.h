#pragma once


namespace aveditor
{
	class AVEDITOR_API CDemuxer : public IStage
	{
	public:
		CDemuxer(CEditor* n_Editor);
		~CDemuxer();

		// Release
		void Release();

		virtual void Run();

	protected:

	};
}
