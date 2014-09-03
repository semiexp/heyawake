
#include "heyawake.h"

#include <cstdio>
#include <cstdlib>
#include <algorithm>

int HYField::dx[] = { -1, 0, 1, 0 };
int HYField::dy[] = { 0, 1, 0, -1 };

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
	progress = 0;

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
	progress = 0;

	sz_pool = src.sz_pool;
	pool = new char[sz_pool];

	field = (Cell*)(pool + ((char*)src.field - src.pool));
	rsets = (RestrictedSet*)(pool + ((char*)src.rsets - src.pool));
	rooms = (Room*)(pool + ((char*)src.rooms - src.pool));

	memcpy(pool, src.pool, sz_pool);

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int id = Id(i, j);

			field[id].cond = (RSetId*)(pool + ((char*)src.field[id].cond - src.pool));
		}
	}
}

HYField::~HYField()
{
	delete[] pool;
}

void HYField::Join(CellId p, CellId q)
{
	p = Root(p); q = Root(q);

	if (p == q) return;

	field[p].unit_root += field[q].unit_root;
	field[q].unit_root = p;
}

HYField::Status HYField::AssureConnectivity(CellCord y, CellCord x)
{
	int ids[5];

	for (int i = 0; i < 4; i++) {
		int y2 = y + dy[i] + dy[(i+3)%4], x2 = x + dx[i] + dx[(i+3)%4];

		ids[i] = BlackUnitId(y2, x2);
	}

	ids[4] = ids[0];

	for (int i = 0; i < 4; i++) {
		int y2 = y + dy[i], x2 = x + dx[i];

		if (Range(y2, x2)) {
			if (ids[i] != -1 && ids[i + 1] != -1) {
				if (Root(ids[i]) == Root(ids[i + 1])) {
					DetermineWhite(y, x);
				}
			}
		}
	}

	if (0 < y && y < height - 1 && 0 < x && x < width - 1) {
		if (ids[0] != -1 && ids[2] != -1 && Root(ids[0]) == Root(ids[2])) DetermineWhite(y, x);
		if (ids[1] != -1 && ids[3] != -1 && Root(ids[1]) == Root(ids[3])) DetermineWhite(y, x);
	}

	return status;
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
	for (int i = 0; i < field[cid].n_conds; ++i) {
		ExcludeFromRSet(field[cid].cond[i], cid);
	}

	return status;
}

HYField::Status HYField::DetermineBlack(CellCord y, CellCord x)
{
	CellId id = Id(y, x);

	if (field[id].stat == WHITE) return status |= INCONSISTENT;
	if (field[id].stat == BLACK) return status;

	AssureConnectivity(y, x);

	if (field[id].stat == WHITE) return status |= INCONSISTENT;
	/* TODO: Check the validity of the connection */
	field[id].stat = BLACK;
	++progress;

	if (progress == height * width) { status |= SOLVED; }

	for (int i = 0; i < 4; ++i) {
		int y2 = y + dy[i] + dy[(i + 1) % 4], x2 = x + dx[i] + dx[(i + 1) % 4];

		int id2 = BlackUnitId(y2, x2);
		if (id2 != -1) Join(id, id2);
	}

	if (Range(y - 1, x)) DetermineWhite(y - 1, x);
	if (Range(y + 1, x)) DetermineWhite(y + 1, x);
	if (Range(y, x - 1)) DetermineWhite(y, x - 1);
	if (Range(y, x + 1)) DetermineWhite(y, x + 1);

	Exclude(id);
	// ret |= SolveRoom(field[id].room_id);

	return status;
}

HYField::Status HYField::DetermineWhite(CellCord y, CellCord x)
{
	CellId id = Id(y, x);

	if (field[id].stat == BLACK) return status |= INCONSISTENT;
	if (field[id].stat == WHITE) return status;

	field[id].stat = WHITE;
	++progress;

	if (progress == height * width) { status |= SOLVED; }

	Exclude(id);

	return status;
}

HYField::Status HYField::SolveRestrictedSet(RSetId sid)
{
	if (rsets[sid].rem_cells == 1 && !(rsets[sid].stat & BLACK)) {
		int cid = rsets[sid].xor_id;

		return DetermineBlack(cid / width, cid % width);
	}

	return status;
}

void HYField::Debug()
{
	FILE* output = stdout;

	for (int i = 0; i <= 2 * height; i++) {
		for (int j = 0; j <= 2 * width; j++) {
			if (i % 2 == 0 && j % 2 == 0) {
				fprintf(output, "+");
			}

			if (i % 2 == 0 && j % 2 != 0) {
				bool edge = !Range(i / 2, j / 2) || !Range(i / 2 - 1, j / 2) || (field[Id(i / 2, j / 2)].room_id != field[Id(i / 2 - 1, j / 2)].room_id);

				if (edge) fprintf(output, "---");
				else fprintf(output, "   ");
			}

			if (i % 2 != 0 && j % 2 == 0) {
				bool edge = !Range(i / 2, j / 2) || !Range(i / 2, j / 2 - 1) || (field[Id(i / 2, j / 2)].room_id != field[Id(i / 2, j / 2 - 1)].room_id);

				if (edge) fprintf(output, "|");
				else fprintf(output, " ");
			}

			if (i % 2 != 0 && j % 2 != 0) {
				int vHint = -1;

				int r_id = field[Id(i / 2, j / 2)].room_id;
				if (rooms[r_id].top_y == i / 2 && rooms[r_id].top_x == j / 2) {
					vHint = rooms[r_id].hint;
				}

				fprintf(output, "%c%c%c",
					field[Id(i / 2, j / 2)].stat == BLACK ? '[' : 
					field[Id(i / 2, j / 2)].stat == WHITE ? '.' : ' ',
					vHint != -1 ? (vHint + '0') : 
					field[Id(i / 2, j / 2)].stat == WHITE ? '.' : ' ',
					field[Id(i / 2, j / 2)].stat == BLACK ? ']' :
					field[Id(i / 2, j / 2)].stat == WHITE ? '.' : ' '
				);
			}
		}
		fprintf(output, "\n");
	}

	fprintf(output, "\n");

}
