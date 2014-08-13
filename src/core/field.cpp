
#include "heyawake.h"

HYField::HYField()
{
	height = width = 0;
	n_rsets = 0;
	n_rooms = 0;

	field = nullptr;
	rsets = nullptr;
	rooms = nullptr;
	pool = nullptr;
	sz_pool = 0;

	status = NORMAL;
}

HYField::HYField(HYProblem &prob)
{
	Load(prob);
}

HYField::HYField(const HYField &src)
{
	height = src.height;
	width = src.width;
	n_rsets = src.n_rsets;
	n_rooms = src.n_rooms;
	status = src.status;

	sz_pool = src.sz_pool;
	pool = new char[sz_pool];

	field = (Cell*)(pool + ((char*)src.field - src.pool));
	rsets = (RestrictedSet*)(pool + ((char*)src.rsets - src.pool));
	rooms = (Room*)(pool + ((char*)src.rooms - src.pool));

	for (int i = 0; i < sz_pool; i++) pool[i] = src.pool[i];

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int id = Id(i, j);

			field[id].cond = (RSetId*)(pool + ((char*)src.field[id].cond - src.pool));
		}
	}
}

void HYField::Load(HYProblem &prob)
{
	height = prob.height;
	width = prob.width;

	n_rsets = 0;
	int nSetCells = 0;
	status = NORMAL;

	for (int i = 0; i < prob.hint.size(); i++) {
		if (0 < prob.top_y[i] && prob.end_y[i] < height) {
			n_rsets += prob.end_x[i] - prob.top_x[i];
			nSetCells += (prob.end_y[i] - prob.top_y[i] + 2) * (prob.end_x[i] - prob.top_x[i]);
		}

		if (0 < prob.top_x[i] && prob.end_x[i] < width) {
			n_rsets += prob.end_y[i] - prob.top_y[i];
			nSetCells += (prob.end_x[i] - prob.top_x[i] + 2) * (prob.end_y[i] - prob.top_y[i]);
		}
	}

	n_rooms = prob.hint.size();
	
	sz_pool = sizeof(Cell) * height * width + sizeof(RestrictedSet) * n_rsets + sizeof(Room) * n_rooms + sizeof(RSetId) * nSetCells;
	pool = new char[sz_pool];

	field = (Cell*)pool;
	rsets = (RestrictedSet*)(pool + sizeof(Cell) * height * width);
	rooms = (Room*)(pool + sizeof(Cell) * height * width + sizeof(RestrictedSet) * n_rsets);
	RSetId *rset_holder = (RSetId*)(pool + sizeof(Cell) * height * width + sizeof(RestrictedSet) * n_rsets + sizeof(Room) * n_rooms);

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			int id = Id(i, j);

			field[id].stat = NORMAL;
		}
	}

	RSetId rset_id = 0;
	int rset_cell_id = 0;

	for (int i = 0; i < n_rooms; ++i) {
		Room &room = rooms[i] = Room(prob.top_y[i], prob.top_x[i], prob.end_y[i], prob.end_x[i], prob.hint[i]);

		for (int y = room.top_y; y < room.end_y; ++y) {
			for (int x = room.top_x; x < room.end_x; ++x) {
				field[y * width + x].room_id = i;
			}
		}

		if (0 < room.top_y && room.end_y < height) {
			for (int x = room.top_x; x < room.end_x; ++x) {
				rsets[rset_id].stat = NORMAL;
				rsets[rset_id].rem_cells = room.end_y - room.top_y + 2;
				rsets[rset_id].xor_id = 0;

				for (int y = room.top_y - 1; y < room.end_y + 1; ++y) {
					int id = Id(y, x);

					rsets[rset_id].xor_id ^= id;
					++field[id].n_conds;
				}

				rsets[rset_id].xor_id = 0;

				++rset_id;
			}
		}

		if (0 < room.top_x && room.end_x < width) {
			for (int y = room.top_y; y < room.end_y; ++y) {
				rsets[rset_id].stat = NORMAL;
				rsets[rset_id].rem_cells = room.end_x - room.top_x + 2;
				rsets[rset_id].xor_id = 0;

				for (int x = room.top_x - 1; x < room.end_x + 1; ++x) {
					int id = Id(y, x);

					rsets[rset_id].xor_id ^= id;
					++field[id].n_conds;
				}

				rsets[rset_id].xor_id = 0;

				++rset_id;
			}
		}
	}

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			int id = Id(i, j);

			field[id].cond = rset_holder + rset_id;
			rset_holder += field[id].n_conds;
		}
	}

	rset_id = 0;

	for (int i = 0; i < n_rooms; ++i) {
		Room &room = rooms[i] = Room(prob.top_y[i], prob.top_x[i], prob.end_y[i], prob.end_x[i], prob.hint[i]);

		if (0 < room.top_y && room.end_y < height) {
			for (int x = room.top_x; x < room.end_x; ++x) {
				for (int y = room.top_y - 1; y < room.end_y + 1; ++y) {
					int id = Id(y, x);

					*(field[id].cond++) = rset_id;
				}

				++rset_id;
			}
		}

		if (0 < room.top_x && room.end_x < width) {
			for (int y = room.top_y; y < room.end_y; ++y) {
				for (int x = room.top_x - 1; x < room.end_x + 1; ++x) {
					int id = Id(y, x);

					*(field[id].cond++) = rset_id;
				}

				++rset_id;
			}
		}
	}

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			int id = Id(i, j);

			field[id].cond -= field[id].n_conds;
		}
	}
}

HYField::Status HYField::ExcludeFromRSet(RSetId sid, CellId cid)
{
	rsets[sid].xor_id ^= cid;
	--rsets[sid].rem_cells;
	rsets[sid].stat |= field[cid].stat;

	return SolveRestrictedSet(sid);
}

HYField::Status HYField::Exclude(CellId cid)
{
	Status ret = NORMAL;
	for (int i = 0; i < field[cid].n_conds; i++) {
		ret |= ExcludeFromRSet(field[cid].cond[i], cid);
	}

	return ret;
}

HYField::Status HYField::DetermineBlack(CellCord y, CellCord x)
{
	CellId id = Id(y, x);

	if (field[id].stat == WHITE) return status |= INCONSISTENT;
	if (field[id].stat == BLACK) return NORMAL;

	Status ret = NORMAL;

	field[id].stat = BLACK;

	if (Range(y - 1, x)) ret |= DetermineWhite(y - 1, x);
	if (Range(y + 1, x)) ret |= DetermineWhite(y + 1, x);
	if (Range(y, x - 1)) ret |= DetermineWhite(y, x - 1);
	if (Range(y, x + 1)) ret |= DetermineWhite(y, x + 1);

	Exclude(id);
	ret |= SolveRoom(field[id].room_id);

	return status |= ret;
}

HYField::Status HYField::DetermineWhite(CellCord y, CellCord x)
{
	CellId id = Id(y, x);

	if (field[id].stat == BLACK) return status |= INCONSISTENT;
	if (field[id].stat == WHITE) return NORMAL;

	Status ret = NORMAL;

	field[id].stat = WHITE;

	Exclude(id);
	ret |= SolveRoom(field[id].room_id);

	return status |= ret;
}

HYField::Status HYField::SolveRestrictedSet(RSetId sid)
{
	if (rsets[sid].rem_cells == 1 && !(rsets[sid].stat | BLACK)) {
		int cid = rsets[sid].xor_id;

		return DetermineBlack(cid / width, cid % width);
	}

	return NORMAL;
}

HYField::Status HYField::SolveRoom(RoomId rid)
{
	return NORMAL;
}
