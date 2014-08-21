
#include "heyawake.h"

std::vector<std::vector<int> > HYRoomDatabase::room;
int HYRoomDatabase::index[10][10][10];

struct union_find
{
	int *val;
	int N;

	union_find(int N) : N(N) { val = new int[N]; for (int i = 0; i < N; i++) val[i] = -1; }
	~union_find() { delete[] val; }

	int root(int p) { return val[p] < 0 ? p : (val[p] = root(val[p])); }
	bool join(int p, int q) {
		p = root(p); q = root(q);
		if (p == q) return true;
		val[p] += val[q];
		val[q] = p;
		return false;
	}
};

void HYRoomDatabase::Visit(int y, int x, int state, int hint, int height, int width, std::vector<int> &sto)
{
	if (y == height) {
		if (hint > 0) return;

		union_find uf(height * width);

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				if (!(state & (1 << (i * width + j)))) continue;

				if (i > 0) {
					if (j > 0) {
						if (state & (1 << ((i - 1) * width + (j - 1))))
							if (uf.join(i * width + j, (i - 1) * width + (j - 1))) return;
					}
					if (j < width - 1) {
						if (state & (1 << ((i - 1) * width + (j + 1))))
							if (uf.join(i * width + j, (i - 1) * width + (j + 1))) return;
					}
				}
			}
		}
		sto.push_back(state);
		return;
	}

	if (x == width) {
		Visit(y + 1, 0, state, hint, height, width, sto);
		return;
	}

	bool allowed_black = (hint > 0);
	if (x > 0) allowed_black &= !(state & (1 << (y * width + (x - 1))));
	if (y > 0) allowed_black &= !(state & (1 << ((y - 1) * width + x)));

	if (allowed_black) {
		Visit(y, x + 1, state | (1 << (y * width + x)), hint - 1, height, width, sto);
	}

	Visit(y, x + 1, state, hint, height, width, sto);
}

void HYRoomDatabase::PreCalc(int height, int width, int hint)
{
	std::vector<int> ret;

	Visit(0, 0, 0, hint, height, width, ret);

	index[height][width][hint] = room.size();
	printf("%d %d %d: %d\n", height, width, hint, ret.size());
	room.push_back(ret);

	if (height < width) PreCalc(width, height, hint);
}

void HYRoomDatabase::Initialize()
{
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 10; ++j) {
			for (int k = 0; k < 10; ++k) {
				index[i][j][k] = -1;
			}
		}
	}

	PreCalc(1, 2, 1);
	PreCalc(1, 3, 1);
	PreCalc(1, 3, 2);
	PreCalc(1, 4, 1);
	PreCalc(1, 4, 2);
	PreCalc(1, 5, 1);
	PreCalc(1, 5, 2);
	PreCalc(1, 5, 3);
	PreCalc(2, 2, 1);
	PreCalc(2, 2, 2);
	PreCalc(2, 3, 1);
	PreCalc(2, 3, 2);
	PreCalc(2, 3, 3);
	PreCalc(2, 4, 1);
	PreCalc(2, 4, 2);
	PreCalc(2, 4, 3);
	PreCalc(2, 4, 4);
	PreCalc(3, 3, 1);
	PreCalc(3, 3, 2);
	PreCalc(3, 3, 3);
	PreCalc(3, 3, 4);
	PreCalc(3, 3, 5);
	PreCalc(3, 4, 5);
	PreCalc(3, 5, 7);
	PreCalc(4, 4, 7);
}
