
#include "heyawake.h"

void HYField::CheckPseudoConnection(CellCord y, CellCord x)
{
	if (!Range(y, x)) return;

	if (1 <= x && x < width - 1 && IsBlackOrOutOfRange(y - 1, x) && IsBlackOrOutOfRange(y + 1, x)) {
		if (CellRoomId(y, x) != CellRoomId(y, x - 1) && CellRoomId(y, x) != CellRoomId(y, x + 1)) {
			rel_pseudo_con[Id(y, x - 1)] = rel_pseudo_con[Id(y, x + 1)] = true;
			conm_ps.Join(BlackUnitId(y - 1, x), BlackUnitId(y + 1, x));
		}
	}

	if (1 <= y && y < height - 1 && IsBlackOrOutOfRange(y, x - 1) && IsBlackOrOutOfRange(y, x + 1)) {
		if (CellRoomId(y, x) != CellRoomId(y - 1, x) && CellRoomId(y, x) != CellRoomId(y + 1, x)) {
			rel_pseudo_con[Id(y - 1, x)] = rel_pseudo_con[Id(y + 1, x)] = true;
			conm_ps.Join(BlackUnitId(y, x - 1), BlackUnitId(y, x + 1));
		}
	}
}

HYField::Status HYSolver::CheckPseudoConnection(HYField &field)
{
	int height = field.height, width = field.width;

	HYField::CellId *space = new HYField::CellId[height * width + 1];
	HYConnectionManager conm(height, width, space);
	conm.Load(field.conm);

	bool *con_related = new bool[height * width];

	for (int i = 0; i < height * width; ++i) con_related[i] = false;

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			if (1 <= j && j < width - 1 && field.IsBlackOrOutOfRange(i - 1, j) && field.IsBlackOrOutOfRange(i + 1, j)) {
				if (field.CellRoomId(i, j) != field.CellRoomId(i, j - 1) && field.CellRoomId(i, j) != field.CellRoomId(i, j + 1)) {
					con_related[field.Id(i, j - 1)] = con_related[field.Id(i, j + 1)] = true;
					conm.Join(field.BlackUnitId(i - 1, j), field.BlackUnitId(i + 1, j));
				}
			}

			if (1 <= i && i < height - 1 && field.IsBlackOrOutOfRange(i, j - 1) && field.IsBlackOrOutOfRange(i, j + 1)) {
				if (field.CellRoomId(i, j) != field.CellRoomId(i - 1, j) && field.CellRoomId(i, j) != field.CellRoomId(i + 1, j)) {
					con_related[field.Id(i - 1, j)] = con_related[field.Id(i + 1, j)] = true;
					conm.Join(field.BlackUnitId(i, j - 1), field.BlackUnitId(i, j + 1));
				}
			}
		}
	}

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			if (con_related[field.Id(i, j)]) continue;

			if (field.CellStatus(i, j) == HYField::UNDECIDED && !conm.CheckValidity(i, j)) {
				field.DetermineWhite(i, j);
			}
		}
	}

	delete[] space;
	delete[] con_related;

	return field.GetStatus();
}
