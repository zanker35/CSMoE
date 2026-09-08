
#include "engine_api/types/basetypes.h"
#include <math.h>

#include <malloc/malloc.h>

#include "game/server/runtime/memory/MemPool.h"
#include "game/shared/interfaces/common.h"
#include "base/math/minmax.h"

/* <3fea40> ../public/MemPool.cpp:35 */
CMemoryPool::CMemoryPool(int blockSize, int numElements)
{
	_blocksPerBlob = numElements;
	_blockSize = blockSize;
	_numBlobs = 0;
	_numElements = 0;

	AddNewBlob();

	_peakAlloc = 0;
	_blocksAllocated = 0;
}

/* <3fe967> ../public/MemPool.cpp:52 */
CMemoryPool::~CMemoryPool()
{
	for (int i = 0; i < _numBlobs; ++i)
		free(_memBlob[i]);
}

/* <3fe99c> ../public/MemPool.cpp:109 */
void CMemoryPool::AddNewBlob()
{
	int sizeMultiplier = pow(2.0, _numBlobs);
	int nElements = _blocksPerBlob * sizeMultiplier;
	int blobSize = _blockSize * nElements;

	_memBlob[_numBlobs] = malloc(blobSize);


	_headOfFreeList = _memBlob[_numBlobs];


	void **newBlob = (void **)_headOfFreeList;
	for (int j = 0; j < nElements - 1; ++j)
	{
		newBlob[0] = (char *)newBlob + _blockSize;
		newBlob = (void **)newBlob[0];
	}

	newBlob[0] = NULL;

	_numElements += nElements;
	++_numBlobs;


}

/* <3fea72> ../public/MemPool.cpp:157 */
void *CMemoryPool::Alloc(unsigned int amount)
{
	void *returnBlock;
	if (amount > (unsigned int)_blockSize)
		return NULL;

	++_blocksAllocated;
	_peakAlloc = max(_peakAlloc, _blocksAllocated);

	if (_blocksAllocated >= _numElements)
		AddNewBlob();


	returnBlock = _headOfFreeList;
	_headOfFreeList = *((void **)_headOfFreeList);
	return returnBlock;
}

/* <3feabe> ../public/MemPool.cpp:193 */
void CMemoryPool::Free(void *memblock)
{
	if (!memblock)
		return;

#ifdef _DEBUG
	Q_memset(memblock, 0xDD, _blockSize);
#endif // _DEBUG

	--_blocksAllocated;
	*((void **)memblock) = _headOfFreeList;
	_headOfFreeList = memblock;
}
