#include "pch.h"
#include "Queue.h"

namespace aveditor
{
#ifndef STD_QUEUE
	void Queue::Push(void* n_Item)
	{
		FNode* Node = (FNode*)malloc(sizeof(FNode));
		Node->next = nullptr;
		Node->item = n_Item;

		if (!m_last)
		{
			m_head = Node;
			m_last = Node;
		}
		else
		{
			m_last->next = Node;
			m_last = Node;
		}

		m_nSize++;
	}

	int Queue::Pop(void*& n_Item, const int n_nTimeout)
	{
		if (!m_head) return -1;

		FNode* Node = m_head;
		n_Item = Node->item;

		if (!Node->next)
		{
			m_head = nullptr;
			m_last = nullptr;
		}
		else
			m_head = m_head->next;

		free(Node);
		Node = nullptr;
		m_nSize--;
	
		return 0;
	}

	int Queue::Front(void*& n_Item)
	{
		if (!m_head) return -1;
		n_Item = m_head->item;

		return 0;
	}

	void Queue::Clear(std::function<void(void*)> func /*= nullptr*/)
	{
		FNode* Node = m_head;
		while (Node)
		{
			if (func) func(Node->item);
			m_head = Node->next;

			free(Node);
			Node = nullptr;
			Node = m_head;
		}

		m_nSize = 0;
		m_head = nullptr;
		m_last = nullptr;
	}

#endif // !STD_QUEUE

}
