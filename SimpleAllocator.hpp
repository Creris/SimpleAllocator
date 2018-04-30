#ifndef HEADER_SIMPLE_ALLOCATOR_HPP
#define HEADER_SIMPLE_ALLOCATOR_HPP

#include <vector>
#include <iostream>
#include <cassert>
#include <string>
#include <memory>

namespace alloc {
	template <class T, int BucketSize = 64 * 1024 * 1024>
	class SimpleAllocatorSized {
		std::vector<std::vector<char>> bucketList;
		std::vector<T*> deallocated;

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
	public:
		using byte_t = char;

		SimpleAllocatorSized() : bucketList(1) {
			bucketList.back().reserve(BucketSize);
		}

		byte_t* getMemory() {
			if (deallocated.size()) {
				byte_t* byte = (byte_t*)deallocated.back();
				deallocated.pop_back();
				return byte;
			}

			static auto findBucket = [](int allocSize, auto& bucketList) -> auto& {
				for (auto& bucket : bucketList) {
					if (bucket.size() + allocSize <= bucket.capacity())
						return bucket;
				}

				auto& bucket = bucketList.emplace_back();
				bucket.reserve(BucketSize);
				return bucket;
			};

			constexpr size_t size = sizeof(T);
			auto& fittingBucket = findBucket(size, bucketList);

			size_t position = fittingBucket.size();
			fittingBucket.resize(position + size);

			return &fittingBucket[position];
		}

		T* allocate() {
			return new (getMemory()) T();
		}

		template <class... Args>
		T* allocate(Args&&... constructorArgs) {
			return new (getMemory()) T(constructorArgs...);
		}

		void deallocate(T* ptr) {
#if _DEBUG
			assert(isWithinFromSelf((char*)(ptr)));
#endif
			deallocated.push_back(ptr);
			ptr->~T();
		}
	};

	template <class T>
	class SmartSimpleAllocator {
		SimpleAllocatorSized<T> innerAllocator;

		auto toUniquePointer(T* ptr) {
			static auto lambda = [&](T* t) { innerAllocator.deallocate(t); };
			return std::unique_ptr<T, decltype(lambda)>{ ptr, lambda };
		}

		auto toSharedPointer(T* ptr) {
			static auto lambda = [&](T* t) { innerAllocator.deallocate(t); };
			return std::shared_ptr<T>{ ptr, lambda };
		}
	public:
		template <class... Args>
		auto allocate(Args&&... constructorArgs) {
			return toUniquePointer(innerAllocator.allocate(std::forward<Args>(constructorArgs)...));
		}

		template <class... Args>
		auto allocateShared(Args&&... constructorArgs) {
			return toSharedPointer(innerAllocator.allocate(std::forward<Args>(constructorArgs)...));
		}
	};

	template <class T>
	using SimpleAllocator = SimpleAllocatorSized<T>;

}

#endif	//HEADER_SIMPLE_ALLOCATOR_HPP