
#include "heyawake.h"

// the size of space should be at least ((height * width + 1) * sizeof(CellId))
HYConnectionManager::HYConnectionManager(CellCord height, CellCord width, CellId *space)
	: height(height), width(width), unit_root(space) {
	aux = height * width;
	for (int i = 0; i <= height * width; ++i) space[i] = -1;
}

void HYConnectionManager::Join(CellId p, CellId q)
{
	p = Root(p); q = Root(q);

	if (p == q) return;

	unit_root[p] += unit_root[q];
	unit_root[q] = p;
}

bool HYConnectionManager::CheckValidity(CellCord y, CellCord x)
{
	int ids[5];

	for (int i = 0; i < 4; i++) {
		int y2 = y + HYField::dy[i] + HYField::dy[(i + 3) % 4], x2 = x + HYField::dx[i] + HYField::dx[(i + 3) % 4];

		ids[i] = Root(Range(y2, x2) ? Id(y2, x2) : aux);
	}

	ids[4] = ids[0];

	for (int i = 0; i < 4; i++) {
		int y2 = y + HYField::dy[i], x2 = x + HYField::dx[i];

		if (Range(y2, x2)) {
			if (ids[i] == ids[i + 1]) return false;
		}
	}

	if (0 < y && y < height - 1 && 0 < x && x < width - 1) {
		if (Root(ids[0]) == Root(ids[2])) return false;
		if (Root(ids[1]) == Root(ids[3])) return false;
	}

	return true;
}

int HYConnectionManager::CheckCost(CellCord y, CellCord x, HYConnectionTree &tree)
{
	int ids[5];
	int locs[5];
	int ret = 1 << 30;

	for (int i = 0; i < 4; i++) {
		int y2 = y + HYField::dy[i] + HYField::dy[(i + 3) % 4], x2 = x + HYField::dx[i] + HYField::dx[(i + 3) % 4];

		locs[i] = Range(y2, x2) ? Id(y2, x2) : aux;
		ids[i] = Root(Range(y2, x2) ? Id(y2, x2) : aux);
	}

	ids[4] = ids[0];
	locs[4] = locs[0];

	for (int i = 0; i < 4; i++) {
		int y2 = y + HYField::dy[i], x2 = x + HYField::dx[i];

		if (Range(y2, x2)) {
			if (ids[i] == ids[i + 1]) {
				int tmp = tree.Distance(locs[i], locs[i+1]);

				if (ret > tmp) ret = tmp;
			}
		}
	}

	if (0 < y && y < height - 1 && 0 < x && x < width - 1) {
		if (Root(ids[0]) == Root(ids[2])) {
			int tmp = tree.Distance(locs[0], locs[2]);

			if (ret > tmp) ret = tmp;
		}

		if (Root(ids[1]) == Root(ids[3])) {
			int tmp = tree.Distance(locs[1], locs[3]);

			if (ret > tmp) ret = tmp;
		}
	}

	return ret;
}
