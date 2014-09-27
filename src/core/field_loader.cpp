
#include "heyawake.h"
#include "../util/util.h"

HYField::Status HYField::Load(HYProblem &prob)
{
	height = prob.height;
	width = prob.width;

	n_rsets = 0;
	int nSetCells = 0;
	status = NORMAL;
	progress = 0;

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

	sz_pool = sizeof(Cell) * (height * width) + sizeof(RestrictedSet) * n_rsets + sizeof(Room) * n_rooms + sizeof(RSetId) * nSetCells + (sizeof(CellId) * 2 + sizeof(bool)) * (height * width + 1);

	pool = new char[sz_pool];
	TrivialAllocator ta(pool);

	field = (Cell*)ta.allocate(sizeof(Cell) * (height * width));
	rsets = (RestrictedSet*)ta.allocate(sizeof(RestrictedSet) * n_rsets);
	rooms = (Room*)ta.allocate(sizeof(Room) * n_rooms);
	RSetId *rset_holder = (RSetId*)ta.allocate(sizeof(RSetId) * nSetCells);
	conm = HYConnectionManager(height, width, (CellId*)ta.allocate(sizeof(CellId) * (height * width + 1)));
	conm_ps = HYConnectionManager(height, width, (CellId*)ta.allocate(sizeof(CellId) * (height * width + 1)));
	rel_pseudo_con = (bool*)ta.allocate(sizeof(bool) * (height * width + 1));

	aux_cell = height * width;

	for (int i = 0; i < height * width; i++) {
		field[i].stat = NORMAL;
		field[i].n_conds = 0;
	}

	std::fill(rel_pseudo_con, rel_pseudo_con + (height * width + 1), false);

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

				++rset_id;
			}
		}
	}

	rset_id = 0;

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			CellId id = Id(i, j);

			field[id].cond = rset_holder + rset_id;
			rset_id += field[id].n_conds;

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

	Status ret = NORMAL;
	if (method.room_check) {
		for (int i = 0; i < n_rooms; i++) ret |= SolveTrivialRoom(i);
	}

	return ret;
}
