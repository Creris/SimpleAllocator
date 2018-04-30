#include "SimpleAllocator.hpp"

class A {
public:
	~A() {
		std::cout << "Destroying instance of A\n";
	}
};

int main() {
	{
		alloc::SmartSimpleAllocator<A> smartStringAllocator;
		auto ptr = smartStringAllocator.allocate();
		{
			auto ptr2 = smartStringAllocator.allocate();
			auto ptr3 = smartStringAllocator.allocateShared();
		}
	}

	{
		alloc::SimpleAllocator<std::string> stringAllocator;
		std::vector<std::string*> strings;

		for (int i = 0; i < 10; ++i) {
			strings.push_back(stringAllocator.allocate(std::to_string(i)));
		}

		stringAllocator.deallocate(strings.back());
		strings.pop_back();

		std::string* s = stringAllocator.allocate();
		(void)s;	//THIS LEAKS
	}
}