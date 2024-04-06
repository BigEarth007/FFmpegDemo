#pragma once
/*
* A buffer queue about AVPacket or AVFrame
*/


namespace aveditor
{
	class AVEDITOR_API CAVQueue
	{
	public:
		CAVQueue();
		~CAVQueue();

		void SetDataType(const EDataType& n_eDataType);
		const EDataType GetDataType() const;

		void Push(const EDataType& n_eDataType, void* n_Data);
		void Push(void* n_Data);

		int Pop(EDataType& n_eItemType, void*& n_Data, const int n_nTimeout);
		int Pop(void*& n_Data, const int n_nTimeout);

		int Front(EDataType& n_eItemType, void*& n_Data);
		int Front(void*& n_Data);

		size_t Size();
		bool Empty();
		void Clear();

		void Release();

	protected:
		Queue<void*>	m_qBuffer;
		EDataType		m_eItemType = EDataType::DT_None;
	};

}
