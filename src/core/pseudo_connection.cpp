
#include "heyawake.h"

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
