#include "btree.hpp"
#include "type.hpp"
#include "legacy-array.hpp"

#include <memory>

BTree::BTree(UInt32 order)
	: ORDER(order)
	, pRoot(new Node(0, 0, nullptr))
{
}


BTree::BTree(BTree&& obj)
	: ORDER(obj.ORDER)
	, pRoot(obj.pRoot)
{
	obj.pRoot = nullptr;
}


BTree::~BTree()
{
	//?
}


void BTree::put(const KeyType& key, const ValuePtr& value)
{
	auto&& addr = this->retrieve(key);
	if(addr->key != key)
	{
		this->insert(addr, key, *value, nullptr);
	}
	else if(value != nullptr)
	{
		this->update(addr, *value);
	}
	else
	{
		this->de1ete(addr);
	}
}


typename BTree::ValuePtr BTree::get(const KeyType& key) const
{
	auto&& attr = this->retrieve(key);
	if(attr->key != key)
	{
		return nullptr;
	}
		
	return std::make_shared<ValueType>(attr->value);
}


void BTree::insert(
	EntryAddress addr,
	KeyType key,
	ValueType value,
	NodePtr pChild
)
{
	//std::assert(nodeCheck());

	auto pNode = addr.p;
	auto iInsert = addr.i;

	// 当节点键值对数量未达到最大值（B树的阶）时，可直接插入无需分裂。	
	if(!this->checkIsFullNode(pNode))
	{
		// 子节点数组从第iInsert+1号开始后移一格，第iInsert+1号填入新插入的子节点pChild。
		CopyLegacyArray(
			pNode->pChilds + iInsert + 1,
			pNode->pChilds + iInsert + 2,
			pNode->nEntries - iInsert
		);
		pNode->pChilds[iInsert + 1] = pChild;
		/*if(!this->checkIsLeafNode(pNode))
		{
			pChild->iNode = iInsert + 1;
			pChild->pParent = pNode;
		}*/

		// 键值对数组从第iInsert号开始后移一格，第iInsert号填入将新插入的键值对kv。
		CopyLegacyArray(
			pNode->entries + iInsert,
			pNode->entries + iInsert + 1,
			pNode->nEntries - iInsert
		);
		//pNode->entries[iInsert] = kv;
		pNode->entries[iInsert].key = key;
		pNode->entries[iInsert].value = value;

		// 节点的键值对总数加一。
		pNode->nEntries += 1;

		this->flushChildrenOf(pNode);

		return;
	}

	// 当节点键值对数量达到最大值时，新建分裂节点。
	NodePtr pNew = new Node();

	// 重新分配键值对与子节点，
	// 插入后键值对总数为ORDER+1，
	// 原节点中键值对数目为ORDER - floor(ORDER / 2)，
	// 新节点中键值对数目为floor(ORDER / 2)，
	// 有一个键值对将插入父节点；
	// 将键值对数组分成两半[iStay,iMove)，[iMove,iEnd)，
	// 若插入位置在前一半中，则前一半的最后一个键值对将插入父节点，
	// 若插入位置在后一半中，则后一半的第一个键值对将插入父节点，
	// 若插入位置正好在中间，则直接将新键值对插入父节点。
	UInt32	nMove = this->ORDER / 2,
		nStay = this->ORDER - nMove;
	UInt32	iStay = 0,
		iMove = iStay + nStay,
		iEnd = iMove + nMove;

	// 根据插入位置iMove与移动位置iMove的关系，
	// 采取不同的数组复制操作。
	if(iInsert < iMove)
	{
		// 当插入位置iInsert在移动位置iMove之前时，
		// pChilds[iMove,iEnd+1)移动到新节点pNew中,
		// pChilds[iInsert+1,iMove)向后移动一格，
		// pChilds[iInsert+1]填入新插入的子节点pChild。
		// pChilds[iStay,iInsert+1)保持不变，
		CopyLegacyArray(
			pNode->pChilds + iMove,
			pNew->pChilds + 0,
			iEnd + 1 - iMove
		);
		CopyLegacyArray(
			pNode->pChilds + iInsert + 1,
			pNode->pChilds + iInsert + 2,
			iMove - (iInsert + 1)
		);
		pNode->pChilds[iInsert + 1] = pChild;

		// entries[iMove,iEnd)移动到新节点pNew中，
		// entries[iInsert,iMove)向后移动一格，
		// entries[iInsert]填入新插入的键值对kv。
		// entries[iStay,iInsert)保持不变，
		CopyLegacyArray(
			pNode->entries + iMove,
			pNew->entries + 0,
			nMove
		);
		CopyLegacyArray(
			pNode->entries + iInsert,
			pNode->entries + iInsert + 1,
			iMove - iInsert
		);
		//pNode->entries[iInsert] = kv;
		pNode->entries[iInsert].key = key;
		pNode->entries[iInsert].value = value;
	}
	else if(iInsert > iMove)
	{
		// 当插入位置iInsert在移动位置iMove之后时，
		// pChilds[iMove+1,iInsert+1)移动到新节点pNew中，
		// 新插入的子节点填入新节点pNew中，
		// pChilds[iInsert+1,iEnd+1)移动到新节点pNew中,
		// pChilds[iStay,iMove+1)保持不变。
		CopyLegacyArray(
			pNode->pChilds + iMove + 1,
			pNew->pChilds + 0,
			iInsert - iMove
		);
		pNew->pChilds[iInsert - iMove] = pChild;
		CopyLegacyArray(
			pNode->pChilds + iInsert + 1,
			pNew->pChilds + (iInsert - iMove) + 1,
			iEnd - iInsert
		);

		// entries[iMove+1,iInsert)移动到新节点pNew中，
		// 新插入的键值对kv填入新节点pNew中，
		// entries[iInsert,iEnd)移动到新节点pNew中，
		// entries[iStay,iMove)保持不变。
		CopyLegacyArray(
			pNode->entries + iMove + 1,
			pNew->entries + 0,
			iInsert - (iMove + 1)
		);
		//pNew->entries[iInsert - (iMove + 1)] = kv;
		pNew->entries[iInsert - (iMove + 1)].key = key;
		pNew->entries[iInsert - (iMove + 1)].value = value;
		CopyLegacyArray(
			pNode->entries + iInsert,
			pNew->entries + iInsert - iMove,
			iEnd - iInsert
		);
	}
	else
	{
		// 当插入位置iInsert与移动位置iMove重合时，
		// pChilds[iMove,iEnd+1)移动到新节点pNew中，
		// pChilds[iMove]填入新插入的子节点pChild，
		// pChilds[iStay,iMove)保持不变。
		CopyLegacyArray(
			pNode->pChilds + iMove,
			pNew->pChilds + 0,
			iEnd + 1 - iMove
		);
		pNode->pChilds[iMove] = pChild;

		// entries[iMove,iEnd)移动到新节点pNew中，
		// entries[iMove]填入新插入的键值对kv，
		// entries[iStay,iMove)保持不变。
		CopyLegacyArray(
			pNode->entries + iMove,
			pNew->entries + 0,
			iEnd - iMove
		);
		//pNode->entries[iMove] = kv;
		pNode->entries[iMove].key = key;
		pNode->entries[iMove].value = value;
	}

	// 修改原节点与新节点键值对数量。
	pNode->nEntries = nStay;
	pNew->nEntries = nMove;

	// 修改原节点与新节点所有子节点的父子关系。
	this->flushChildrenOf(pNode);
	this->flushChildrenOf(pNew);

	// 若当前节点为根节点，
	// 则新建一个根节点，
	// 原节点和新节点均作为子节点。
	if(pNode->pParent == nullptr)
	{
		pNode->iNode = 0;
		pNode->pParent = this->pRoot = new Node(0, 0, nullptr);
		pNode->pParent->pChilds[0] = pNode;
	}

	// 将节点分裂后的中间值递归插入到父节点中。
	return this->insert(
		EntryAddress(
			pNode->pParent,
			pNode->iNode
		),
		pNode->entries[iMove].key,
		pNode->entries[iMove].value,
		pNew
	);
}


void BTree::de1ete(EntryAddress addr)
{
	//std::assert(pNode != nullptr && iDelete < pNode->nEntries)

	auto pNode = addr.p;
	auto iDelete = addr.i;

	NodePtr	pParent = pNode->pParent,
		pLeft = this->getLeftSiblingOf(pNode),
		pRight = this->getRightSiblingOf(pNode);

	if(pNode->pChilds[iDelete + 1] != nullptr)
	{
		// 以删除位置右子树的最左侧节点作为替代节点。
		NodePtr pExange;
		for(	pExange = pNode->pChilds[iDelete + 1];
			pExange->pChilds[0] != nullptr;
			pExange = pExange->pChilds[0]
		);
	
		// 使用替代节点最左侧的键值对覆盖待删除的键值对。
		pNode->entries[iDelete] = pExange->entries[0];

		// 递归删除替代节点的最左侧键值对。
		this->de1ete(
			EntryAddress(
				pExange,
				0
			)
		);
	}
	else if(!this->checkIsHalfNode(pNode) || pParent == nullptr)
	{
		// 无需重新分配。
		CopyLegacyArray(
			pNode->pChilds + iDelete + 2,
			pNode->pChilds + iDelete + 1,
			(pNode->nEntries + 1) - (iDelete + 2)
		);

		CopyLegacyArray(
			pNode->entries + iDelete + 1,
			pNode->entries + iDelete,
			pNode->nEntries - (iDelete + 1)
		);

		pNode->nEntries -= 1;
	
		this->flushChildrenOf(pNode);

		if(pNode->nEntries == 0 && !this->checkIsLeafNode(pNode))
		{
			this->pRoot = pNode->pChilds[0];
			this->pRoot->pParent = nullptr;
			delete pNode;
		}
	}
	else if(pRight != nullptr)
	{
		if(!this->checkIsHalfNode(pRight))
		{
			CopyLegacyArray(
				pNode->pChilds + iDelete + 2,
				pNode->pChilds + iDelete + 1,
				(pNode->nEntries + 1) - (iDelete + 2)
			);
			pNode->pChilds[pNode->nEntries] = pRight->pChilds[0];

			CopyLegacyArray(
				pNode->entries + iDelete + 1,
				pNode->entries + iDelete,
				pNode->nEntries - (iDelete + 1)
			);
			pNode->entries[pNode->nEntries - 1] = pParent->entries[pNode->iNode];
			pParent->entries[pNode->iNode] = pRight->entries[0];
	
			this->flushChildrenOf(pNode);

			pRight->pChilds[0] = pRight->pChilds[1];
			pRight->pChilds[1] = nullptr;
			this->de1ete(
				EntryAddress(
					pRight,
					0
				)
			);
		}
		else
		{
			CopyLegacyArray(
				pNode->pChilds + iDelete + 2,
				pNode->pChilds + iDelete + 1,
				(pNode->nEntries + 1) - (iDelete + 2)
			);

			CopyLegacyArray(
				pRight->pChilds + 0,
				pNode->pChilds + pNode->nEntries,
				pRight->nEntries + 1
			);

			CopyLegacyArray(
				pNode->entries + iDelete + 1,
				pNode->entries + iDelete,
				pNode->nEntries - (iDelete + 1)
			);

			pNode->entries[pNode->nEntries - 1] = pParent->entries[pNode->iNode];

			CopyLegacyArray(
				pRight->entries + 0,
				pNode->entries + pNode->nEntries,
				pRight->nEntries
			);

			pNode->nEntries += pRight->nEntries;

			this->flushChildrenOf(pNode);

			pParent->pChilds[pNode->iNode + 1] = nullptr;
			this->de1ete(
				EntryAddress(
					pParent,
					pNode->iNode
				)
			);
		}
	}
	else
	{
		if(!this->checkIsHalfNode(pLeft))
		{
			CopyLegacyArray(
				pNode->pChilds + 0,
				pNode->pChilds + 1,
				iDelete + 1
			);
			pNode->pChilds[0] = pLeft->pChilds[pLeft->nEntries];

			CopyLegacyArray(
				pNode->entries + 0,
				pNode->entries + 1,
				iDelete
			);
			pNode->entries[0] = pParent->entries[pNode->iNode - 1];
			pParent->entries[pNode->iNode - 1] = pLeft->entries[pLeft->nEntries - 1];

			this->flushChildrenOf(pNode);

			pLeft->pChilds[pLeft->nEntries] = nullptr;
			this->de1ete(
				EntryAddress(
					pLeft,
					pLeft->nEntries - 1
				)
			);
		}
		else
		{
			CopyLegacyArray(
				pNode->pChilds + 0,
				pLeft->pChilds + pLeft->nEntries + 1,
				iDelete + 1
			);
			CopyLegacyArray(
				pNode->pChilds + iDelete + 2,
				pLeft->pChilds + pLeft->nEntries + iDelete + 1,
				(pNode->nEntries + 1) - (iDelete + 2)
			);

			pLeft->entries[pLeft->nEntries] = pLeft->pParent->entries[pLeft->iNode];

			CopyLegacyArray(
				pNode->entries + 0,
				pLeft->entries + pLeft->nEntries + 1,
				iDelete
			);
			CopyLegacyArray(
				pNode->entries + iDelete + 1,
				pLeft->entries + pLeft->nEntries + 1 + iDelete,
				pNode->nEntries - (iDelete + 1)
			);

			pLeft->nEntries += pNode->nEntries;

			this->flushChildrenOf(pLeft);

			pParent->pChilds[pNode->iNode] = nullptr;
			this->de1ete(
				EntryAddress(
					pParent,
					pNode->iNode - 1
				)
			);
		}
	}
}


void BTree::update(EntryAddress addr, ValueType value)
{
	addr->value = value;

	return;
}


typename BTree::EntryAddress BTree::retrieve(KeyType key) const
{
	EntryAddress addr;
	NodePtr pTemp = nullptr;
	for(addr.p = this->pRoot; addr.p != nullptr; pTemp = addr.p, addr.p = addr.p->pChilds[addr.i])
	{
		for(addr.i = 0; addr.i < addr.p->nEntries; ++addr.i)
		{
			if(addr->key == key)
			{
				return addr;
			}
			else if(addr->key > key)
			{
				break;
			}
			else
			{
				continue;
			}
		}
	}
	addr.p = pTemp;
	
	return addr;
}


void BTree::flushChildrenOf(NodePtr pNode)
{
	if(!this->checkIsLeafNode(pNode))
	{
		for(int i = 0; i < pNode->nEntries + 1; ++i)
		{
			pNode->pChilds[i]->iNode = i;
			pNode->pChilds[i]->pParent = pNode;
		}
	}
}


BTree::NodePtr BTree::getLeftSiblingOf(const NodePtr& pNode) const
{
	//std::assert(pNode != nullptr)
		
	return	pNode->pParent != nullptr && pNode->iNode > 0 ?
		pNode->pParent->pChilds[pNode->iNode - 1] :
		nullptr
	;
}


BTree::NodePtr BTree::getRightSiblingOf(const NodePtr& pNode) const
{
	//std::assert(pNode != nullptr)
		
	return	pNode->pParent != nullptr && pNode->iNode < pNode->pParent->nEntries ?
		pNode->pParent->pChilds[pNode->iNode + 1] :
		nullptr
	;
}


bool BTree::checkIsLeafNode(const NodePtr& pNode) const
{
	//std::assert(pNode != nullptr)
		
	return	pNode->pChilds[0] == nullptr;
}


bool BTree::checkIsFullNode(const NodePtr& pNode) const
{
	//std::assert(pNode != nullptr)
		
	return	pNode->nEntries == this->ORDER;
}


bool BTree::checkIsHalfNode(const NodePtr& pNode) const
{
	//std::assert(pNode != nullptr)
		
	return	pNode->nEntries == this->ORDER / 2;
}


BTree::Entry::Entry(const BTree::KeyType& key, const BTree::ValueType& value)
	: key(key)
	, value(value)
{
}


typename BTree::Entry& BTree::Entry::operator=(const Entry& obj)
{
	this->key = obj.key;
	this->value = obj.value;

	return *this;
}


BTree::Node::Node(BTree::IndexType iNode, BTree::IndexType nEntries, NodePtr pParent)
	: iNode(iNode)
	, nEntries(nEntries)
	, pParent(pParent)
{
}


BTree::EntryAddress::EntryAddress(const BTree::NodePtr& p, const BTree::IndexType& i)
	: p(p)
	, i(i)
{
}


typename BTree::Entry& BTree::EntryAddress::operator*(void)
{
	//std::assert(this->p != nullptr && this->i < pNode->nEntries);
	
	return this->p->entries[this->i];
	
}


typename BTree::Entry* BTree::EntryAddress::operator->(void)
{
	//std::assert(this->p != nullptr && this->i < pNode->nEntries);
	
	return &(this->p->entries[this->i]);
	
}
