#include <cstdio>
#include <cmath>
#include <ctime>
#include "pin.H"

#define ADDR_SIZE 32
/**************************************
 * Cache Model Base Class
**************************************/
class CacheModel
{
public:
    // Constructor
    CacheModel(){}
    CacheModel(UINT32 block_num, UINT32 log_block_size)
        : m_block_num(block_num), m_blksz_log(log_block_size),
          m_rd_reqs(0), m_wr_reqs(0), m_rd_hits(0), m_wr_hits(0)
    {
        m_valids = new bool[m_block_num];
        m_tags = new UINT32[m_block_num];
        m_replace_q = new UINT32[m_block_num];

        for (UINT i = 0; i < m_block_num; i++)
        {
            m_valids[i] = false;
            m_replace_q[i] = i;
        }
    }

    // Destructor
    virtual ~CacheModel()
    {
        delete[] m_valids;
        delete[] m_tags;
        delete[] m_replace_q;
    }

    // Update the cache state whenever data is read
    void readReq(UINT32 mem_addr)
    {
        m_rd_reqs++;
        if (access(mem_addr)) m_rd_hits++;
    }

    // Update the cache state whenever data is written
    void writeReq(UINT32 mem_addr)
    {
        m_wr_reqs++;
        if (access(mem_addr)) m_wr_hits++;
    }

    UINT32 getRdReq() { return m_rd_reqs; }
    UINT32 getWrReq() { return m_wr_reqs; }

    void dumpResults()
    {
        float rdHitRate = 100 * (float)m_rd_hits/m_rd_reqs;
        float wrHitRate = 100 * (float)m_wr_hits/m_wr_reqs;
        printf("\tread req: %lu,\thit: %lu,\thit rate: %.2f%%\n", m_rd_reqs, m_rd_hits, rdHitRate);
        printf("\twrite req: %lu,\thit: %lu,\thit rate: %.2f%%\n", m_wr_reqs, m_wr_hits, wrHitRate);
    }

protected:
    UINT32 m_block_num;     // The number of cache blocks
    UINT32 m_blksz_log;     // 块大小的对数

    bool* m_valids;
    UINT32* m_tags;
    UINT32* m_replace_q;    // Cache块替换的候选队列

    UINT64 m_rd_reqs;       // The number of read-requests
    UINT64 m_wr_reqs;       // The number of write-requests
    UINT64 m_rd_hits;       // The number of hit read-requests
    UINT64 m_wr_hits;       // The number of hit write-requests

    // Look up the cache to decide whether the access is hit or missed
    virtual bool lookup(UINT32 mem_addr, UINT32& blk_id) = 0;

    // Access the cache: update m_replace_q if hit, otherwise replace a block and update m_replace_q
    virtual bool access(UINT32 mem_addr) = 0;

    // Update m_replace_q
    virtual void updateReplaceQ(UINT32 blk_id) = 0;
};

/**************************************
 * Fully Associative Cache Class
**************************************/
class FullAssoCache : public CacheModel
{
public:
    // Constructor
    FullAssoCache(UINT32 block_num, UINT32 log_block_size)
        : CacheModel(block_num, log_block_size) {}

    // Destructor
    ~FullAssoCache() {}

private:
    UINT32 getTag(UINT32 addr) { return addr>>m_blksz_log; }

    // Look up the cache to decide whether the access is hit or missed
    bool lookup(UINT32 mem_addr, UINT32& blk_id)
    {
        UINT32 tag = getTag(mem_addr);
        // 全相联需要遍历整个cache查找tag是否存在
        for(UINT32 i = 0; i < m_block_num; i++)
        {
            // 要求tag找到且有效，如果无效也不需要继续找下去了
            if(m_tags[i]==tag)
            {
                blk_id = i;
                if(m_valids[i])
                {
                    return true;
                }
                else
                    return false;
            }
        }
        // 没有找到
        return false;
    }

    // Access the cache: update m_replace_q if hit, otherwise replace a block and update m_replace_q
    bool access(UINT32 mem_addr)
    {
        UINT32 blk_id;
        // printf("Try access!\n");
        if (lookup(mem_addr, blk_id))
        {
            updateReplaceQ(blk_id);     // Update m_replace_q
            return true;
        }

        // Get the to-be-replaced block id using m_replace_q
        UINT32 bid_2be_replaced = m_replace_q[0];// TODO
        // Replace the cache block
        m_tags[bid_2be_replaced] = getTag(mem_addr);
        m_valids[bid_2be_replaced] = true;
        updateReplaceQ(bid_2be_replaced);
        // printf("Finish access!\n");
        return false;
    }

    // Update m_replace_q
    void updateReplaceQ(UINT32 blk_id)
    {
        // TODO
        // 遍历找到队列中的blk_id
        for(UINT32 i = 0; i < m_block_num; i++)
        {
            if(m_replace_q[i]==blk_id)
            {
                // 将队列中blk_id插入队列尾，i+1到队列尾的位置全部向前挪动1位
                for(UINT32 j = i+1; j < m_block_num; j++)
                {
                    m_replace_q[j-1] = m_replace_q[j];
                }
                m_replace_q[m_block_num-1] = blk_id;
                break;
            }
        }
        // printf("update!\n");
    }
};

/**************************************
 * Directly Mapped Cache Class
**************************************/
class DirectMapCache : public CacheModel
{
public:
    // Constructor
    DirectMapCache(UINT32 block_num, UINT32 log_block_size)
        : CacheModel(block_num, log_block_size) {
            m_block_num_log = (UINT32)log((double)block_num)/log((double)2);  // 计算block号对应的位数
        }

    // Destructor
    ~DirectMapCache() {}

private:

    // 
    UINT32 m_block_num_log;
    //tag | blkID | offset
    UINT32 getTag(UINT32 addr)
    { 
        return addr>>(m_block_num_log+m_blksz_log);
    }

    UINT32 getBlkId(UINT32 addr)
    {
        return (addr<<(ADDR_SIZE - m_block_num_log-m_blksz_log))>>(ADDR_SIZE - m_block_num_log);
    }

    // Look up the cache to decide whether the access is hit or missed
    bool lookup(UINT32 mem_addr, UINT32& blk_id)
    {
        // TODO
        // 获得cache块号
        blk_id = getBlkId(mem_addr);
        if(m_tags[blk_id] == getTag(mem_addr))
        {
            if(m_valids[blk_id]==true)
                return true;
            else
                return false;
        }
        return false;
    }

    // Access the cache: update m_replace_q if hit, otherwise replace a block and update m_replace_q
    bool access(UINT32 mem_addr)
    {
        // TODO
        UINT32 blk_id;
        // 找到则直接返回true
        if(lookup(mem_addr, blk_id))
            return true;
        // 否则换入
        m_tags[blk_id] = getTag(mem_addr);
        m_valids[blk_id] = true;
        return false;
    }

    // Update m_replace_q
    void updateReplaceQ(UINT32 blk_id)
    {
        // TODO: do nothing
    }
};

/**************************************
 * Set-Associative Cache Class
**************************************/
class SetAssoCache : public CacheModel
{
public:
    // Constructor
    SetAssoCache(UINT32 setNum_log, UINT32 log_block_size, UINT32 setBlkNum)
            : CacheModel(pow((double)2, (double)setNum_log) * setBlkNum, log_block_size)
    {
        this->setBlkNum = setBlkNum;
        this->setNum_log = setNum_log;
    }

    // Destructor
    ~SetAssoCache() {}

private:

    // 
    UINT32 setBlkNum;
    UINT32 setNum_log;
    // UINT32 ** replaceQ_array;
    // tag | setID | offset  
    UINT32 getTag(UINT32 addr) { return addr>>(m_blksz_log+setNum_log); }

    UINT32 getSetID(UINT32 addr) { return ((addr<<(ADDR_SIZE-m_blksz_log-setNum_log))>>(ADDR_SIZE-setNum_log)); }
    // Look up the cache to decide whether the access is hit or missed
    bool lookup(UINT32 mem_addr, UINT32& blk_id)
    {
        // TODO
        UINT32 set_start = getSetID(mem_addr) * setBlkNum;
        UINT32 tag = getTag(mem_addr);
        for(UINT32 i = 0; i < setBlkNum; i++)
        {
            blk_id = set_start+i;
            if(m_tags[set_start+i] == tag)
            {
                if(m_valids[blk_id] == true)
                    return true;
                else
                    return false;
            }
        }
        return false;
    }

    // Access the cache: update m_replace_q if hit, otherwise replace a block and update m_replace_q
    bool access(UINT32 mem_addr)
    {
        // TODO
        // printf("try access!\n");
        UINT32 blk_id;
        UINT32 set_id;
        if(lookup(mem_addr, blk_id))
        {
            updateReplaceQ(blk_id);
            return true;
        }
        set_id = blk_id / setBlkNum;
        UINT32 bid_2be_replaced = m_replace_q[set_id*setBlkNum];
        m_tags[bid_2be_replaced] = getTag(mem_addr);
        m_valids[bid_2be_replaced] = true;
        updateReplaceQ(bid_2be_replaced);
        return false
        ;
    }

    // Update m_replace_q
    // m_replace_q被切分为每个组的小队列，每次只更新小队列
    void updateReplaceQ(UINT32 blk_id)
    {
        // TODO
        // printf("try update!\n");
        UINT32 set_id = blk_id / setBlkNum;
        UINT32 set_start = set_id * setBlkNum;
        for(UINT32 i = 0; i < setBlkNum; i++)
        {
            if(m_replace_q[set_start+i]==blk_id)
            {
                // 将队列中blk_id插入队列尾，i+1到队列尾的位置全部向前挪动1位
                for(UINT32 j = i+1; j < setBlkNum; j++)
                {
                    m_replace_q[set_start + j - 1] = m_replace_q[set_start + j];
                }
                m_replace_q[set_start + setBlkNum - 1] = blk_id;
                break;
            }
        }
    }
};

CacheModel* my_fa_cache;
CacheModel* my_dm_cache;
CacheModel* my_sa_cache;

double time_fa_rd = 0, time_fa_wr = 0;
double time_dm_rd = 0, time_dm_wr = 0;
double time_sa_rd = 0, time_sa_wr = 0;

// Cache reading analysis routine
void readCache(UINT32 mem_addr)
{
    // printf("Try read cache!\n");
    mem_addr = (mem_addr >> 2) << 2;
    // clock_t pt0 = clock();
    my_fa_cache->readReq(mem_addr);
    // printf("finish one read!\n");
    // clock_t pt1 = clock();
    my_dm_cache->readReq(mem_addr);
    // clock_t pt2 = clock();
    my_sa_cache->readReq(mem_addr);
    // clock_t pt3 = clock();
    // printf("Finish all read!\n");
    // time_fa_rd += 1000000*(double)(pt1 - pt0) / CLOCKS_PER_SEC;
    // time_dm_rd += 1000000*(double)(pt2 - pt1) / CLOCKS_PER_SEC;
    // time_sa_rd += 1000000*(double)(pt3 - pt2) / CLOCKS_PER_SEC;
    // printf("Finish read!\n");
}

// Cache writing analysis routine
void writeCache(UINT32 mem_addr)
{
    mem_addr = (mem_addr >> 2) << 2;
    // clock_t pt0 = clock();
    my_fa_cache->writeReq(mem_addr);
    // clock_t pt1 = clock();
    my_dm_cache->writeReq(mem_addr);
    // clock_t pt2 = clock();
    my_sa_cache->writeReq(mem_addr);
    // clock_t pt3 = clock();

    // time_fa_wr += 1000000*(double)(pt1 - pt0) / CLOCKS_PER_SEC;
    // time_dm_wr += 1000000*(double)(pt2 - pt1) / CLOCKS_PER_SEC;
    // time_sa_wr += 1000000*(double)(pt3 - pt2) / CLOCKS_PER_SEC;
}

// This knob will set the cache param m_block_num
KNOB<UINT32> KnobBlockNum(KNOB_MODE_WRITEONCE, "pintool",
        "n", "512", "specify the number of blocks in bytes");

// This knob will set the cache param m_blksz_log
KNOB<UINT32> KnobBlockSizeLog(KNOB_MODE_WRITEONCE, "pintool",
        "b", "6", "specify the log of the block size in bytes");

// This knob will set the cache param m_sets_log
KNOB<UINT32> KnobSetsLog(KNOB_MODE_WRITEONCE, "pintool",
        "r", "7", "specify the log of the number of rows");

// This knob will set the cache param m_asso
KNOB<UINT32> KnobAssociativity(KNOB_MODE_WRITEONCE, "pintool",
        "a", "4", "specify the m_asso");

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
    if (INS_IsMemoryRead(ins))
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)readCache, IARG_MEMORYREAD_EA, IARG_END);
    if (INS_IsMemoryWrite(ins))
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)writeCache, IARG_MEMORYWRITE_EA, IARG_END);
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    printf("\nFully Associative Cache:\n");
    printf("average read time: %.2fus\n", time_fa_rd/my_fa_cache->getRdReq());
    printf("average write time: %.2fus\n", time_fa_rd/my_fa_cache->getWrReq());
    my_fa_cache->dumpResults();
    printf("\nDirectly Mapped Cache:\n");
    printf("average read time: %.2fus\n", time_dm_rd/my_dm_cache->getRdReq());
    printf("average write time: %.2fus\n", time_dm_rd/my_dm_cache->getWrReq());
    my_dm_cache->dumpResults();
    printf("\nSet-Associative Cache:\n");
    printf("average read time: %.2fus\n", time_sa_rd/my_sa_cache->getRdReq());
    printf("average write time: %.2fus\n", time_sa_rd/my_sa_cache->getWrReq());
    my_sa_cache->dumpResults();

    delete my_fa_cache;
    delete my_dm_cache;
    delete my_sa_cache;
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    // my_fa_cache = new FullAssoCache(KnobBlockNum.Value(), KnobBlockSizeLog.Value());
    // my_dm_cache = new DirectMapCache(KnobBlockNum.Value(), KnobBlockSizeLog.Value());
    // my_sa_cache = new SetAssoCache(KnobSetsLog.Value(), KnobBlockSizeLog.Value(), KnobAssociativity.Value());
    // 测试块大小地影响
    my_fa_cache = new SetAssoCache(7, 4, 4);
    my_dm_cache = new SetAssoCache(7, 5, 4);
    my_sa_cache = new SetAssoCache(7, 6, 4);

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
