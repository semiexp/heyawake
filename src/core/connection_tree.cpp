
#include "heyawake.h"

void HYConnectionTree::AddEdge(CellId s, CellId d, int dist)
{
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
