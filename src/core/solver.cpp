
#include "heyawake.h"

HYField::Status HYSolver::Solve(HYField &field)
{
	int cur_progress;

	do {
		cur_progress = field.GetProgress();

		AssureConnectivity(field);
		CheckAllRoom(field);
	} while (field.status == HYField::NORMAL && cur_progress != field.GetProgress());

	return field.status;
}

HYField::Status HYSolver::AssureConnectivity(HYField &field)
{
	int height = field.height, width = field.width;

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			if (field.CellStatus(i, j) == HYField::UNDECIDED) {
				field.AssureConnectivity(i, j);
			}
		}
	}

	return field.status;
}

HYField::Status HYSolver::CheckAllRoom(HYField &field)
{
	for (int i = 0; i < field.n_rooms; i++) {
		field.SolveRoom(i);
	}

	return field.status;
}
