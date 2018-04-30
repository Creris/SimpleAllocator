# SimpleAllocator

This repository contains a library with an implementation of simple type aware allocator in C++.

- [SimpleAllocator](#simpleallocator)
    - [API](#api)
        - [`alloc::SimpleAllocatorSized<T, BucketSize>`](#alloc--simpleallocatorsizedt--bucketsize)
        - [`alloc::SmartSimpleAllocatorSized<T, BucketSize>`](#alloc--smartsimpleallocatorsizedt--bucketsize)
        - [Full listing of declarations in this library](#full-listing-of-declarations-in-this-library)
    - [Requirements](#requirements)
    - [Tested compilers](#tested-compilers)
    - [Issues](#issues)

## API

### `alloc::SimpleAllocatorSized<T, BucketSize>`

`using byte_t = char`

Provides a typedef for the raw type of bytes that this allocator works with.

`SimpleAllocatorSized()`

Constructs new instance of allocator. Always has 1 bucket ready to be used.

`byte_t* getMemory()`

Returns pointer to raw storage allocated by this allocator without initializing an instance of T inside of it. Always allocates enough bytes to store instance of type T.

`T* allocate(Args&&... constructorArgs)`

Allocates and constructs new instance of type T on heap.
The arguments passed to this function are perfectly forwarded into constructor of type T.

`void deallocate(T* ptr)`

Destructs instance of type T pointed at by ptr and returns the memory pointed at by ptr to the allocator to be reused by another call to function allocate.

### `alloc::SmartSimpleAllocatorSized<T, BucketSize>`

`std::unique_ptr<T,/* unspecified */> allocate(Args&&... constrArgs)`

Constructs new instance of type T with perfectly forwarded arguments and stores it inside unique_ptr. This pointer will call deallocate on stored pointer once its lifetime ends.

The instance of this object must outlive all unique pointers returned by this function.

`std::shared_ptr<T> allocateShared(Args&&... constrArgs)`

Constructs new instance of type T with perfectly forwarded arguments and stores it inside shared_ptr. This pointer will call deallocate on stored pointer once the last shared pointer owning this pointer ends its lifetime.

The instance of this object must outlive all shared pointers returned by this function.

### Full listing of declarations in this library

```c++
namespace alloc{
    template <class T, int BucketSize = 64 * 1024 * 1024>
    class SimpleAllocatorSized{
    public:
        using byte_t = char;

        SimpleAllocatorSized();

        byte_t* getMemory();

        template <class... Args>
        T* allocate(Args&&... constrArgs);

        void deallocate(T* ptr);
    };

    template <class T, int BucketSize = 64 * 1024 * 1024>
    class SmartSimpleAllocatorSized{
    public:
        template <class... Args>
        std::unique_ptr<T,/* unspecified */> allocate(Args&&... constrArgs);

        template <class... Args>
        std::shared_ptr<T> allocateShared(Args&&... constrArgs);
    };

    template <class T>
    using SmartSimpleAllocator = SmartSimpleAllocatorSized<T>;

    template <class T>
    using SimpleAllocator = SimpleAllocatorSized<T>;
}
```

## Requirements

This library requires a compiler that supports C++17. The reason being that this library relies on C++17 change to vector's emplace_back returning reference to emplaced value.

## Tested compilers

Currently, the only tested compiler that this implementation compiles and runs on flawlesly is Visual Studio version 15.5.2.

## Issues

If you find any issues, feel free to open new issue on Github page for this project.