// util.h

class TrivialAllocator
{
	char *ptr;

public:
	TrivialAllocator(char *ptr) : ptr(ptr) {}
	char *allocate(int size) {
		char *ret = ptr;
		ptr += size;
		return ret;
	}
};
