#pragma once


namespace aveditor
{
	class AVEDITOR_API CMuxer : public IStage
	{
	public:
		CMuxer(CEditor* n_Editor);
		~CMuxer();

		virtual void Run();

	protected:
	};
}
