
#pragma once

#include <unordered_map>

#include "primitives/transaction.h"
#include "primitives/block.h"
#include "chain.h"
#include "script/standard.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/functional.hpp>

struct AddTxMapItem
{
	std::string address;
	CTransactionRef tx;

	uint256 txHash() const {
		return tx->GetHash();
	}
};

struct uint256_hasher
{
	std::size_t operator()(const uint256& u) const
	{
		return u.GetCheapHash();
	}
};

typedef boost::multi_index_container<
	AddTxMapItem,
	boost::multi_index::indexed_by<
		boost::multi_index::hashed_non_unique<
			boost::multi_index::member<AddTxMapItem, std::string, &AddTxMapItem::address>
		>,

		boost::multi_index::hashed_non_unique<
			boost::multi_index::const_mem_fun<AddTxMapItem, uint256, &AddTxMapItem::txHash>, uint256_hasher
		>,
		boost::multi_index::hashed_unique<
			boost::multi_index::composite_key<
				boost::multi_index::member<AddTxMapItem, std::string, &AddTxMapItem::address>,
				boost::multi_index::const_mem_fun<AddTxMapItem, uint256, &AddTxMapItem::txHash>
			>,
			boost::multi_index::composite_key_hash<
				boost::hash<std::string>,
				uint256_hasher
			>
		>
	>
> AddressTxMap;

typedef std::unordered_multimap<std::string, CTransactionRef> mapTxToAddress;
typedef std::unordered_multimap<std::string, CTransactionRef> mapTxFromAddress;

extern mapTxToAddress mapreceiveAddressIndex;
extern mapTxFromAddress mapSendAddressIndex;

CBlockIndex* RescanforAddressIndex(CBlockIndex* pindexStart, CBlockIndex* pindexStop);

void ConnectForBlock(const CBlock& b);