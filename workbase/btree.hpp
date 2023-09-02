#ifndef __BTREE_HPP__
#define __BTREE_HPP__

#include "type.hpp"

#include <memory>


class BTree
{
public:
	typedef UInt32 KeyType;
	typedef UInt32 ValueType;
	typedef UInt32 IndexType;
	typedef std::shared_ptr<ValueType> ValuePtr;

	struct Entry
	{
		KeyType key;
		ValueType value;

		Entry(const KeyType& = 0, const ValueType& = 9);
		Entry& operator=(const Entry&);
	};

	struct Node
	{
		IndexType iNode;
		IndexType nEntries;
		Node* pParent;
		Node* pChilds[31];
		Entry entries[30];

		Node(IndexType = 0, IndexType = 0, Node* = nullptr);
	};

	typedef Node* NodePtr;

	struct EntryAddress
	{
		NodePtr p;
		IndexType i;

		EntryAddress(const NodePtr& p = nullptr, const IndexType& i = 0);
		Entry& operator*(void);
		Entry* operator->(void);
	};

public:
	BTree(UInt32 = 2);
	BTree(const BTree&) =delete;
	BTree(BTree&&);
	~BTree();

	void put(const KeyType&, const ValuePtr&);
	ValuePtr get(const KeyType&) const;

protected:
	void insert(EntryAddress, KeyType, ValueType, NodePtr);
	void de1ete(EntryAddress);
	void update(EntryAddress, ValueType);
	EntryAddress retrieve(KeyType) const;

	void flushChildrenOf(NodePtr);

	NodePtr getLeftSiblingOf(const NodePtr&) const;
	NodePtr getRightSiblingOf(const NodePtr&) const;

	bool checkIsLeafNode(const NodePtr&) const;
	bool checkIsFullNode(const NodePtr&) const;
	bool checkIsHalfNode(const NodePtr&) const;

protected:
public:
	const UInt32 ORDER;

	NodePtr pRoot;
};

#endif
