
#include "heyawake.h"

void HYConnectionTree::AddEdge(CellId s, CellId d, int dist)
{
	// printf("%d--%d\n", s, d);
	adj[s].push_back(std::make_pair(d, dist));
	adj[d].push_back(std::make_pair(s, dist));
}

bool HYConnectionTree::Dfs(CellId s, CellId d, CellId rt, int &sol)
{
	if (s == d) {
		sol = 0;
		return true;
	}

	for (auto e : adj[s]) {
		if (e.first == rt) continue;

		if (Dfs(e.first, d, s, sol)) {
			sol += e.second;
			return true;
		}
	}

	return false;
}

int HYConnectionTree::Distance(CellId s, CellId d)
{
	int ret;
	if (Dfs(s, d, -1, ret)) return ret;
	return -1;
}

void HYField::CreateConnectionTree(HYConnectionTree &tree)
{
	// real connection
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			if (CellStatus(i, j) != BLACK) continue;

			if (i == 0 || j == 0 || i == height - 1 || j == width - 1) {
				tree.AddEdge(Id(i, j), aux_cell, 1);
			}

			for (int k = 0; k < 4; ++k) {
				int y2 = i + dy[k] + dy[(k + 1) % 4], x2 = j + dx[k] + dx[(k + 1) % 4];
				if (Range(y2, x2) && CellStatus(y2, x2) == BLACK) tree.AddEdge(Id(i, j), Id(y2, x2), 1);
			}
		}
	}

	// pseudo connection
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			if (1 <= i && i < height - 1 && IsBlackOrOutOfRange(i, j - 1) && IsBlackOrOutOfRange(i, j + 1)) {
				if (CellRoomId(i, j) != CellRoomId(i - 1, j) && CellRoomId(i, j) != CellRoomId(i + 1, j) && CellStatus(i - 1, j) != BLACK && CellStatus(i + 1, j) != BLACK) {
					tree.AddEdge(BlackUnitId(i, j - 1), BlackUnitId(i, j + 1), 40);
				}
			}

			if (1 <= j && j < width - 1 && IsBlackOrOutOfRange(i - 1, j) && IsBlackOrOutOfRange(i + 1, j)) {
				if (CellRoomId(i, j) != CellRoomId(i, j - 1) && CellRoomId(i, j) != CellRoomId(i, j + 1) && CellStatus(i, j - 1) != BLACK && CellStatus(i, j + 1) != BLACK) {
					tree.AddEdge(BlackUnitId(i - 1, j), BlackUnitId(i + 1, j), 40);
				}
			}
		}
	}
}
