
#include "heyawake.h"

#include <algorithm>

HYField::Status HYField::SolveRoomWithDatabase(RoomId rid)
{
	Room &room = rooms[rid];
	Status ret = NORMAL;

	int room_h = room.end_y - room.top_y, room_w = room.end_x - room.top_x;

	int black = 0, white = 0;

	for (int i = 0; i < room_h; ++i) {
		for (int j = 0; j < room_w; ++j) {
			HYField::Status c_stat = CellStatus(room.top_y + i, room.top_x + j);

			if (c_stat == BLACK) black |= 1 << (i * room_w + j);
			if (c_stat == WHITE) white |= 1 << (i * room_w + j);
		}
	}

	std::vector<int> &db = HYRoomDatabase::Fetch(room_h, room_w, room.hint);
	auto &det = HYRoomDatabase::FetchDetail(room_h, room_w, room.hint);

	int cand = -1;

	for (int i = 0; i < db.size(); i++) {
		int bit = db[i];

		// simple check
		if (!((black & ~bit) == 0 && (white & bit) == 0)) continue;

		int n_aux = 0, repr = -1;
		std::vector<int> adjs;

		for (int pt : det[i]) {
			bool end_flg = pt < 0;
			if (pt < 0) pt = ~pt;

			int y = room.top_y + pt / room_w, x = room.top_x + pt % room_w;

			bool aux_flg = false;

			if (CellStatus(y, x) != BLACK) { // TODO
				for (int j = 0; j < 4; j++) {
					int y2 = y + dy[j] + dy[(j + 1) % 4], x2 = x + dx[j] + dx[(j + 1) % 4];
					int bid = BlackUnitId(y2, x2);

					if (Range(y2, x2) && field[Id(y2, x2)].room_id == rid) continue;
					if (bid == -1) continue;
					if (bid == Root(aux_cell)) aux_flg = true;
					else adjs.push_back(bid);
				}

				if (aux_flg) ++n_aux;
			}

			if (CellStatus(y, x) == BLACK) repr = Root(y * width + x);

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
		if (cand == -1) cand = bit;
		else cand = -2;

	next:
		continue;
	}

	if (cand == -1) {
		return status |= INCONSISTENT;
	}

	if (cand != -2) {
		for (int i = 0; i < room_h; ++i) {
			for (int j = 0; j < room_w; ++j) {
				if (cand & (1 << (i * room_w + j))) ret |= DetermineBlack(room.top_y + i, room.top_x + j);
				else ret |= DetermineWhite(room.top_y + i, room.top_x + j);
			}
		}
	}

	return ret;
}

HYField::Status HYField::SolveRoom(RoomId rid)
{
	Room &room = rooms[rid];
	Status ret = NORMAL;

	int room_h = room.end_y - room.top_y, room_w = room.end_x - room.top_x;

	// trivial case: number of black cells is already equal to the hint

	if (HYRoomDatabase::IsAvailable(room_h, room_w, room.hint)) ret |= SolveRoomWithDatabase(rid);

	CellId n_black = 0;

	for (int i = room.top_y; i < room.end_y; ++i) {
		for (int j = room.top_x; j < room.end_x; ++j) {
			if (CellStatus(i, j) == BLACK) ++n_black;
		}
	}

	if (n_black == room.hint) {
		for (int i = room.top_y; i < room.end_y; ++i) {
			for (int j = room.top_x; j < room.end_x; ++j) {
				if (CellStatus(i, j) == UNDECIDED) ret |= DetermineWhite(i, j);
			}
		}
	}

	// 2 * 2 room

	/*
	if (room_h == 2 && room_w == 2 && room.hint == 2) {
		int id1, id2;

		id1 = BlackUnitId(room.top_y - 1, room.top_x - 1);
		id2 = BlackUnitId(room.top_y + 2, room.top_x + 2);

		if ((id1 != -1 && id2 != -1 && Root(id1) == Root(id2)) || CellStatus(room.top_y, room.top_x) == WHITE || CellStatus(room.top_y + 1, room.top_x + 1) == WHITE) {
			ret |= DetermineWhite(room.top_y, room.top_x);
			ret |= DetermineBlack(room.top_y, room.top_x + 1);
			ret |= DetermineBlack(room.top_y + 1, room.top_x);
			ret |= DetermineWhite(room.top_y + 1, room.top_x + 1);
		}

		id1 = BlackUnitId(room.top_y + 2, room.top_x - 1);
		id2 = BlackUnitId(room.top_y - 1, room.top_x + 2);

		if ((id1 != -1 && id2 != -1 && Root(id1) == Root(id2)) || CellStatus(room.top_y + 1, room.top_x) == WHITE || CellStatus(room.top_y, room.top_x + 1) == WHITE) {
			ret |= DetermineBlack(room.top_y, room.top_x);
			ret |= DetermineWhite(room.top_y, room.top_x + 1);
			ret |= DetermineWhite(room.top_y + 1, room.top_x);
			ret |= DetermineBlack(room.top_y + 1, room.top_x + 1);
		}
	}
	*/

	return status |= ret;
}

HYField::Status HYField::SolveTrivialRoom(RoomId rid)
{
	Room &room = rooms[rid];
	Status ret = NORMAL;

	int room_h = room.end_y - room.top_y, room_w = room.end_x - room.top_x;

	if (room.hint == 0) {
		// hint == 0 (trivial)
		for (int i = room.top_y; i < room.end_y; i++) {
			for (int j = room.top_x; j < room.end_x; j++) {
				ret |= DetermineWhite(i, j);
			}
		}
	}

	if (room_h == 1 && room_w == room.hint * 2 - 1) {
		for (int i = 0; i < room_w; i++) {
			if (i % 2 == 0) ret |= DetermineBlack(room.top_y, room.top_x + i);
			else ret |= DetermineWhite(room.top_y, room.top_x + i);
		}
	}

	if (room_w == 1 && room_h == room.hint * 2 - 1) {
		for (int i = 0; i < room_h; i++) {
			if (i % 2 == 0) ret |= DetermineBlack(room.top_y + i, room.top_x);
			else ret |= DetermineWhite(room.top_y + i, room.top_x);
		}
	}

	if (room_h == 3 && room_w == 3 && room.hint == 5) {
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				if ((i + j) % 2 == 0) ret |= DetermineBlack(room.top_y + i, room.top_x + j);
				else ret |= DetermineWhite(room.top_y + i, room.top_x + j);
			}
		}
	}

	ret |= SolveRoom(rid);

	return ret;
}
