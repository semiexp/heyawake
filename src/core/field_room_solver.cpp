
#include "heyawake.h"

#include <algorithm>

HYField::Status HYField::SolveVirtualRoomWithDatabase(int top_y, int top_x, int end_y, int end_x, int hint)
{
	int room_h = end_y - top_y, room_w = end_x - top_x;
	if (!HYRoomDatabase::IsAvailable(room_h, room_w, hint)) return status;

	int black = 0, white = 0;

	for (int i = 0; i < room_h; ++i) {
		for (int j = 0; j < room_w; ++j) {
			HYField::Status c_stat = CellStatus(top_y + i, top_x + j);

			if (c_stat == BLACK) black |= 1 << (i * room_w + j);
			if (c_stat == WHITE) white |= 1 << (i * room_w + j);
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

	if (n_cand > 10) return status;

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

	for (int i = 0; i < room_h; i++) {
		for (int j = 0; j < room_w; j++) {
			if (tb_black & (1 << (i * room_w + j))) DetermineBlack(top_y + i, top_x + j);
			if (tb_white & (1 << (i * room_w + j))) DetermineWhite(top_y + i, top_x + j);
		}
	}
	
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

	return status;
}

HYField::Status HYField::SolveRoom(RoomId rid)
{
	Room &room = rooms[rid];
	if (room.hint == -1) return NORMAL;

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

	SolveRoom(rid);

	return status;
}
