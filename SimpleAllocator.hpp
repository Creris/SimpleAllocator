#ifndef HEADER_SIMPLE_ALLOCATOR_HPP
#define HEADER_SIMPLE_ALLOCATOR_HPP

#include <vector>
#include <iostream>
#include <cassert>
#include <string>
#include <memory>

namespace alloc {
	template <class T, int BucketSize = 64 * 1024>
	class SimpleAllocatorSized {
		std::vector<std::vector<char>> bucketList;
		std::vector<T*> deallocated;
		bool autoFree = false;

		bool isWithinFromSelf(char* ptr) {
			static auto isWithin = [](auto ptr, auto check, int size) {
				char* vPtr = (char*)ptr;
				char* vCheck = (char*)check;

				return vPtr >= vCheck && vPtr < vCheck + size;
			};

			for (auto& bucket : bucketList) {
				if (!bucket.size())
					continue;

				if (isWithin(ptr, &bucket[0], bucket.size()))
					return true;
			}

			return false;
		}

		auto& addBucket() {
			auto& fittingBucket = bucketList.emplace_back();
			fittingBucket.reserve(BucketSize);
			return fittingBucket;
		}
	public:
		using byte_t = char;

		SimpleAllocatorSized() : bucketList(1) {
			bucketList.back().reserve(BucketSize);
		}

		byte_t* getMemory() {
			if (deallocated.size()) {
				byte_t* byte = (byte_t*)deallocated.back();
				deallocated.pop_back();
				if (deallocated.size() * 2 <= deallocated.capacity() && autoFree)
					deallocated.shrink_to_fit();

				return byte;
			}

			constexpr size_t size = sizeof(T);
			auto& fittingBucket = bucketList.back();

			if (fittingBucket.size() + size > fittingBucket.capacity())
				fittingBucket = addBucket();

			size_t position = fittingBucket.size();
			fittingBucket.resize(position + size);
			return &fittingBucket[position];
		}

		template <class... Args>
		T* allocate(Args&&... constructorArgs) {
			return new (getMemory()) T(std::forward<Args>(constructorArgs)...);
		}

		void deallocate(T* ptr) {
#if _DEBUG
			assert(isWithinFromSelf((char*)(ptr)));
#endif
			deallocated.push_back(ptr);
			ptr->~T();
		}

		void freeUnusedMemory() {
			deallocated.shrink_to_fit();
		}

		void setAutoFree(bool set) {
			autoFree = set;
		}
	};

	template <class T, int BucketSize = 64 * 1024>
	class SmartSimpleAllocatorSized {
		using innerAllocator_t = SimpleAllocatorSized<T, BucketSize>;
		innerAllocator_t innerAllocator;

		template <class Allocator>
		static auto toUniquePointer(T* ptr, Allocator& innerAlloc) {
			auto lambda = [&](T* t) { innerAlloc.deallocate(t); };
			return std::unique_ptr<T, decltype(lambda)>{ ptr, lambda };
		}

		template <class Allocator>
		static auto toSharedPointer(T* ptr, Allocator& innerAlloc) {
			auto lambda = [&](T* t) { innerAlloc.deallocate(t); };
			return std::shared_ptr<T>{ ptr, lambda };
		}
	public:
		using unique_t = decltype(toUniquePointer(std::declval<T*>(), std::declval<innerAllocator_t&>()));
		using shared_t = std::shared_ptr<T>;

		template <class... Args>
		auto allocate(Args&&... constructorArgs) {
			return toUniquePointer(innerAllocator.allocate(std::forward<Args>(constructorArgs)...), innerAllocator);
		}

		template <class... Args>
		auto allocateShared(Args&&... constructorArgs) {
			return toSharedPointer(innerAllocator.allocate(std::forward<Args>(constructorArgs)...), innerAllocator);
		}
	};

	template <class T>
	using SmartSimpleAllocator = SmartSimpleAllocatorSized<T>;

	template <class T>
	using SimpleAllocator = SimpleAllocatorSized<T>;

}

#endif	//HEADER_SIMPLE_ALLOCATOR_HPP