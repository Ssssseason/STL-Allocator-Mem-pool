#pragma once
#include <climits>
class memoryPool {
	typedef size_t size_type;

public:
	static void *allocate(size_type N);
	static void deallocate(void *Ptr, size_type N);

private:
	enum{Align = 8};
	enum{MaxBytes = 4096};
	enum{SecondMaxBytes = 40960};
	enum{NFreeLists = MaxBytes / Align};
	enum{BigFreeLists = (SecondMaxBytes - MaxBytes)/Align};
	enum{NumOfObj = 20};
	enum{Increment = 8};

	#define roundUp(x)	 (((x) + Align - 1) & (~(Align - 1)))

	union Obj {
		Obj * freeListLink;
		char Data;
	};

	static char *startFree;
	static char *endFree;
	static Obj * freeList[NFreeLists];
	static Obj * BFreeList[BigFreeLists];
	static int freeListCnt[NFreeLists];
	static size_type heapSize;

	static inline size_type freeListIndex(size_type N);
	static void *refill(size_type N);
};

// Initialize static class members.
char *memoryPool::startFree = NULL;
char *memoryPool::endFree = NULL;
memoryPool::Obj * memoryPool::freeList[NFreeLists] = { 0 };
memoryPool::Obj * memoryPool::BFreeList[BigFreeLists] = { 0 };
int memoryPool::freeListCnt[NFreeLists] = { 0 };
memoryPool::size_type memoryPool::heapSize = 0;

void *memoryPool::allocate(size_type N) {
	if (N > MaxBytes) {
		if(N > SecondMaxBytes)
		// Space to be applied is big enough, so just directly allocate it.
		return (void *)::operator new(N);
		else {
			Obj * volatile *myBFreeList = BFreeList + freeListIndex(roundUp(N)) - freeListIndex(roundUp(MaxBytes));
			void *result = (void*)*myBFreeList;
			if (result == NULL) {
				// No corresponding middle space left, so we need to ajust memory pool to split space.
				size_type bytesLeft = endFree - startFree;
				if (bytesLeft > roundUp(N)) {
					result = startFree;
					startFree += roundUp(N);
				}
				else {
					result = (void *)::operator new(roundUp(N));
				}
			}
			else {
				// There does exist some corresponding space.
				*myBFreeList = (*myBFreeList)->freeListLink;
			}
			// Return void type pointer pointing corresponging space.
			return result;
		}
	}
	else {
		// Space to be applied is small, so search it in corresponding free linked list.
		Obj * volatile *myFreeList = freeList + freeListIndex(roundUp(N));
		void *result = (void*)*myFreeList;
		if (result == NULL) {
			// No corresponding small space left, so we need to ajust memory pool to split space.
			result = refill(roundUp(N));
		}
		else {
			// There does exist some corresponding space.
			*myFreeList = (*myFreeList)->freeListLink;
		}
		// Return void type pointer pointing corresponging space.
		return result;
	}
}

void memoryPool::deallocate(void *Ptr, size_type N) {
	if (N > MaxBytes) {
		if(N>SecondMaxBytes)
		// Space to be freed is big enough, so just directly free it.
		::operator delete(Ptr);
		else {
			// Space to be freed is middle, so add it back to corresponding free linked list.
			Obj * volatile *myBFreeList = BFreeList + freeListIndex(roundUp(N)) - freeListIndex(roundUp(MaxBytes));
			((Obj*)Ptr)->freeListLink = *myBFreeList;
			*myBFreeList = (Obj*)Ptr;
		}
	}
	else {
		// Space to be freed is small, so add it back to corresponding free linked list.
		Obj * volatile *myFreeList = freeList + freeListIndex(roundUp(N));
		((Obj*)Ptr)->freeListLink = *myFreeList;
		*myFreeList = (Obj*)Ptr;
	}
}

void *memoryPool::refill(size_type N) {
	// Caculate the capacity of space needed to refill lists.
	// Actual number of nodes to be added.
	int *myFreeListCnt = freeListCnt + freeListIndex(N);
	int cnt = *myFreeListCnt;
	int NObj = NumOfObj + Increment << cnt;
	// Update count.
	(*myFreeListCnt) ++;
	size_type totalBytes = N * NObj;
	// Caculate capacity of current memory pool.
	size_type bytesLeft = endFree - startFree;
	char *result;
	if (bytesLeft >= totalBytes) {
		// If current memory pool is big enough, just split a space to refill free lists.
		result = startFree;
		startFree += totalBytes;
	}
	else if (bytesLeft >= N) {
		// If current memory pool is not big enough, ajust the number of nodes to be added.
		result = startFree;
		NObj = bytesLeft / N;
		// Update capacity of space we need.
		totalBytes = NObj * N;
		// Split space.
		startFree += totalBytes;
	}
	else {
		if (bytesLeft > 0) {
			// Add the left space (if has) to free lists to reuse it, since its capacity must be multiple of 8.
			Obj * volatile *myFreeList = freeList + freeListIndex(bytesLeft);
			((Obj *)startFree)->freeListLink = *myFreeList;
			*myFreeList = (Obj *)startFree;
		}
		// Considering the memory already consumed, apply a new memory pool with different capacity.
		size_type newBytes = roundUp(totalBytes * 2 + (heapSize >> 4));
		// Create new memory pool.
		startFree = (char *)::operator new(newBytes);
		endFree = startFree + newBytes;
		// Acculate consumed memory space.
		heapSize += newBytes;
		// Split a space to refill free lists.
		result = startFree;
		startFree += totalBytes;
	}

	// Locate corresponding linked lists.
	Obj * volatile *myFreeList = freeList + freeListIndex(roundUp(N));
	char *currChar;
	// The first N bytes are used to store element in a minute.
	currChar = result + N;
	// Upadate linked lists.
	for (int i = 1; i < NObj; i++) {
		((Obj*)currChar)->freeListLink = *myFreeList;
		*myFreeList = (Obj*)currChar;
		currChar += N;
	}
	return (void*)result;
}

inline memoryPool::size_type memoryPool::freeListIndex(size_type N) {
	// Get index of corresponding free linked list.
	return (N - 1) / Align;
}

template <typename T>
class myAllocator {
public:
	typedef void _Not_user_specialized;
	typedef T value_type;
	typedef value_type *pointer;
	typedef const value_type *const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef ::std::true_type propagate_on_container_move_assignment;
	typedef ::std::true_type is_always_equal;

	pointer address(reference V) const noexcept {
		return &V;
	}

	const_pointer address(const_reference V) const noexcept {
		return &V;
	}

	template<typename Other>
	struct rebind {
		typedef myAllocator<Other> other;
	};

	myAllocator() noexcept {}

	myAllocator(const myAllocator<T> &) noexcept {}

	template<typename Other>
	myAllocator(const myAllocator<Other> &) noexcept {}

	template<typename Other>
	myAllocator<T> & operator=(const myAllocator<Other> &) {
		return (*this);
	}

	template<typename Objty, typename... Types>
	void construct(Objty *Ptr, Types&&... Args) {
		::new ((void *)Ptr) Objty(::std::forward<Types>(Args)...);
	}

	template<typename U>
	void destroy(U *Ptr) {
		Ptr->~U();
	}

	size_t maxSize() const noexcept {
		return UINT_MAX / sizeof(T);
	}

	void deallocate(pointer Ptr, size_type Count) {
		size_type Size = Count * sizeof(T);
		memoryPool::deallocate(Ptr, Size);
	}

	_DECLSPEC_ALLOCATOR pointer allocate(size_type Count) {
		void *Ptr = NULL;
		size_type Size = Count * sizeof(T);
		if (Count) {
			Ptr = memoryPool::allocate(Size);
		}
		return static_cast<pointer>(Ptr);
	}

	_DECLSPEC_ALLOCATOR pointer allocate(size_type Count, const void*) {
		return allocate(Count);
	}
};
