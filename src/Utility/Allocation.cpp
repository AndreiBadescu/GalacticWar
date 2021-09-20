#include "Utility/Allocation.hpp"
#include <iostream>

// In loc ca destructorul sau functia releases sa dealoce memoria, ii schimba protectia, asta face detectarea de buguri mai simpla
#define GAL_ALLOC_PROTECT

#ifdef _WIN32
#include <Windows.h>

internal_func void *PageAlloc(size_t uAllocSize)
{
	return VirtualAlloc(nullptr, uAllocSize, MEM_COMMIT, PAGE_READWRITE);
}

internal_func void PageFree(void *pPage)
{
	VirtualFree(pPage, 0, MEM_RELEASE);
}

internal_func DWORD PageProtect(void *pPage, size_t uSize)
{
	DWORD dwOldProtect;
	VirtualProtect(pPage, uSize, PAGE_NOACCESS, &dwOldProtect);
	return dwOldProtect;
}

#endif

#ifdef __linux__
#include <unistd.h>
#include <sys/mman.h>
#include <iostream>

internal_func void *PageAlloc(size_t uAllocSize)
{
	return mmap(nullptr, uAllocSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

internal_func void PageFree(void *pData)
{
	free(pData);
}

internal_func void PageProtect(void *pPage, size_t uSize)
{
	mprotect(pPage, uSize, PROT_NONE);
}

#endif

#include <assert.h>

namespace gal {

	size_t ArenaAllocator::s_uPageSize = 0;

	size_t ArenaAllocator::GetPageSize()
	{
		return ArenaAllocator::s_uPageSize;
	}

	void ArenaAllocator::Init()
	{
#ifdef __linux__
		s_uPageSize = sysconf(_SC_PAGE_SIZE);
#else
#ifdef _WIN32

		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		s_uPageSize = sysInfo.dwPageSize;

#endif
#endif
	}

	ArenaAllocator::ArenaAllocator()
		: m_pMemory(PageAlloc(4 * s_uPageSize + (4 * s_uPageSize) / s_fDataToMetaRatio)),
		  m_pUserMemory((std::byte *)m_pMemory + (size_t)((4 * s_uPageSize) / s_fDataToMetaRatio)),
		  m_pLast(m_pMemory),
		  m_pCapacity((std::byte *)m_pMemory + (size_t)(4 * s_uPageSize + (4 * s_uPageSize) / s_fDataToMetaRatio)),
		  m_pFreeBlocks((FreeBlock *)m_pMemory)
	{
		assert(m_pMemory && "Arena allocation failed.");
	}

	ArenaAllocator::ArenaAllocator(size_t uNumPages)
		: m_pMemory(PageAlloc(uNumPages * s_uPageSize + (uNumPages * s_uPageSize) / s_fDataToMetaRatio)),
		  m_pUserMemory((std::byte *)m_pMemory + (size_t)((uNumPages * s_uPageSize) / s_fDataToMetaRatio)),
		  m_pLast(m_pMemory),
		  m_pCapacity((std::byte *)m_pMemory + (size_t)(uNumPages * s_uPageSize + (uNumPages * s_uPageSize) / s_fDataToMetaRatio)),
		  m_pFreeBlocks((FreeBlock *)m_pMemory)
	{

		assert(m_pMemory && "Arena allocation failed.");
	}

	ArenaAllocator::ArenaAllocator(ArenaAllocator&& other) noexcept
		: m_pMemory(other.m_pMemory),
		m_pUserMemory(other.m_pUserMemory),
		m_pLast(other.m_pLast),
		m_pCapacity(other.m_pCapacity),
		m_pFreeBlocks(other.m_pFreeBlocks)
	{
		other.m_pMemory = nullptr;
		other.m_pUserMemory = nullptr;
		other.m_pLast = nullptr;
		other.m_pCapacity = nullptr;
		other.m_pFreeBlocks = nullptr;
	}

	ArenaAllocator::~ArenaAllocator()
	{
#ifndef GAL_ALLOC_PROTECT
		PageFree(m_pMemory);
#else
		PageProtect(m_pMemory, (std::byte *)m_pCapacity - (std::byte *)m_pMemory);
#endif
	}

	size_t ArenaAllocator::freeBlockCount() const
	{
		return ((FreeBlock *)m_pUserMemory - (FreeBlock *)m_pMemory);
	}

	void *ArenaAllocator::allocate(size_t uSize)
	{
		uSize += 8;
		const size_t uAlignment = 8;
		const size_t uFBcount = freeBlockCount();
		void *retval = nullptr;

		size_t uAlignDistance = ((size_t)m_pLast % uAlignment);

		if (uAlignDistance)
			uAlignDistance = uAlignment - uAlignDistance;

		void *pNewLast = (std::byte *)m_pLast + uAlignDistance;

		if ((std::byte *)m_pCapacity - (std::byte *)pNewLast >= uSize)
		{
			retval = (std::byte *)pNewLast + 8;
			m_pLast = (std::byte *)pNewLast + uSize;
			*(size_t *)pNewLast = uSize - 8;
			return retval;
		}

		for (u32 i = 0; i < uFBcount; i++)
		{

			if (m_pFreeBlocks[i].pAddress && m_pFreeBlocks[i].uSize >= uSize)
			{

				retval = (std::byte *)m_pFreeBlocks + (uAlignment - ((size_t)m_pFreeBlocks[i].pAddress % uAlignment));
				*(size_t *)retval = uSize - 8;
				retval = (std::byte *)retval + 8;
				const size_t newSize = m_pFreeBlocks[i].uSize - (uSize + (uAlignment - ((size_t)m_pFreeBlocks[i].pAddress % uAlignment)));

				if (newSize == 0)
					m_pFreeBlocks[i].pAddress = nullptr;
				else
				{
					m_pFreeBlocks[i].pAddress = (std::byte *)m_pFreeBlocks[i].pAddress + uSize;
					m_pFreeBlocks[i].uSize = newSize;
				}

				return retval;
			}
		}

		return nullptr;
	}

	void ArenaAllocator::deallocate(void *pAddress)
	{

		const size_t uFBcount = freeBlockCount();
		size_t uAllocSize = *((size_t *)pAddress - 1);

		if ((std::byte *)pAddress + uAllocSize == m_pLast)
		{
			m_pLast = (std::byte *)pAddress - 8;
			return;
		}

		for (u32 i = 0; i < uFBcount; i++)
		{
			if (m_pFreeBlocks[i].pAddress == nullptr)
			{
				m_pFreeBlocks[i].pAddress = pAddress;
				m_pFreeBlocks[i].uSize = uAllocSize + 8;
				return;
			}
		}
	}

	void ArenaAllocator::release()
	{

#ifndef GAL_ALLOC_PROTECT
		PageFree(m_pMemory);
		m_pMemory = nullptr;
#else
		PageProtect(m_pMemory, (std::byte *)m_pCapacity - (std::byte *)m_pMemory);
#endif
	}

}