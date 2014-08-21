
#include "heyawake.h"

HYField::Status HYSolver::AssureConnectivity(HYField &field)
{
	int height = field.height, width = field.width;

	HYField::Status ret = HYField::NORMAL;

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			if (field.CellStatus(i, j) == HYField::UNDECIDED) {
				ret |= field.AssureConnectivity(i, j);
			}
		}
	}

	return ret;
}

HYField::Status HYSolver::CheckAllRoom(HYField &field)
{
	HYField::Status ret = HYField::NORMAL;

	for (int i = 0; i < field.n_rooms; i++) {
		ret |= field.SolveRoom(i);
	}

	return ret;
}
