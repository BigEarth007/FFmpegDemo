#pragma once


namespace aveditor
{
	struct AVEDITOR_API FPacket
	{
		FPacket() = default;
		~FPacket();

		// Alloc packet memory
		AVPacket* Alloc();
		// Release 
		void Release();

		// Copy packet. No need to alloc m_Packet
		AVPacket* Clone(const AVPacket* n_Packet);
		// Move packet ref
		void MoveRef(AVPacket* n_Packet);
		// Unref packet
		void UnRef();

		AVPacket* m_Packet = nullptr;
	};
}

