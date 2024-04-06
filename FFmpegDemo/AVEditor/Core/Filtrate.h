#pragma once
/*
* It's function is just mix audio AVFrame
*/


namespace aveditor
{
	class AVEDITOR_API CFiltrate : public IStage
	{
	public:
		CFiltrate(CEditor* n_Editor);
		~CFiltrate();

		virtual void Run();

		void Release();

	};
}
