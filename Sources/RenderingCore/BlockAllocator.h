#pragma once

#include <memory>
#include <deque>
#include <algorithm>
#include <optional>

class BlockAllocator
{
public:
    struct FreeMemBlock
    {
        uint64_t pos;
        uint64_t size;
    };

    BlockAllocator(uint64_t size)
    {
        mFreeSpace.push_back({ 0, size });
    }

    bool AllocateBlocks(uint64_t numOfBlocks, uint64_t* output)
    {
        for (auto iter = mFreeSpace.begin(); iter != mFreeSpace.end(); ++iter)
        {
            auto& freeMemBlock = *iter;
            if (numOfBlocks < freeMemBlock.size)
            {
                *output = freeMemBlock.pos;
                freeMemBlock.size -= numOfBlocks;
                freeMemBlock.pos += numOfBlocks;
                if (freeMemBlock.size == 0)
                {
                    mFreeSpace.erase(iter);
                }
                return true;
            }
        }
        return false;
    }

    void FreeBlocks(uint64_t blockStart, uint64_t numOfBlocks)
    {
        FreeMemBlock blockToFree{ blockStart, numOfBlocks };

        if (mFreeSpace.empty())
        {
            mFreeSpace.insert(mFreeSpace.end(), blockToFree);
            return;
        }

        auto iter = std::lower_bound(mFreeSpace.begin(), mFreeSpace.end(), blockToFree, [](const FreeMemBlock& a, const FreeMemBlock& b)
        {
            return a.pos < b.pos;
        });

        //   n - 1        n         n + 1
        // leftBlock blockToFree rightBlock

        std::optional<FreeMemBlock&> rightBlock;
        std::optional<FreeMemBlock&> leftBlock;

        bool isNotEnd = iter != mFreeSpace.end();
        bool isNotBegin = iter != mFreeSpace.begin();

        bool mergeRight = false;
        bool mergeLeft = false;

        if (isNotBegin)
        {
            leftBlock = *(iter - 1);
            mergeLeft = (*leftBlock).pos + (*leftBlock).size == blockToFree.pos;
        }
        if (isNotEnd)
        {
            rightBlock = *iter;
            mergeRight = blockToFree.pos + blockToFree.size == (*rightBlock).pos;
        }

        if (mergeLeft && mergeRight)
        {
            (*leftBlock).size += (*rightBlock).size + blockToFree.size;
            mFreeSpace.erase(iter);
        }
        else if(mergeRight)
        {
            (*rightBlock).pos = blockToFree.pos;
        }
        else if(mergeLeft)
        {
            (*leftBlock).size += blockToFree.size;
        }
        else
        {
            mFreeSpace.insert(iter, blockToFree);
        }
    }

private:
    std::deque<FreeMemBlock> mFreeSpace;
};
