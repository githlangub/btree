#include "../workbase/btree.hpp"

#include <iostream>
#include <queue>


void print(BTree::NodePtr pNode)
{
	std::cout << "========================================" << '\n';

	std::cout << "\tpNode = " << pNode << '\n';
	if(pNode == nullptr) return;

	std::cout << "\tiNode = " << pNode->iNode << '\n';
	std::cout << "\tnEntries = " << pNode->nEntries << '\n';
	std::cout << "\tpParent = " << pNode->pParent << '\n';

	std::cout << "\tpChilds = " << '\n';
	for(int i = 0; i < pNode->nEntries + 1; ++i)
	{
		std::cout << "\t\t" << pNode->pChilds[i] << '\n';
	}

	std::cout << "\tentries = " << '\n';
	for(int i = 0; i < pNode->nEntries; ++i)
	{
		std::cout << "\t\t" << pNode->entries[i].key << ' ' << pNode->entries[i].value << '\n';
	}

	std::cout << "========================================" << '\n';
}


void levelOrder(BTree::NodePtr pRoot)
{
	int level = 0;
	for(std::deque<BTree::NodePtr> q(1, pRoot); !q.empty();)
	{
		std::cout << "Level " << level++ << '\n';

		auto count = q.size();
		for(int i = 0; i < count; ++i)
		{
			auto p = q.front();
			q.pop_front();

			print(p);
			std::cout << std::endl;

			for(int i = 0; i < p->nEntries + 1; ++i)
			{
				if(p->pChilds[i] != nullptr)
				{
					q.push_back(p->pChilds[i]);
				}
			}
		}
	}
}


void midOrder(BTree::NodePtr pRoot)
{
	if(pRoot == nullptr)
	{
		return;
	}

	for(int i = 0; i < pRoot->nEntries; ++i)
	{
		midOrder(pRoot->pChilds[i]);
		std::cout << pRoot->entries[i].key << ' ' << pRoot->entries[i].value << '\n';
	}
	midOrder(pRoot->pChilds[pRoot->nEntries]);
}


bool checkBTree(BTree::NodePtr pRoot)
{
	if(pRoot->nEntries < 1 || pRoot->nEntries > 3)
	{
		std::cout << "Invalid key: " << pRoot->entries[0].key << std::endl;
		return false;
	}

	for(int i = 0; i < pRoot->nEntries + 1; ++i)
	{
		if(pRoot->pChilds[i] != nullptr)
		{
			if(pRoot->pChilds[i]->pParent != pRoot || pRoot->pChilds[i]->iNode != i)
			{
				std::cout << "Invalid key: " << pRoot->pChilds[i]->entries[0].key << std::endl;
				return false;
			}
		}
		if(pRoot->pChilds[0] == nullptr && pRoot->pChilds[i] != nullptr)
		{
			std::cout << "Invalid key: " << pRoot->entries[0].key << std::endl;
			return false;
		}
		if(pRoot->pChilds[0] != nullptr && pRoot->pChilds[i] == nullptr)
		{
			std::cout << "Invalid key: " << pRoot->entries[0].key << std::endl;
			return false;
		}
	}

	return true;
}


int main()
{
	BTree bt(3);
	auto v = new BTree::ValueType(0);
	auto pV = BTree::ValuePtr(v);
	for(int i = 50; i >= 1; --i)
	{
		*pV = i * 10 + i;
		bt.put(i, pV);
		if(!checkBTree(bt.pRoot))
		{
			std::cout << "Insert " << i << std::endl;
			break;
		}
	}
	for(int i = 151; i <= 200; ++i)
	{
		*pV = i * 10 + i;
		bt.put(i, pV);
		if(!checkBTree(bt.pRoot))
		{
			std::cout << "Insert " << i << std::endl;
			break;
		}
	}
	for(int i = 150; i >= 51; --i)
	{
		*pV = i * 10 + i;
		bt.put(i, pV);
		if(!checkBTree(bt.pRoot))
		{
			std::cout << "Insert " << i << std::endl;
			break;
		}
	}
	for(int i = 12; i < 200; ++i)
	{
		bt.put(i, NULL);
		if(!checkBTree(bt.pRoot))
		{
			std::cout << "Delete " << i << std::endl;
			break;
		}
	}
		
	*pV = 1024;
	bt.put(200, pV);
	std::cout << *bt.get(200) << std::endl;

	//levelOrder(bt.pRoot);
	
	midOrder(bt.pRoot);
	
	std::cout << std::endl;

	return 0;
}
