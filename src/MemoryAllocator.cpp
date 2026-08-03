#include <ilive2d.h>

//
// Cubism ランタイムへ渡すメモリアロケータ。
//
//  - Windows: 専用ヒープ (LFH 有効) を作り、そこから確保する従来実装。
//             大量の細かいアロケートに対する断片化を抑える目的。
//  - その他 : 標準 malloc/free による汎用実装。
//
// IMemoryAllocator は Allocate/Deallocate のみを要求するので、
// プラットフォームごとに実装を差し替えても呼び出し側は不変。
//

#ifdef _WIN32

#include <Windows.h>

class MemoryAllocator: public IMemoryAllocator {
public:
    MemoryAllocator();
    virtual ~MemoryAllocator();
    virtual void *Allocate(size_t size);
    virtual bool Deallocate(void *mem);
private:
    static const DWORD HeapFlag = 0;
    HANDLE HeapHandle;
};

MemoryAllocator::MemoryAllocator()
:HeapHandle(nullptr)
{
}

MemoryAllocator::~MemoryAllocator()
{
    if( HeapHandle ) {
        ::HeapDestroy(HeapHandle);
        HeapHandle = nullptr;
    }
}

void* MemoryAllocator::Allocate(size_t alloc_size)
{
    if (!HeapHandle) {
        int64_t size = 0;
        // 空きメモリの1/4をもっていく
        if( size == 0 ) {
            MEMORYSTATUSEX status = { sizeof(MEMORYSTATUSEX) };
            ::GlobalMemoryStatusEx(&status);
            if( status.ullAvailVirtual < status.ullTotalPhys ) {
                size = status.ullAvailVirtual / 4;
            } else {
                size = status.ullTotalPhys / 4;
            }
        }
        while( HeapHandle == NULL && size > (1024*1024) ) {
            HeapHandle = ::HeapCreate( HeapFlag, (SIZE_T)size, 0 );
            if( HeapHandle == NULL ) {
                size /= 2;
            }
        }
        if( HeapHandle ) {
            ULONG HeapInformation = 2;
            BOOL lfhenable = ::HeapSetInformation( HeapHandle, HeapCompatibilityInformation, &HeapInformation, sizeof(HeapInformation) );
        }
    }
    if( HeapHandle == NULL ) return NULL;
    void* result = ::HeapAlloc( HeapHandle, HeapFlag, alloc_size );
    if( result == NULL ) {
        ::HeapCompact( HeapHandle, HeapFlag );	// try compact
        result = ::HeapAlloc( HeapHandle, HeapFlag, alloc_size ); // retry
    }
    return result;
}

bool MemoryAllocator::Deallocate(void* memory)
{
    bool ret = false;
    if (HeapHandle) {
        ret = ::HeapFree( HeapHandle, HeapFlag, memory ) != 0;
        ::HeapCompact( HeapHandle, HeapFlag );
    }
    return ret;
}

#else  // !_WIN32

#include <cstdlib>

// 汎用実装: 標準ライブラリの malloc/free を用いる。
class MemoryAllocator: public IMemoryAllocator {
public:
    virtual ~MemoryAllocator() {}
    virtual void *Allocate(size_t size) {
        return ::malloc(size);
    }
    virtual bool Deallocate(void *mem) {
        ::free(mem);
        return true;
    }
};

#endif // _WIN32

IMemoryAllocator *createMemoryAllocator()
{
    return new MemoryAllocator();
}
