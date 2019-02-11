// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <coins.h>
#include <consensus/consensus.h>
#include <consensus/merkle.h>
#include <consensus/tx_verify.h>
#include <consensus/validation.h>
#include <validation.h>
#include <miner.h>
#include <policy/policy.h>
#include <pubkey.h>
#include <script/standard.h>
#include <txmempool.h>
#include <uint256.h>
#include <util.h>
#include <utilstrencodings.h>

#include <test/test_bitcoin.h>

#include <memory>
#include <crypto/equihash.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(miner_tests, TestingSetup)

// BOOST_CHECK_EXCEPTION predicates to check the specific validation error
class HasReason {
public:
    HasReason(const std::string& reason) : m_reason(reason) {}
    bool operator() (const std::runtime_error& e) const {
        return std::string(e.what()).find(m_reason) != std::string::npos;
    };
private:
    const std::string m_reason;
};

static CFeeRate blockMinFeeRate = CFeeRate(DEFAULT_BLOCK_MIN_TX_FEE);

static BlockAssembler AssemblerForTest(const CChainParams& params) {
    BlockAssembler::Options options;

    options.nBlockMaxWeight = MAX_BLOCK_WEIGHT;
    options.blockMinFeeRate = blockMinFeeRate;
    return BlockAssembler(params, options);
}

// todo 下面的数据都是临时的，等mainnet的参数确定后要重新计算
static
struct {
    const char *nonce_hex;
    const char *solution_hex;
} blockinfo[] = {
    {"000000000000000000000000000000000000000000000000000000000000000d", "004e8dfa09ca8fbf2fee30a9ba552bd438dd6eb20b01c186a994c7fd2161ae203e2063b8d53efa3afde6033f202f8b21c8afda6a33732de8ef75a82b0fe80022714a6b1e657c0b943024776dab1495f5ec1c4fd0086898d1fda17ab370b174eb8f48c8f2bc9078dd583a052ff6a97bb335ec9303a3e3f6da4518f4ac4ad81ae24acfd0d8756b849f92c01361a45add757d113347c287ab3515c8abcdef753b0371045e3ce875ca0c009e4f7da6d99739f008d73122f41c7a8dfad8027f07861a848c4b9db6c2ad33e04ed7fafd20add6017216bdd754ba072948697cc51334487b9164f7acd3ee46b722e1d39eaeb92a429522acd05a4ddcd03baf080f6fa3664cd3ca79a953142aa4760ef978bceec7b15d01c5a00d76907fc23b85d19b45be5259b5b5dfab18e7c9f9914cdc1b62a5d4916de72dd5b1ffdfbf4a5692e33a9d1bfa6513bb36e3bce76245e51fb11a8c04ae67f9e1c7f590bd88d0c8d7957378fbaca90a531649f3230507df97497b379d08e52ece6201f61aba0caacdf3e609a711b58b311460f16e385194a8ade5114e441a44a651a7a9f621255edba58d9719f0167326353bb3828f1f8fdf62ba611b6e33abd6421ed90932e65d0a51a8f3ff8d3543a42ccd5212543af2c86828a335690dda26736a32e614d0ec90670ec21d0d15423c7d1f789a48a6fb5576a17f3a6b79ec415b29850cc0c3ed78259cdb9905585426f738faa23fd5dfd6237182e2b86316492845a3dd2d4ab08b50739aec021e2b5189654e5ebba78ab3ad2dc268fcf768350b6124741f5eeada75412457e6adbfb5e86b4bde3f689b0ce9766f112b7f51827081d9b6a60ef9058a93cf83136f44c7558b6e72c7fe9278d8c26788a3d9b85a9825a9b2821da03f3f787319a0d67c75f27d84fc046034ddfeee2d732fb3f25767c945676e6f64d71b8a4701196def428373937a16782159567ff687ab37db2f1532454016e8fea9e2a943a1f6a0c5fd9b6c9aa36d2343c28bb55a54e1ec36b25dfbfd3114fc557adfc0272e2e46aa8ee76e8790384fa9655b3a3d9f9f573f0d307d104103adc22bbcb42d7450d8c2f5f95d0f050e08890648e281135d67f21306a74f68ec5cb88b8c15791e9f2948c880ee68f3f98d7f01c12e08bb7e4a2708edde338b560eef7892861379bc10c9197a3a2b0f8cd97f9e959257db829298fba7202a88ffb55c1312a0959f3e614d73caad71a09c2b8dded4c01e02ad3a45372ff3da26e9eae33951f55b03aa8407165435479bc71ec1a09a891c2fb697933d369ddf762f086c175db4f636a3afbbe9f4d25a71c14c8992a03b7d213a603695cfdc9f054e86659351c1fc899d2294433a22af6411c94f7cf4d4fb95c332cbdd4618b5f5dbb42e35527da50f964abc366465b5e6f70e9b4eb697b705a0dec234ee514bbcddf1aea1542d128801948740485032629b75ce97c074f566f5545f25ff48d5c56714e237883f474fe9da5003ab1d672305239778bc3d155d717ad5d21a72a2fbb29b7eb8059ccdb831103b0c4137acac463e45604b96a7e9e6c9a5fa107f30df20cf14c5e61a96a5a2e3176a515b41d25509fd0ac71f1f522789aec9f189b2252af9337c462bfef56c6142991afd1735dabdf23aaa99cb5b2cc3a589dde0000a7982c5ea53597fb44d0298a4ac9959a9731b8d1b0ecc90a7dc4bcf42a705b5bf77405e95d4e35f82410cd6bd3e36e57b7dc39c62e87b785729d406168a331ea6ca74e88fc5f70629146ed9645b3f2e5f7b3276105a82cec00a790ec6ea134574a5f4ed53dc6bec4a39cf0407e1502b16d2975480ed65be9593419c487f3fcc937436941ff7b0f554faa6f9b241e66416578148bcf427d29385de9cf12ab9c77db6b714b4ba44c6"}
};

CBlockIndex CreateBlockIndex(int nHeight)
{
    CBlockIndex index;
    index.nHeight = nHeight;
    index.pprev = chainActive.Tip();
    return index;
}

bool TestSequenceLocks(const CTransaction &tx, int flags)
{
    LOCK(mempool.cs);
    return CheckSequenceLocks(tx, flags);
}

// Test suite for ancestor feerate transaction selection.
// Implemented as an additional function, rather than a separate test case,
// to allow reusing the blockchain created in CreateNewBlock_validity.
void TestPackageSelection(const CChainParams& chainparams, CScript scriptPubKey, std::vector<CTransactionRef>& txFirst)
{
    // Test the ancestor feerate transaction selection.
    TestMemPoolEntryHelper entry;

    // Test that a medium fee transaction will be selected after a higher fee
    // rate package with a low fee rate parent.
    CMutableTransaction tx;
    tx.vin.resize(1);
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    tx.vin[0].prevout.n = 0;
    tx.vout.resize(1);
    tx.vout[0].nValue = 5000000000LL - 1000;
    // This tx has a low fee: 1000 satoshis
    uint256 hashParentTx = tx.GetHash(); // save this txid for later use
    mempool.addUnchecked(hashParentTx, entry.Fee(1000).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));

    // This tx has a medium fee: 10000 satoshis
    tx.vin[0].prevout.hash = txFirst[1]->GetHash();
    tx.vout[0].nValue = 5000000000LL - 10000;
    uint256 hashMediumFeeTx = tx.GetHash();
    mempool.addUnchecked(hashMediumFeeTx, entry.Fee(10000).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));

    // This tx has a high fee, but depends on the first transaction
    tx.vin[0].prevout.hash = hashParentTx;
    tx.vout[0].nValue = 5000000000LL - 1000 - 50000; // 50k satoshi fee
    uint256 hashHighFeeTx = tx.GetHash();
    mempool.addUnchecked(hashHighFeeTx, entry.Fee(50000).Time(GetTime()).SpendsCoinbase(false).FromTx(tx));

    std::unique_ptr<CBlockTemplate> pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey);
    BOOST_CHECK(pblocktemplate->block.vtx[1]->GetHash() == hashParentTx);
    BOOST_CHECK(pblocktemplate->block.vtx[2]->GetHash() == hashHighFeeTx);
    BOOST_CHECK(pblocktemplate->block.vtx[3]->GetHash() == hashMediumFeeTx);

    // Test that a package below the block min tx fee doesn't get included
    tx.vin[0].prevout.hash = hashHighFeeTx;
    tx.vout[0].nValue = 5000000000LL - 1000 - 50000; // 0 fee
    uint256 hashFreeTx = tx.GetHash();
    mempool.addUnchecked(hashFreeTx, entry.Fee(0).FromTx(tx));
    size_t freeTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);

    // Calculate a fee on child transaction that will put the package just
    // below the block min tx fee (assuming 1 child tx of the same size).
    CAmount feeToUse = blockMinFeeRate.GetFee(2*freeTxSize) - 1;

    tx.vin[0].prevout.hash = hashFreeTx;
    tx.vout[0].nValue = 5000000000LL - 1000 - 50000 - feeToUse;
    uint256 hashLowFeeTx = tx.GetHash();
    mempool.addUnchecked(hashLowFeeTx, entry.Fee(feeToUse).FromTx(tx));
    pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey);
    // Verify that the free tx and the low fee tx didn't get selected
    for (size_t i=0; i<pblocktemplate->block.vtx.size(); ++i) {
        BOOST_CHECK(pblocktemplate->block.vtx[i]->GetHash() != hashFreeTx);
        BOOST_CHECK(pblocktemplate->block.vtx[i]->GetHash() != hashLowFeeTx);
    }

    // Test that packages above the min relay fee do get included, even if one
    // of the transactions is below the min relay fee
    // Remove the low fee transaction and replace with a higher fee transaction
    mempool.removeRecursive(tx);
    tx.vout[0].nValue -= 2; // Now we should be just over the min relay fee
    hashLowFeeTx = tx.GetHash();
    mempool.addUnchecked(hashLowFeeTx, entry.Fee(feeToUse+2).FromTx(tx));
    pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey);
    BOOST_CHECK(pblocktemplate->block.vtx[4]->GetHash() == hashFreeTx);
    BOOST_CHECK(pblocktemplate->block.vtx[5]->GetHash() == hashLowFeeTx);

    // Test that transaction selection properly updates ancestor fee
    // calculations as ancestor transactions get included in a block.
    // Add a 0-fee transaction that has 2 outputs.
    tx.vin[0].prevout.hash = txFirst[2]->GetHash();
    tx.vout.resize(2);
    tx.vout[0].nValue = 5000000000LL - 100000000;
    tx.vout[1].nValue = 100000000; // 1BTC output
    uint256 hashFreeTx2 = tx.GetHash();
    mempool.addUnchecked(hashFreeTx2, entry.Fee(0).SpendsCoinbase(true).FromTx(tx));

    // This tx can't be mined by itself
    tx.vin[0].prevout.hash = hashFreeTx2;
    tx.vout.resize(1);
    feeToUse = blockMinFeeRate.GetFee(freeTxSize);
    tx.vout[0].nValue = 5000000000LL - 100000000 - feeToUse;
    uint256 hashLowFeeTx2 = tx.GetHash();
    mempool.addUnchecked(hashLowFeeTx2, entry.Fee(feeToUse).SpendsCoinbase(false).FromTx(tx));
    pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey);

    // Verify that this tx isn't selected.
    for (size_t i=0; i<pblocktemplate->block.vtx.size(); ++i) {
        BOOST_CHECK(pblocktemplate->block.vtx[i]->GetHash() != hashFreeTx2);
        BOOST_CHECK(pblocktemplate->block.vtx[i]->GetHash() != hashLowFeeTx2);
    }

    // This tx will be mineable, and should cause hashLowFeeTx2 to be selected
    // as well.
    tx.vin[0].prevout.n = 1;
    tx.vout[0].nValue = 100000000 - 10000; // 10k satoshi fee
    mempool.addUnchecked(tx.GetHash(), entry.Fee(10000).FromTx(tx));
    pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey);
    BOOST_CHECK(pblocktemplate->block.vtx[8]->GetHash() == hashLowFeeTx2);
}

// NOTE: These tests rely on CreateNewBlock doing its own self-validation!
BOOST_AUTO_TEST_CASE(CreateNewBlock_validity)
{
    // Note that by default, these tests run with size accounting enabled.
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    const CChainParams& chainparams = *chainParams;
    CScript scriptPubKey = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    std::unique_ptr<CBlockTemplate> pblocktemplate;
    CMutableTransaction tx,tx2;
    CScript script;
    uint256 hash;
    TestMemPoolEntryHelper entry;
    entry.nFee = 11;
    entry.nHeight = 11;

    fCheckpointsEnabled = false;

    // Simple block creation, nothing special yet:
    BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));

    // We can't make transactions until we have inputs
    // Therefore, load 100 blocks :)
    int baseheight = 0;
    std::vector<CTransactionRef> txFirst;
    for (unsigned int i = 0; i < sizeof(blockinfo)/sizeof(*blockinfo); ++i)
    {
        CBlock *pblock = &pblocktemplate->block; // pointer for convenience
        {
            LOCK(cs_main);
            pblock->nTime = chainActive.Tip()->GetMedianTimePast()+1;
            CMutableTransaction txCoinbase(*pblock->vtx[0]);
            txCoinbase.nVersion = 1;
            pblock->vtx[0] = MakeTransactionRef(std::move(txCoinbase));
            if (txFirst.size() == 0)
                baseheight = chainActive.Height();
            if (txFirst.size() < 4)
                txFirst.push_back(pblock->vtx[0]);
            pblock->hashMerkleRoot = BlockMerkleRoot(*pblock);
            pblock->nNonce = uint256S(blockinfo[i].nonce_hex);
            pblock->nSolution = ParseHex(blockinfo[i].solution_hex);
        }

//         unsigned int n = chainparams.EquihashN();
//         unsigned int k = chainparams.EquihashK();
//         // Hash state
//         crypto_generichash_blake2b_state state;
//         EhInitialiseState(n, k, state);
//         // I = the block header minus nonce and solution.
//         CEquihashInput I{*pblock};
//         // I||V
//         CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
//         ss << I;
//         // H(I||V||...
//         crypto_generichash_blake2b_update(&state, (unsigned char*)&ss[0], ss.size());
//         while (true) {
//             // Yes, there is a chance every nonce could fail to satisfy the -regtest
//             // target -- 1 in 2^(2^256). That ain't gonna happen
//             pblock->nNonce = ArithToUint256(UintToArith256(pblock->nNonce) + 1);
//             printf("nonce = %s\n", pblock->nNonce.GetHex().c_str());
//             // H(I||V||...
//             crypto_generichash_blake2b_state curr_state;
//             curr_state = state;
//             crypto_generichash_blake2b_update(&curr_state,
//                                               pblock->nNonce.begin(),
//                                               pblock->nNonce.size());

//             // (x_1, x_2, ...) = A(I, V, n, k)
//             std::function<bool(std::vector<unsigned char>)> validBlock =
//                     [&pblock](std::vector<unsigned char> soln) {
//                 printf("soln = %s\n", HexStr(soln).c_str());        
//                 pblock->nSolution = soln;
//                 return CheckProofOfWork(pblock->GetHash(), pblock->nBits, Params().GetConsensus());
//             };
//             bool found = EhBasicSolveUncancellable(n, k, curr_state, validBlock);
//             if (found) {
//                 goto endloop;
//             }
//         }
// endloop:
//         printf("pblock nonce = %s\n", pblock->nNonce.GetHex().c_str());
//         printf("pblock soln = %s\n", HexStr(pblock->nSolution).c_str());

        std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(*pblock);
        BOOST_CHECK(ProcessNewBlock(chainparams, shared_pblock, true, nullptr));
        pblock->hashPrevBlock = pblock->GetHash();
    }

    LOCK(cs_main);

    // Just to make sure we can still make simple blocks
    BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));

    const CAmount BLOCKSUBSIDY = 50*COIN;
    const CAmount LOWFEE = CENT;
    const CAmount HIGHFEE = COIN;
    const CAmount HIGHERFEE = 4*COIN;

    // block sigops > limit: 1000 CHECKMULTISIG + 1
    tx.vin.resize(1);
    // NOTE: OP_NOP is used to force 20 SigOps for the CHECKMULTISIG
    tx.vin[0].scriptSig = CScript() << OP_0 << OP_0 << OP_0 << OP_NOP << OP_CHECKMULTISIG << OP_1;
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    tx.vin[0].prevout.n = 0;
    tx.vout.resize(1);
    tx.vout[0].nValue = BLOCKSUBSIDY;
    for (unsigned int i = 0; i < 1001; ++i)
    {
        tx.vout[0].nValue -= LOWFEE;
        hash = tx.GetHash();
        bool spendsCoinbase = i == 0; // only first tx spends coinbase
        // If we don't set the # of sig ops in the CTxMemPoolEntry, template creation fails
        mempool.addUnchecked(hash, entry.Fee(LOWFEE).Time(GetTime()).SpendsCoinbase(spendsCoinbase).FromTx(tx));
        tx.vin[0].prevout.hash = hash;
    }

    BOOST_CHECK_EXCEPTION(AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey), std::runtime_error, HasReason("bad-blk-sigops"));
    mempool.clear();

// todo 后面的测试需要等到blockinfo中填满至少100个块（第1个块成熟）之后才能测试
    // tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    // tx.vout[0].nValue = BLOCKSUBSIDY;
    // for (unsigned int i = 0; i < 1001; ++i)
    // {
    //     tx.vout[0].nValue -= LOWFEE;
    //     hash = tx.GetHash();
    //     bool spendsCoinbase = i == 0; // only first tx spends coinbase
    //     // If we do set the # of sig ops in the CTxMemPoolEntry, template creation passes
    //     mempool.addUnchecked(hash, entry.Fee(LOWFEE).Time(GetTime()).SpendsCoinbase(spendsCoinbase).SigOpsCost(80).FromTx(tx));
    //     tx.vin[0].prevout.hash = hash;
    // }
    // BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));
    // mempool.clear();

    // // block size > limit
    // tx.vin[0].scriptSig = CScript();
    // // 18 * (520char + DROP) + OP_1 = 9433 bytes
    // std::vector<unsigned char> vchData(520);
    // for (unsigned int i = 0; i < 18; ++i)
    //     tx.vin[0].scriptSig << vchData << OP_DROP;
    // tx.vin[0].scriptSig << OP_1;
    // tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    // tx.vout[0].nValue = BLOCKSUBSIDY;
    // for (unsigned int i = 0; i < 128; ++i)
    // {
    //     tx.vout[0].nValue -= LOWFEE;
    //     hash = tx.GetHash();
    //     bool spendsCoinbase = i == 0; // only first tx spends coinbase
    //     mempool.addUnchecked(hash, entry.Fee(LOWFEE).Time(GetTime()).SpendsCoinbase(spendsCoinbase).FromTx(tx));
    //     tx.vin[0].prevout.hash = hash;
    // }
    // BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));
    // mempool.clear();

    // // orphan in mempool, template creation fails
    // hash = tx.GetHash();
    // mempool.addUnchecked(hash, entry.Fee(LOWFEE).Time(GetTime()).FromTx(tx));
    // BOOST_CHECK_EXCEPTION(AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey), std::runtime_error, HasReason("bad-txns-inputs-missingorspent"));
    // mempool.clear();

    // // child with higher feerate than parent
    // tx.vin[0].scriptSig = CScript() << OP_1;
    // tx.vin[0].prevout.hash = txFirst[1]->GetHash();
    // tx.vout[0].nValue = BLOCKSUBSIDY-HIGHFEE;
    // hash = tx.GetHash();
    // mempool.addUnchecked(hash, entry.Fee(HIGHFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    // tx.vin[0].prevout.hash = hash;
    // tx.vin.resize(2);
    // tx.vin[1].scriptSig = CScript() << OP_1;
    // tx.vin[1].prevout.hash = txFirst[0]->GetHash();
    // tx.vin[1].prevout.n = 0;
    // tx.vout[0].nValue = tx.vout[0].nValue+BLOCKSUBSIDY-HIGHERFEE; //First txn output + fresh coinbase - new txn fee
    // hash = tx.GetHash();
    // mempool.addUnchecked(hash, entry.Fee(HIGHERFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    // BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));
    // mempool.clear();

    // // coinbase in mempool, template creation fails
    // tx.vin.resize(1);
    // tx.vin[0].prevout.SetNull();
    // tx.vin[0].scriptSig = CScript() << OP_0 << OP_1;
    // tx.vout[0].nValue = 0;
    // hash = tx.GetHash();
    // // give it a fee so it'll get mined
    // mempool.addUnchecked(hash, entry.Fee(LOWFEE).Time(GetTime()).SpendsCoinbase(false).FromTx(tx));
    // // Should throw bad-cb-multiple
    // BOOST_CHECK_EXCEPTION(AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey), std::runtime_error, HasReason("bad-cb-multiple"));
    // mempool.clear();

    // // double spend txn pair in mempool, template creation fails
    // tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    // tx.vin[0].scriptSig = CScript() << OP_1;
    // tx.vout[0].nValue = BLOCKSUBSIDY-HIGHFEE;
    // tx.vout[0].scriptPubKey = CScript() << OP_1;
    // hash = tx.GetHash();
    // mempool.addUnchecked(hash, entry.Fee(HIGHFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    // tx.vout[0].scriptPubKey = CScript() << OP_2;
    // hash = tx.GetHash();
    // mempool.addUnchecked(hash, entry.Fee(HIGHFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    // BOOST_CHECK_EXCEPTION(AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey), std::runtime_error, HasReason("bad-txns-inputs-missingorspent"));
    // mempool.clear();

    // // subsidy changing
    // int nHeight = chainActive.Height();
    // // Create an actual 209999-long block chain (without valid blocks).
    // while (chainActive.Tip()->nHeight < 209999) {
    //     CBlockIndex* prev = chainActive.Tip();
    //     CBlockIndex* next = new CBlockIndex();
    //     next->phashBlock = new uint256(InsecureRand256());
    //     pcoinsTip->SetBestBlock(next->GetBlockHash());
    //     next->pprev = prev;
    //     next->nHeight = prev->nHeight + 1;
    //     next->BuildSkip();
    //     chainActive.SetTip(next);
    // }
    // BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));
    // // Extend to a 210000-long block chain.
    // while (chainActive.Tip()->nHeight < 210000) {
    //     CBlockIndex* prev = chainActive.Tip();
    //     CBlockIndex* next = new CBlockIndex();
    //     next->phashBlock = new uint256(InsecureRand256());
    //     pcoinsTip->SetBestBlock(next->GetBlockHash());
    //     next->pprev = prev;
    //     next->nHeight = prev->nHeight + 1;
    //     next->BuildSkip();
    //     chainActive.SetTip(next);
    // }
    // BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));

    // // invalid p2sh txn in mempool, template creation fails
    // tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    // tx.vin[0].prevout.n = 0;
    // tx.vin[0].scriptSig = CScript() << OP_1;
    // tx.vout[0].nValue = BLOCKSUBSIDY-LOWFEE;
    // script = CScript() << OP_0;
    // tx.vout[0].scriptPubKey = GetScriptForDestination(CScriptID(script));
    // hash = tx.GetHash();
    // mempool.addUnchecked(hash, entry.Fee(LOWFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    // tx.vin[0].prevout.hash = hash;
    // tx.vin[0].scriptSig = CScript() << std::vector<unsigned char>(script.begin(), script.end());
    // tx.vout[0].nValue -= LOWFEE;
    // hash = tx.GetHash();
    // mempool.addUnchecked(hash, entry.Fee(LOWFEE).Time(GetTime()).SpendsCoinbase(false).FromTx(tx));
    // // Should throw block-validation-failed
    // BOOST_CHECK_EXCEPTION(AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey), std::runtime_error, HasReason("block-validation-failed"));
    // mempool.clear();

    // // Delete the dummy blocks again.
    // while (chainActive.Tip()->nHeight > nHeight) {
    //     CBlockIndex* del = chainActive.Tip();
    //     chainActive.SetTip(del->pprev);
    //     pcoinsTip->SetBestBlock(del->pprev->GetBlockHash());
    //     delete del->phashBlock;
    //     delete del;
    // }

    // // non-final txs in mempool
    // SetMockTime(chainActive.Tip()->GetMedianTimePast()+1);
    // int flags = LOCKTIME_VERIFY_SEQUENCE|LOCKTIME_MEDIAN_TIME_PAST;
    // // height map
    // std::vector<int> prevheights;

    // // relative height locked
    // tx.nVersion = 2;
    // tx.vin.resize(1);
    // prevheights.resize(1);
    // tx.vin[0].prevout.hash = txFirst[0]->GetHash(); // only 1 transaction
    // tx.vin[0].prevout.n = 0;
    // tx.vin[0].scriptSig = CScript() << OP_1;
    // tx.vin[0].nSequence = chainActive.Tip()->nHeight + 1; // txFirst[0] is the 2nd block
    // prevheights[0] = baseheight + 1;
    // tx.vout.resize(1);
    // tx.vout[0].nValue = BLOCKSUBSIDY-HIGHFEE;
    // tx.vout[0].scriptPubKey = CScript() << OP_1;
    // tx.nLockTime = 0;
    // hash = tx.GetHash();
    // mempool.addUnchecked(hash, entry.Fee(HIGHFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    // BOOST_CHECK(CheckFinalTx(tx, flags)); // Locktime passes
    // BOOST_CHECK(!TestSequenceLocks(tx, flags)); // Sequence locks fail
    // BOOST_CHECK(SequenceLocks(tx, flags, &prevheights, CreateBlockIndex(chainActive.Tip()->nHeight + 2))); // Sequence locks pass on 2nd block

    // // relative time locked
    // tx.vin[0].prevout.hash = txFirst[1]->GetHash();
    // tx.vin[0].nSequence = CTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG | (((chainActive.Tip()->GetMedianTimePast()+1-chainActive[1]->GetMedianTimePast()) >> CTxIn::SEQUENCE_LOCKTIME_GRANULARITY) + 1); // txFirst[1] is the 3rd block
    // prevheights[0] = baseheight + 2;
    // hash = tx.GetHash();
    // mempool.addUnchecked(hash, entry.Time(GetTime()).FromTx(tx));
    // BOOST_CHECK(CheckFinalTx(tx, flags)); // Locktime passes
    // BOOST_CHECK(!TestSequenceLocks(tx, flags)); // Sequence locks fail

    // for (int i = 0; i < CBlockIndex::nMedianTimeSpan; i++)
    //     chainActive.Tip()->GetAncestor(chainActive.Tip()->nHeight - i)->nTime += 512; //Trick the MedianTimePast
    // BOOST_CHECK(SequenceLocks(tx, flags, &prevheights, CreateBlockIndex(chainActive.Tip()->nHeight + 1))); // Sequence locks pass 512 seconds later
    // for (int i = 0; i < CBlockIndex::nMedianTimeSpan; i++)
    //     chainActive.Tip()->GetAncestor(chainActive.Tip()->nHeight - i)->nTime -= 512; //undo tricked MTP

    // // absolute height locked
    // tx.vin[0].prevout.hash = txFirst[2]->GetHash();
    // tx.vin[0].nSequence = CTxIn::SEQUENCE_FINAL - 1;
    // prevheights[0] = baseheight + 3;
    // tx.nLockTime = chainActive.Tip()->nHeight + 1;
    // hash = tx.GetHash();
    // mempool.addUnchecked(hash, entry.Time(GetTime()).FromTx(tx));
    // BOOST_CHECK(!CheckFinalTx(tx, flags)); // Locktime fails
    // BOOST_CHECK(TestSequenceLocks(tx, flags)); // Sequence locks pass
    // BOOST_CHECK(IsFinalTx(tx, chainActive.Tip()->nHeight + 2, chainActive.Tip()->GetMedianTimePast())); // Locktime passes on 2nd block

    // // absolute time locked
    // tx.vin[0].prevout.hash = txFirst[3]->GetHash();
    // tx.nLockTime = chainActive.Tip()->GetMedianTimePast();
    // prevheights.resize(1);
    // prevheights[0] = baseheight + 4;
    // hash = tx.GetHash();
    // mempool.addUnchecked(hash, entry.Time(GetTime()).FromTx(tx));
    // BOOST_CHECK(!CheckFinalTx(tx, flags)); // Locktime fails
    // BOOST_CHECK(TestSequenceLocks(tx, flags)); // Sequence locks pass
    // BOOST_CHECK(IsFinalTx(tx, chainActive.Tip()->nHeight + 2, chainActive.Tip()->GetMedianTimePast() + 1)); // Locktime passes 1 second later

    // // mempool-dependent transactions (not added)
    // tx.vin[0].prevout.hash = hash;
    // prevheights[0] = chainActive.Tip()->nHeight + 1;
    // tx.nLockTime = 0;
    // tx.vin[0].nSequence = 0;
    // BOOST_CHECK(CheckFinalTx(tx, flags)); // Locktime passes
    // BOOST_CHECK(TestSequenceLocks(tx, flags)); // Sequence locks pass
    // tx.vin[0].nSequence = 1;
    // BOOST_CHECK(!TestSequenceLocks(tx, flags)); // Sequence locks fail
    // tx.vin[0].nSequence = CTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG;
    // BOOST_CHECK(TestSequenceLocks(tx, flags)); // Sequence locks pass
    // tx.vin[0].nSequence = CTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG | 1;
    // BOOST_CHECK(!TestSequenceLocks(tx, flags)); // Sequence locks fail

    // BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));

    // // None of the of the absolute height/time locked tx should have made
    // // it into the template because we still check IsFinalTx in CreateNewBlock,
    // // but relative locked txs will if inconsistently added to mempool.
    // // For now these will still generate a valid template until BIP68 soft fork
    // BOOST_CHECK_EQUAL(pblocktemplate->block.vtx.size(), 3);
    // // However if we advance height by 1 and time by 512, all of them should be mined
    // for (int i = 0; i < CBlockIndex::nMedianTimeSpan; i++)
    //     chainActive.Tip()->GetAncestor(chainActive.Tip()->nHeight - i)->nTime += 512; //Trick the MedianTimePast
    // chainActive.Tip()->nHeight++;
    // SetMockTime(chainActive.Tip()->GetMedianTimePast() + 1);

    // BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));
    // BOOST_CHECK_EQUAL(pblocktemplate->block.vtx.size(), 5);

    // chainActive.Tip()->nHeight--;
    // SetMockTime(0);
    // mempool.clear();

    // TestPackageSelection(chainparams, scriptPubKey, txFirst);

    // fCheckpointsEnabled = true;
}

BOOST_AUTO_TEST_SUITE_END()
