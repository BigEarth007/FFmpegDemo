#include "pch.h"
#include "Packet.h"


namespace aveditor
{
	FPacket::~FPacket()
	{
		Release();
	}

	AVPacket* FPacket::Alloc()
	{
		m_Packet = av_packet_alloc();
		ThrowExceptionExpr(!m_Packet, "Fail to create packet\n");

		return m_Packet;
	}

	void FPacket::Release()
	{
		if (m_Packet)
			av_packet_free(&m_Packet);
	}
	
	AVPacket* FPacket::Clone(const AVPacket* n_Packet)
	{
		m_Packet = av_packet_clone(n_Packet);
		return m_Packet;
	}

	void FPacket::MoveRef(AVPacket* n_Packet)
	{
		if (!m_Packet) Alloc();
		av_packet_move_ref(m_Packet, n_Packet);
	}

	void FPacket::UnRef()
	{
		if (m_Packet) av_packet_unref(m_Packet);
	}


}