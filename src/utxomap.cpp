
#include "chainparams.h"
#include "utxomap.hpp"
#include "validation.h"
#include "key_io.h"

mapTxToAddress mapreceiveAddressIndex;
mapTxFromAddress mapSendAddressIndex;

void RescanforAddressIndexd()
{
	// rescan the block and build index for address.




}

static void add_to_map(CTransactionRef tx)
{
	for (const CTxOut& txout : tx->vout)
	{
		CTxDestination dest;
		if (ExtractDestination(txout.scriptPubKey, dest))
		{
			auto addr =  EncodeDestination(dest);
			mapreceiveAddressIndex.insert({addr, tx});
		}
	}

	CCoinsViewCache view(pcoinsTip.get());

	for (const CTxIn& txin : tx->vin)
	{
		CTxDestination dest;
		Coin c;
		if (view.GetCoin(txin.prevout, c))
		{
			CTxDestination dest;

			if (ExtractDestination(c.out.scriptPubKey, dest))
			{
				auto addr =  EncodeDestination(dest);
				mapSendAddressIndex.insert({addr, tx});
			}
		}
		else
		{
			std::cout << "no coin in " << txin.prevout.hash.ToString() << std::endl;
			//mapSendAddressIndex
		}
	}
}

CBlockIndex* RescanforAddressIndex(CBlockIndex* pindexStart, CBlockIndex* pindexStop)
{
	const CChainParams& chainParams = Params();

	if (pindexStop)
	{
		assert(pindexStop->nHeight >= pindexStart->nHeight);
	}

	CBlockIndex* pindex = pindexStart;
	CBlockIndex* ret = nullptr;
	{
		while (pindex)
		{
			CBlock block;
			if (ReadBlockFromDisk(block, pindex, Params().GetConsensus())) {
				LOCK(cs_main);
				if (pindex && !chainActive.Contains(pindex)) {
					// Abort scan if current block is no longer active, to prevent
					// marking transactions as coming from the wrong block.
					ret = pindex;
					break;
				}

				ConnectForBlock(block);
			} else {
				ret = pindex;
			}
			if (pindex == pindexStop) {
				break;
			}
			{
				LOCK(cs_main);
				pindex = chainActive.Next(pindex);
			}
		}
	}
	return ret;
}

void ConnectForBlock(const CBlock& block)
{
	for (size_t posInBlock = 0; posInBlock < block.vtx.size(); ++posInBlock)
	{
		add_to_map(block.vtx[posInBlock]);//, pindex, posInBlock);
	}
}