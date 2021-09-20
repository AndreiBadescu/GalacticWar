#pragma once
#include "RendererCommon.hpp"
#include <memory>

#define MALLOC(x) malloc(x)
#define FREE(x) free(x)

namespace gal
{

	struct FreeBlock
	{
		void *pAddress;
		size_t uSize;
	};

	/// <summary>
	/// Allocator local, bun pentru functii si obiecte temporare care necesita alocarea pe heap
	/// Nu merge pentru obiecte care se au lifetime-ul mai mare ca acesta
	/// </summary>

	class ArenaAllocator
	{
	public:
		/// <summary>
		/// Aloca 4 pagini de memorie (4KB * 4)
		/// </summary>
		ArenaAllocator();

		/// <summary>
		/// Aloca uNumPages pagini de memorie (4KB * uNumPages)
		/// </summary>
		/// <param name="uNumPages:">Numarul de pagini care vor fi alocate</param>
		ArenaAllocator(size_t uNumPages);

		ArenaAllocator(ArenaAllocator&& other) noexcept;

		template <typename T>
		T *alloc(size_t uCount)
		{
			return (T *)allocate(uCount * sizeof(T));
		}

		/// <summary>
		/// Aloca sSize bytes in arena
		/// </summary>
		/// <param name="uSize:">numarul de bytes alocati in arena</param>
		/// <returns>Pointer catre inceputul memoriei alocate</returns>
		void *allocate(size_t uSize);

		/// <summary>
		/// Dealoca adresa pData si o adauga intr-un freelist
		/// </summary>
		/// <param name="pData:">Pointerul catre blocul de memorie care trebuie dealocat</param>
		void deallocate(void *pData);

		/// <summary>
		/// Dealoca toata arena
		/// Dupa apelarea acestei functii, arena devine inutila (deci release poate fi tratat ca un destructor)
		/// </summary>
		void release();

		inline const void *const GetPMemory() { return m_pMemory; }

		~ArenaAllocator();

	public:
		static void Init();
		static size_t GetPageSize();

	private:
		static constexpr size_t s_fDataToMetaRatio = 100 / 20;
		static size_t s_uPageSize;

	private:
		size_t freeBlockCount() const;

	private:
		void *m_pMemory;
		void *m_pUserMemory;
		void *m_pLast;
		void *m_pCapacity;
		FreeBlock *m_pFreeBlocks;
	};

}
