#ifndef __PROGTEST__

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <pthread.h>
#include <semaphore.h>
#include "common.h"
#include "helper.h"

using namespace std;
#endif /* __PROGTEST__ */


/* ------------------------------------ CLASSES --------------------------------------------- */

class CStack
{
private:
    int64_t stackPointer = -1;
    uint32_t max;
    uint32_t pagesUsed = 0;
public:
    CStack(uint32_t max);
    ~CStack();
    uint32_t * S;
    bool push(uint32_t in);
    bool pull(uint32_t & out);
    [[nodiscard]] uint32_t getPagesUsed() const { return pagesUsed; };
    [[nodiscard]] uint32_t pagesLeft() const { return stackPointer+1; };
};

/**
 * Class for managing memory.
 */
class CMemoryManager
{
public:
    CMemoryManager(void *mem, uint32_t totalPages);
    static uint32_t getPhysicalAddress(uint32_t page) { return page*CCPU::PAGE_SIZE; };
    static uint32_t * getRealAddress(uint32_t page, uint8_t * mem) { return (uint32_t *) (mem + page*CCPU::PAGE_SIZE); };
    static uint32_t getPageNum(uint32_t addr);
    void updateThreads(bool increase);
    ~CMemoryManager();

    pthread_mutex_t mMemoryAccess;
    pthread_mutex_t mThreads;
    pthread_cond_t cvThreads;
    uint32_t pageTableRoot;
    CStack * pagesStack;
    uint32_t threadsNum = 0;
    uint32_t totalPages;
    [[nodiscard]] uint32_t usedPages() const { return pagesStack->getPagesUsed(); };
};

class CProcessParams
{
public:
    CCPU * cpu;
    CMemoryManager * memoryManager;
    void * args;
    void (* entryPoint) (CCPU *, void *);
};

/*
 * Own cpu class
 */
class CCPUIml : public CCPU
{
public:
    CCPUIml(uint8_t * mem, uint32_t pageTableRoot, CMemoryManager * memManager);
    ~CCPUIml() override;

    virtual uint32_t GetMemLimit() const
    { return dataPagesAllocated; }

    virtual bool SetMemLimit(uint32_t pages);

    virtual bool NewProcess(void *processArg,
                            void           (*entryPoint)(CCPU *, void *),
                            bool copyMem);

    bool increaseLimit(uint32_t pages);
    bool decreaseLimit(uint32_t pages);
    uint32_t requiredTables(uint32_t pages);
    void writeTableRecord(uint32_t * tableStart, uint32_t recordNum, uint32_t pageNum);
    bool writeL2Table(uint32_t page);
    bool newL2Table();
    void givePage();
    bool copyMemory(CCPUIml * destCpu);
    bool copyOnWrite(CCPUIml * destCpu);
    uint32_t * pageWalk(uint32_t rootIndex, uint32_t inL2Index);
    bool isShared(uint32_t * pageEntryAddr);

    CMemoryManager * memoryManager;
    uint16_t rootTableIndex = 0;
    uint32_t dataPagesAllocated = 0;
    uint32_t tablePagesAllocated = 1;
    uint16_t l2Index = CCPU::PAGE_DIR_ENTRIES;
    uint32_t * rootTableAddr = nullptr;
    uint32_t * currentL2Addr = nullptr;

protected:
     virtual bool             pageFaultHandler              ( uint32_t          address,
                                                              bool              write );
};


/* ------------------------------------ FUNCTIONS --------------------------------------------- */


void * threadWrapper(void * params)
{
    CProcessParams * rParams = (CProcessParams *) params;

    rParams->memoryManager->updateThreads(true);
    rParams->entryPoint(rParams->cpu, rParams->args);
    delete rParams->cpu;
    rParams->memoryManager->updateThreads(false);

    delete rParams;
    return nullptr;
}


void MemMgr(void *mem, uint32_t totalPages, void *processArg, void (*mainProcess)(CCPU *, void *))
{
    CMemoryManager * memoryManager = new CMemoryManager(mem, totalPages);
    CCPUIml * myCpu = new CCPUIml((uint8_t *) mem, memoryManager->pageTableRoot, memoryManager);

    mainProcess(myCpu, processArg);

    pthread_mutex_lock(&memoryManager->mThreads);
    while (memoryManager->threadsNum > 0)
        pthread_cond_wait(&memoryManager->cvThreads, &memoryManager->mThreads);
    pthread_mutex_unlock(&memoryManager->mThreads);

    delete myCpu;
    delete memoryManager;
}


/* ------------------------------------ CLASSES IMPLEMENTATION --------------------------------------------- */

CStack::CStack(uint32_t max) : max(max)
{
    S = new uint32_t[max];
}

CStack::~CStack()
{
    delete [] S;
}

bool CStack::push(uint32_t in)
{
    if (stackPointer >= max-1)
    {
        printf("Pushing beyond limit\n");
        return false;
    }
    stackPointer++;
    if (pagesUsed > 0)
        pagesUsed--;
    S[stackPointer] = in;
    return true;
}

bool CStack::pull(uint32_t & out)
{
    if (stackPointer < 0)
    {
        printf("Unable to allocate new page.\n");
        return false;
    }
    out = S[stackPointer];
    stackPointer--;
    pagesUsed++;
    return true;
}

CMemoryManager::CMemoryManager(void *mem, uint32_t totalPages) : totalPages(totalPages)
{
    pthread_mutex_init(&mMemoryAccess, nullptr);
    pthread_mutex_init(&mThreads, nullptr);
    pthread_cond_init(&cvThreads, nullptr);

    pthread_mutex_lock(&mMemoryAccess);

    // null whole memory block
    memset(mem, 0, CCPU::PAGE_SIZE * totalPages);

    // push all pages to stack
    pagesStack = new CStack(totalPages);

    for (uint32_t i = totalPages; i > 0; i--)
        pagesStack->push(i-1);

    uint32_t newPage;
    if (!pagesStack->pull(newPage))
        printf("cannot allocate first page\n");

    pageTableRoot = getPhysicalAddress(newPage);

    pthread_mutex_unlock(&mMemoryAccess);
}

CMemoryManager::~CMemoryManager()
{
    delete pagesStack;
}

uint32_t CMemoryManager::getPageNum(uint32_t addr)
{
    return addr / CCPU::PAGE_SIZE;
}

void CMemoryManager::updateThreads(bool increase)
{
    pthread_mutex_lock(&mThreads);
    if (increase)
        threadsNum++;
    else
    {
        if (threadsNum == 0)
            printf("Threads zero!\n");
        threadsNum--;
        pthread_cond_signal(&cvThreads);
    }
    pthread_mutex_unlock(&mThreads);
    if (threadsNum > 65)
        printf("More threads than expected.\n");
}


CCPUIml::CCPUIml(uint8_t *mem, uint32_t pageTableRoot, CMemoryManager * memManager) :
        CCPU(mem, pageTableRoot), memoryManager(memManager)
{
    // null pageTableRoot
    rootTableAddr = CMemoryManager::getRealAddress(CMemoryManager::getPageNum(m_PageTableRoot), m_MemStart);
}


bool CCPUIml::SetMemLimit(uint32_t pages)
{
    if (pages > dataPagesAllocated && pages-dataPagesAllocated + requiredTables(pages)-tablePagesAllocated > memoryManager->pagesStack->pagesLeft())
        return false;

    // edge case: pages == allocated -- just lock,unlock,return true
    bool res = true;

    pthread_mutex_lock(&memoryManager->mMemoryAccess);
    if (pages > dataPagesAllocated)
        res = increaseLimit(pages);
    else if (pages < dataPagesAllocated)
        res = decreaseLimit(pages);
    pthread_mutex_unlock(&memoryManager->mMemoryAccess);
    return res;
}

bool CCPUIml::NewProcess(void *processArg, void (*entryPoint)(CCPU *, void *), bool copyMem)
{
    // get root Table Page
    pthread_mutex_lock(&memoryManager->mMemoryAccess);
    uint32_t newPage;
    if(!memoryManager->pagesStack->pull(newPage))
    {
        pthread_mutex_unlock(&memoryManager->mMemoryAccess);
        return false;
    }

    CCPUIml * newCpu = new CCPUIml(m_MemStart, CMemoryManager::getPhysicalAddress(newPage), memoryManager);

    if (copyMem)
    {
        if (!copyOnWrite(newCpu))
        {
            delete newCpu;
            pthread_mutex_unlock(&memoryManager->mMemoryAccess);
            return false;
        }

    }

    pthread_mutex_unlock(&memoryManager->mMemoryAccess);


    // prepare arguments
    CProcessParams * processParams = new CProcessParams;
    processParams->cpu = newCpu;
    processParams->memoryManager = memoryManager;
    processParams->args = processArg;
    processParams->entryPoint = entryPoint;

    // start it in new thread
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_t thread;
    if (pthread_create(&thread, &attr, threadWrapper, (void *) processParams) != 0)
    {
        printf("Cannot create thread\n");
        return false;
    }
    pthread_attr_destroy(&attr);
    return true;
}

bool CCPUIml::increaseLimit(uint32_t pages)
{
    uint32_t memPagesNeeded = pages - dataPagesAllocated;

    if (memPagesNeeded+requiredTables(pages)-tablePagesAllocated > memoryManager->pagesStack->pagesLeft())
        return false;

    for (uint32_t i = 0; i < memPagesNeeded; i++)
    {
        uint32_t newPage;
        if (!memoryManager->pagesStack->pull(newPage))
            return false;
        dataPagesAllocated++;
        if (!writeL2Table(newPage))
            return false;
    }

    return true;
}

bool CCPUIml::decreaseLimit(uint32_t pages)
{
    while (dataPagesAllocated > pages)
        givePage();
    return true;
}

uint32_t CCPUIml::requiredTables(uint32_t pages)
{
    return (!pages) ? 1 : (1 + (pages/CCPU::PAGE_DIR_ENTRIES) + ((pages % CCPU::PAGE_DIR_ENTRIES) ? 1 : 0));
}

void CCPUIml::writeTableRecord(uint32_t * tableStart, uint32_t recordNum, uint32_t pageNum)
{
    if (recordNum > CCPU::PAGE_DIR_ENTRIES)
    {
        printf("Trying to write out of bound of table");
        return;
    }

    uint32_t record = CMemoryManager::getPhysicalAddress(pageNum) & CCPU::ADDR_MASK;

    // need to set last 3 bits
    const uint32_t reqMask = BIT_PRESENT | BIT_USER | BIT_WRITE;
    record |= reqMask;
    (*(tableStart + recordNum)) = record;
}

bool CCPUIml::writeL2Table(uint32_t page)
{
    // if needed allocate new page table
    if (l2Index >= CCPU::PAGE_DIR_ENTRIES)
        if (!newL2Table())
            return false;
    writeTableRecord(currentL2Addr, l2Index, page);
    l2Index++;
    return true;
}

bool CCPUIml::newL2Table()
{
    uint32_t newTable;
    if (!memoryManager->pagesStack->pull(newTable))
    {
        printf("Unable to allocate new table\n");
        return false;
    }
    tablePagesAllocated++;
    writeTableRecord(rootTableAddr, rootTableIndex, newTable);
    rootTableIndex++;
    if (rootTableIndex >= CCPU::PAGE_DIR_ENTRIES)
    {
        printf("Ooof, smth went terribly wrong.\n");
        return false;
    }
    currentL2Addr = CMemoryManager::getRealAddress(newTable, m_MemStart);
    l2Index = 0;
    return true;
}

void CCPUIml::givePage()
{
    if (rootTableIndex == 0 && l2Index < 0)
    {
        printf("Too much deallocating\n");
        return;
    }

    if (l2Index <= 0)
    {
        l2Index = CCPU::PAGE_DIR_ENTRIES;
        memset(currentL2Addr, 0, CCPU::PAGE_SIZE);
        rootTableIndex--;
        memoryManager->pagesStack->push(CMemoryManager::getPageNum(*(rootTableAddr + rootTableIndex)));
        memset((rootTableAddr + rootTableIndex), 0, 4);
        tablePagesAllocated--;
        currentL2Addr = CMemoryManager::getRealAddress(CMemoryManager::getPageNum(*(rootTableAddr + rootTableIndex-1)), m_MemStart);
    }

    else
    {
        l2Index--;

        if (!isShared(currentL2Addr + l2Index))
        {
            memoryManager->pagesStack->push(CMemoryManager::getPageNum(*(currentL2Addr + l2Index)));
            memset((currentL2Addr + l2Index), 0, 4);
        }

        dataPagesAllocated--;
    }
}

CCPUIml::~CCPUIml()
{
    if (dataPagesAllocated > 0)
        decreaseLimit(0);
    pthread_mutex_lock(&memoryManager->mMemoryAccess);
    memset(CMemoryManager::getRealAddress(CMemoryManager::getPageNum(m_PageTableRoot), m_MemStart), 0, CCPU::PAGE_SIZE);
    memoryManager->pagesStack->push(CMemoryManager::getPageNum(m_PageTableRoot));
    pthread_mutex_unlock(&memoryManager->mMemoryAccess);
}

bool CCPUIml::copyMemory(CCPUIml *destCpu)
{
    pthread_mutex_unlock(&memoryManager->mMemoryAccess);
    if (!destCpu->SetMemLimit(dataPagesAllocated))
    {
        pthread_mutex_lock(&memoryManager->mMemoryAccess);
        return false;
    }
    pthread_mutex_lock(&memoryManager->mMemoryAccess);

    uint32_t tmpRootIndex = 0;
    uint32_t tmpL2Index = 0;

    for (uint32_t i = 0; i < dataPagesAllocated; i++)
    {

        if (tmpL2Index >= CCPU::PAGE_DIR_ENTRIES)
        {
            tmpRootIndex++;
            tmpL2Index = 0;
        }
        // L1 --- physical addr of L2 ---> pointer to Page what we want to copy
        uint32_t * origPage = pageWalk(tmpRootIndex, tmpL2Index);
        uint32_t * newPage = destCpu->pageWalk(tmpRootIndex, tmpL2Index);
        memcpy(newPage, origPage, CCPU::PAGE_SIZE);
        tmpL2Index++;
    }

    return true;
}

uint32_t *CCPUIml::pageWalk(uint32_t rootIndex, uint32_t inL2Index)
{
    // L1Entry ---> L2start physical
    // L2startAddr + index == L2entry
    uint32_t L1Entry = (*(rootTableAddr + rootIndex)) & CCPU::ADDR_MASK;
    uint32_t * L2Addr = CMemoryManager::getRealAddress(CMemoryManager::getPageNum(L1Entry), m_MemStart);
    uint32_t  L2Entry = (*(L2Addr + inL2Index)) & CCPU::ADDR_MASK;
    return CMemoryManager::getRealAddress(CMemoryManager::getPageNum(L2Entry), m_MemStart);
}

bool CCPUIml::pageFaultHandler(uint32_t address, bool write)
{
    if (!write)
        return false;

    uint32_t rootIndex = address >> 22;
    uint32_t tmpL2Index = (address >> 12) & (CCPU::PAGE_DIR_ENTRIES-1);

    // unallocated memory
    if (rootIndex * CCPU::PAGE_DIR_ENTRIES + tmpL2Index >= dataPagesAllocated)
        return false;

    pthread_mutex_lock(&memoryManager->mMemoryAccess);
    uint32_t newPage;
    if (!memoryManager->pagesStack->pull(newPage))
    {
        pthread_mutex_unlock(&memoryManager->mMemoryAccess);
        return false;
    }
    pthread_mutex_unlock(&memoryManager->mMemoryAccess);

    uint32_t oldL1Entry = (*(rootTableAddr + rootIndex)) & CCPU::ADDR_MASK;
    uint32_t * oldL2Addr = CMemoryManager::getRealAddress(CMemoryManager::getPageNum(oldL1Entry), m_MemStart);
    uint32_t * oldPageAddr = pageWalk(rootIndex, tmpL2Index);
    uint32_t * newPageAddr = CMemoryManager::getRealAddress(newPage, m_MemStart);

    memcpy(newPageAddr, oldPageAddr, CCPU::PAGE_SIZE);
    writeTableRecord(oldL2Addr, tmpL2Index, newPage);
    return true;
}

bool CCPUIml::copyOnWrite(CCPUIml *destCpu)
{
    destCpu->dataPagesAllocated = dataPagesAllocated;
    destCpu->tablePagesAllocated = tablePagesAllocated;

    // table pages - L1
    for (uint32_t i = 0; i < tablePagesAllocated-1; i++)
    {
        uint32_t newPage;
        if (!memoryManager->pagesStack->pull(newPage))
            return false;

        destCpu->writeTableRecord(destCpu->rootTableAddr, destCpu->rootTableIndex, newPage);
        destCpu->rootTableIndex++;
    }

    uint32_t rootIndex = 0;
    uint32_t tmpL2index = 0;

    for (uint32_t i = 0; i <dataPagesAllocated; i++)
    {
        if (tmpL2index >= CCPU::PAGE_DIR_ENTRIES)
        {
            rootIndex++;
            tmpL2index = 0;
        }

        uint32_t oldL1Entry = (*(rootTableAddr + rootIndex)) & CCPU::ADDR_MASK;
        uint32_t * oldL2Addr = CMemoryManager::getRealAddress(CMemoryManager::getPageNum(oldL1Entry), m_MemStart);
        *(oldL2Addr+tmpL2index) = *(oldL2Addr+tmpL2index) & ~(0x2);   // unset writable bit

        uint32_t newL1Entry = (*(destCpu->rootTableAddr + rootIndex)) & CCPU::ADDR_MASK;
        uint32_t * newL2Addr = CMemoryManager::getRealAddress(CMemoryManager::getPageNum(newL1Entry), m_MemStart);

        (*(newL2Addr+tmpL2index)) = (*(oldL2Addr+tmpL2index));
        tmpL2index++;
    }

    destCpu->rootTableIndex = rootTableIndex;
    destCpu->l2Index = l2Index;
    destCpu->currentL2Addr = CMemoryManager::getRealAddress(CMemoryManager::getPageNum(*(destCpu->rootTableAddr + destCpu->rootTableIndex-1)), destCpu->m_MemStart);

    return true;
}

bool CCPUIml::isShared(uint32_t * pageEntryAddr)
{
    //printTables(m_MemStart, m_PageTableRoot, dataPagesAllocated);
    if ((*pageEntryAddr) & CCPU::BIT_WRITE)
        return false;
    return true;
}
