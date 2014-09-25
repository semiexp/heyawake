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

class UnionFind
{
	int *rt;

public:
	UnionFind() : rt(nullptr) {}
	UnionFind(int N, int* rt) : rt(rt) {
		std::fill(rt, rt + N, -1);
	}

	int root(int p) { return rt[p] < 0 ? p : (rt[p] = root(rt[p])); }
	void join(int p, int q) {
		p = root(p); q = root(q);
		if (p == q) return;

		rt[p] += rt[q];
		rt[q] = p;
	}
};
