
#include "heyawake.h"
#include "../util/util.h"

#include <algorithm>
#include <memory>

std::pair<int, int> HYField::SolveRegionSub(int top_y, int top_x, int end_y, int end_x, int hint, int tb_det)
{
	int room_h = end_y - top_y, room_w = end_x - top_x;

	if (hint >= 0 && HYRoomDatabase::Limit(room_h, room_w) < hint) return std::make_pair(tb_det, tb_det);
	if (!HYRoomDatabase::IsAvailable(room_h, room_w, hint)) return std::make_pair(0, 0);

	int black = 0, white = ((1 << (room_w * room_h)) - 1) ^ tb_det;

	for (int i = 0; i < room_h; ++i) {
		for (int j = 0; j < room_w; ++j) {
			HYField::Status c_stat = CellStatus(top_y + i, top_x + j);

			if (tb_det & (1 << (i * room_w + j))) {
				if (c_stat == BLACK) black |= 1 << (i * room_w + j);
				if (c_stat == WHITE) white |= 1 << (i * room_w + j);
			}
		}
	}

	std::vector<int> &db = HYRoomDatabase::Fetch(room_h, room_w, hint);
	auto &det = HYRoomDatabase::FetchDetail(room_h, room_w, hint);

	int tb_black = (1 << (room_h * room_w)) - 1;
	int tb_white = tb_black;

	int n_cand = 0;

	for (int i = 0; i < db.size(); i++) {
		int bit = db[i];

		// simple check
		if (!((black & ~bit) == 0 && (white & bit) == 0)) continue;
		++n_cand;
	}

	bool mode = true;
	for (int i = top_y; i < end_y; ++i) {
		for (int j = top_x; j < end_x; ++j) {
			if (rel_pseudo_con[Id(i, j)]) mode = false;
		}
	}

	int r_aux = mode ? PseudoRoot(aux_cell) : Root(aux_cell);

	for (int i = 0; i < db.size(); i++) {
		int bit = db[i];

		// simple check
		if (!((black & ~bit) == 0 && (white & bit) == 0)) continue;

		int n_aux = 0, repr = -1;
		std::vector<int> adjs;

		for (int pt : det[i]) {
			bool end_flg = pt < 0;
			if (pt < 0) pt = ~pt;

			int y = top_y + pt / room_w, x = top_x + pt % room_w;

			bool aux_flg = false;

			if (CellStatus(y, x) != BLACK) { // TODO
				for (int j = 0; j < 4; j++) {
					int y2 = y + dy[j] + dy[(j + 1) % 4], x2 = x + dx[j] + dx[(j + 1) % 4];
					int bid = BlackUnitId(y2, x2); if (bid == -1) continue;
					bid = mode ? PseudoRoot(bid) : Root(bid);

					if (Range(y2, x2) && top_y <= y2 && y2 < end_y && top_x <= x2 && x2 < end_x) continue;
					if (bid == r_aux) aux_flg = true;
					else adjs.push_back(bid);
				}

				if (aux_flg) ++n_aux;
			}

			if (CellStatus(y, x) == BLACK) {
				repr = mode ? PseudoRoot(y * width + x) : Root(y * width + x);
			}

			if (end_flg) {
				if (n_aux >= 2) goto next;

				std::sort(adjs.begin(), adjs.end());

				if (adjs.size() > 0 && adjs[0] == repr) goto next;
				for (int j = 1; j < adjs.size(); j++) if (adjs[j] == repr || adjs[j] == adjs[j - 1]) goto next;

				n_aux = 0;
				repr = -1;
				adjs.clear();
			}
		}

		// update
		tb_black &= bit;
		tb_white &= ~bit;

		if (tb_black == 0 && tb_white == 0) break;
	next:
		continue;
	}

	tb_black &= tb_det;
	tb_white &= tb_det;

	return std::make_pair(tb_black, tb_white);
}

HYField::Status HYField::SolveVirtualRoomWithDatabase(int top_y, int top_x, int end_y, int end_x, int hint)
{
	int room_h = end_y - top_y, room_w = end_x - top_x;

	auto tmp = SolveRegionSub(top_y, top_x, end_y, end_x, hint, (1 << (room_h * room_w)) - 1);
	int tb_black = tmp.first, tb_white = tmp.second;

	for (int i = 0; i < room_h; i++) {
		for (int j = 0; j < room_w; j++) {
			if (tb_black & (1 << (i * room_w + j))) DetermineBlack(top_y + i, top_x + j);
			if (tb_white & (1 << (i * room_w + j))) DetermineWhite(top_y + i, top_x + j);
		}
	}
	
	return status;
}

int HYField::MaximumBlackCells(int top_y, int top_x, int end_y, int end_x, int tb_det)
{
	int room_h = end_y - top_y, room_w = end_x - top_x;
	int ret = HYRoomDatabase::Limit(room_h, room_w);

	while (ret > 0) {
		auto tmp = SolveRegionSub(top_y, top_x, end_y, end_x, ret, tb_det);

		if (tmp.first == tb_det && tmp.second == tb_det) {
			--ret;
			continue;
		}

		break;
	}

	return ret;
}

int HYField::MaximumBlackCells(std::vector<int> &ys, std::vector<int> &xs)
{
	int top_y = height, top_x = width, end_y = -1, end_x = -1;

	for (int y : ys) {
		if (y < top_y) top_y = y;
		if (end_y < y) end_y = y;
	}

	for (int x : xs) {
		if (x < top_x) top_x = x;
		if (end_x < x) end_x = x;
	}

	++end_y; ++end_x;

	int room_h = end_y - top_y, room_w = end_x - top_x;
	int tb_det = 0;

	for (int i = 0; i < ys.size(); ++i) {
		tb_det |= 1 << ((ys[i] - top_y) * room_w + (xs[i] - top_x));
	}

	return MaximumBlackCells(top_y, top_x, end_y, end_x, tb_det);
}

HYField::Status HYField::SolveSkewRoom(std::vector<int> &ys, std::vector<int> &xs, int hint)
{
	int top_y = height, top_x = width, end_y = -1, end_x = -1;

	for (int y : ys) {
		if (y < top_y) top_y = y;
		if (end_y < y) end_y = y;
	}

	for (int x : xs) {
		if (x < top_x) top_x = x;
		if (end_x < x) end_x = x;
	}

	++end_y; ++end_x;

	int room_h = end_y - top_y, room_w = end_x - top_x;
	int tb_det = 0;

	for (int i = 0; i < ys.size(); ++i) {
		tb_det |= 1 << ((ys[i] - top_y) * room_w + (xs[i] - top_x));
	}

	auto tmp = SolveRegionSub(top_y, top_x, end_y, end_x, hint, tb_det);
	int tb_black = tmp.first, tb_white = tmp.second;

	for (int i = 0; i < room_h; i++) {
		for (int j = 0; j < room_w; j++) {
			if (tb_black & (1 << (i * room_w + j))) DetermineBlack(top_y + i, top_x + j);
			if (tb_white & (1 << (i * room_w + j))) DetermineWhite(top_y + i, top_x + j);
		}
	}

	return status;
}

HYField::Status HYField::SeparateRoom(int top_y, int top_x, int end_y, int end_x, int hint)
{
	int room_h = end_y - top_y, room_w = end_x - top_x;

	std::unique_ptr<int[]> uft(new int[room_h * room_w]);
	UnionFind uf(room_h * room_w, uft.get());

	for (int i = 0; i < room_h; ++i) {
		for (int j = 0; j < room_w; ++j) {
			if (CellStatus(i + top_y, j + top_x) == UNDECIDED) {
				if (i + 1 < room_h && CellStatus(i + 1 + top_y, j + top_x) == UNDECIDED) {
					uf.join((i + 1) * room_w + j, i * room_w + j);
				}
				if (j + 1 < room_w && CellStatus(i + top_y, j + 1 + top_x) == UNDECIDED) {
					uf.join(i * room_w + j, i * room_w + (j + 1));
				}
			} else if (CellStatus(i + top_y, j + top_x) == BLACK) {
				--hint;
			}
		}
	}

	std::vector<std::vector<int> > yss, xss;
	std::vector<int> maxs;

	for (int i = 0; i < room_h * room_w; ++i) {
		if (uf.root(i) == i && CellStatus(top_y + i / room_w, top_x + i % room_w) == UNDECIDED) {
			std::vector<int> ys, xs;

			for (int j = 0; j < room_h * room_w; ++j) {
				if (uf.root(j) == i && CellStatus(top_y + j / room_w, top_x + j % room_w) == UNDECIDED) {
					ys.push_back(top_y + j / room_w);
					xs.push_back(top_x + j % room_w);
				}
			}

			yss.push_back(ys);
			xss.push_back(xs);
		}
	}

	int max_black = 0;
	for (int i = 0; i < yss.size(); ++i) {
		int tmp = MaximumBlackCells(yss[i], xss[i]);
		max_black += MaximumBlackCells(yss[i], xss[i]);
		maxs.push_back(tmp);
	}

	if (max_black != hint) return status;

	for (int i = 0; i < yss.size(); ++i) SolveSkewRoom(yss[i], xss[i], maxs[i]);

	return status;
}

HYField::Status HYField::SolveVirtualRoom(int top_y, int top_x, int end_y, int end_x, int hint)
{
	SolveVirtualRoomWithDatabase(top_y, top_x, end_y, end_x, hint);

	CellId n_black = 0, n_undecided = 0;

	for (int i = top_y; i < end_y; ++i) {
		for (int j = top_x; j < end_x; ++j) {
			if (CellStatus(i, j) == BLACK) ++n_black;
			if (CellStatus(i, j) == UNDECIDED) ++n_undecided;
		}
	}

	if (n_black == hint) {
		for (int i = top_y; i < end_y; ++i) {
			for (int j = top_x; j < end_x; ++j) {
				if (CellStatus(i, j) == UNDECIDED) DetermineWhite(i, j);
			}
		}
	}

	if (hint - n_black > n_undecided) status |= INCONSISTENT;

	if (hint - n_black == n_undecided) {
		for (int i = top_y; i < end_y; ++i) {
			for (int j = top_x; j < end_x; ++j) {
				if (CellStatus(i, j) == UNDECIDED) {
					DetermineBlack(i, j);
				}
			}
		}
	}

	if (method.separate_room) SeparateRoom(top_y, top_x, end_y, end_x, hint);

	return status;
}

HYField::Status HYField::WhiteRestriction(RoomId rid)
{
	Room &room = rooms[rid];
	if (room.hint == -1) return status;
	
	if (0 < room.top_y && room.end_y < height) {
		int res = 0, n_black = 0;

		for (int j = room.top_x; j < room.end_x; ++j) {
			int row_black = 0;
			for (int i = room.top_y; i < room.end_y; ++i) {
				if (CellStatus(i, j) == BLACK) ++row_black;
			}

			if (row_black > 0) n_black += row_black;
			else if (CellStatus(room.top_y - 1, j) == WHITE && CellStatus(room.end_y, j) == WHITE) ++res;
		}

		if (res + n_black > room.hint) return status |= INCONSISTENT;
		if (res + n_black == room.hint) {
			std::vector<std::pair<int, int> > cand;
			for (int j = room.top_x; j < room.end_x; ++j) {
				bool flg = CellStatus(room.top_y - 1, j) == WHITE && CellStatus(room.end_y, j) == WHITE;

				for (int i = room.top_y; i < room.end_y; ++i) {
					if (CellStatus(i, j) == BLACK) flg = false;
				}

				if (!flg) {
					for (int i = room.top_y; i < room.end_y; ++i) {
						if (CellStatus(i, j) == UNDECIDED) cand.push_back(std::make_pair(i, j));
					}
				}
			}

			for (auto c : cand) DetermineWhite(c.first, c.second);
		}
	}

	if (0 < room.top_x && room.end_x < width) {
		int res = 0, n_black = 0;

		for (int i = room.top_y; i < room.end_y; ++i) {
			int row_black = 0;
			for (int j = room.top_x; j < room.end_x; ++j) {
				if (CellStatus(i, j) == BLACK) ++row_black;
			}

			if (row_black > 0) n_black += row_black;
			else if (CellStatus(i, room.top_x - 1) == WHITE && CellStatus(i, room.end_x) == WHITE) ++res;
		}

		if (res + n_black > room.hint) return status |= INCONSISTENT;
		if (res + n_black == room.hint) {
			std::vector<std::pair<int, int> > cand;
			for (int i = room.top_y; i < room.end_y; ++i) {
				bool flg = CellStatus(i, room.top_x - 1) == WHITE && CellStatus(i, room.end_x) == WHITE;

				for (int j = room.top_x; j < room.end_x; ++j) {
					if (CellStatus(i, j) == BLACK) flg = false;
				}

				if (!flg) {
					for (int j = room.top_x; j < room.end_x; ++j) {
						if (CellStatus(i, j) == UNDECIDED) cand.push_back(std::make_pair(i, j));
					}
				}
			}

			for (auto c : cand) DetermineWhite(c.first, c.second);
		}
	}

	return status;
}

HYField::Status HYField::SolveRoom(RoomId rid)
{
	Room &room = rooms[rid];
	if (room.hint == -1) return status;

	if (!method.virtual_room) return SolveVirtualRoom(room.top_y, room.top_x, room.end_y, room.end_x, room.hint);

	CellCord top_y = 127, top_x = 127, end_y = -1, end_x = -1;
	CellId rem_hint = room.hint;

	for (int i = room.top_y; i < room.end_y; ++i) {
		for (int j = room.top_x; j < room.end_x; ++j) {
			if (CellStatus(i, j) == UNDECIDED) {
				if (i < top_y) top_y = i;
				if (end_y < i) end_y = i;
				if (j < top_x) top_x = j;
				if (end_x < j) end_x = j;
			} else if (CellStatus(i, j) == BLACK) { --rem_hint; }
		}
	}

	++end_y; ++end_x;

	if (top_y >= end_y || top_x >= end_x) {
		return status |= (rem_hint == 0) ? NORMAL : INCONSISTENT;
	}

	for (int i = top_y; i < end_y; ++i) {
		for (int j = top_x; j < end_x; ++j) {
			if (CellStatus(i, j) == BLACK) ++rem_hint;
		}
	}

	WhiteRestriction(rid);

	return SolveVirtualRoom(top_y, top_x, end_y, end_x, rem_hint);
}

HYField::Status HYField::SolveTrivialRoom(RoomId rid)
{
	Room &room = rooms[rid];

	int room_h = room.end_y - room.top_y, room_w = room.end_x - room.top_x;

	if (room.hint == 0) {
		// hint == 0 (trivial)
		for (int i = room.top_y; i < room.end_y; i++) {
			for (int j = room.top_x; j < room.end_x; j++) {
				DetermineWhite(i, j);
			}
		}
	}

	if (room_h == 1 && room_w == room.hint * 2 - 1) {
		for (int i = 0; i < room_w; i++) {
			if (i % 2 == 0) DetermineBlack(room.top_y, room.top_x + i);
			else DetermineWhite(room.top_y, room.top_x + i);
		}
	}

	if (room_w == 1 && room_h == room.hint * 2 - 1) {
		for (int i = 0; i < room_h; i++) {
			if (i % 2 == 0) DetermineBlack(room.top_y + i, room.top_x);
			else DetermineWhite(room.top_y + i, room.top_x);
		}
	}

	if (room_h == 3 && room_w == 3 && room.hint == 5) {
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				if ((i + j) % 2 == 0) DetermineBlack(room.top_y + i, room.top_x + j);
				else DetermineWhite(room.top_y + i, room.top_x + j);
			}
		}
	}

	if (room_h == 3 && room_w == 2 && room.hint == 3) {
		if (Range(room.top_y + 1, room.top_x - 1)) DetermineWhite(room.top_y + 1, room.top_x - 1);
		if (Range(room.top_y + 1, room.top_x + 2)) DetermineWhite(room.top_y + 1, room.top_x + 2);
	}

	if (room_h == 2 && room_w == 3 && room.hint == 3) {
		if (Range(room.top_y - 1, room.top_x + 1)) DetermineWhite(room.top_y - 1, room.top_x + 1);
		if (Range(room.top_y + 2, room.top_x + 1)) DetermineWhite(room.top_y + 2, room.top_x + 1);
	}

	SolveRoom(rid);

	return status;
}
