
#include "heyawake.h"

HYField::Status HYSolver::CheckPseudoConnection(HYField &field)
{
	int height = field.height, width = field.width;

	HYField::CellId *space = new HYField::CellId[height * width + 1];
	HYConnectionManager conm(height, width, space);
	conm.Load(field.conm);

	bool *con_related = new bool[height * width];

	for (int i = 0; i < height * width; ++i) con_related[i] = false;

	for (int i = 1; i < height - 1; ++i) {
		for (int j = 1; j < width - 1; ++j) {
			if (field.CellStatus(i - 1, j) == HYField::BLACK && field.CellStatus(i + 1, j) == HYField::BLACK) {
				if (field.CellRoomId(i, j) != field.CellRoomId(i, j - 1) && field.CellRoomId(i, j) != field.CellRoomId(i, j + 1)) {
					con_related[field.Id(i, j - 1)] = con_related[field.Id(i, j + 1)] = true;
					conm.Join(field.Id(i - 1, j), field.Id(i + 1, j));
				}
			}

			if (field.CellStatus(i, j - 1) == HYField::BLACK && field.CellStatus(i, j + 1) == HYField::BLACK) {
				if (field.CellRoomId(i, j) != field.CellRoomId(i - 1, j) && field.CellRoomId(i, j) != field.CellRoomId(i + 1, j)) {
					con_related[field.Id(i - 1, j)] = con_related[field.Id(i + 1, j)] = true;
					conm.Join(field.Id(i, j - 1), field.Id(i, j + 1));
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
	return field.GetStatus();
}
