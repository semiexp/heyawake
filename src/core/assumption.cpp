
#include "heyawake.h"

HYField::Status HYSolver::Assume(HYField &field)
{
	HYField::Status ret = HYField::NORMAL;

	if (field.GetStatus() != HYField::NORMAL) return ret;

	int height = field.height, width = field.width;

	bool update;

	do {
		update = false;

		for (int i = 0; i < height; ++i) {
			for (int j = 0; j < width; ++j) {
				if (field.CellStatus(i, j) == HYField::UNDECIDED) {
					HYField f_black = field, f_white = field;

					f_black.DetermineBlack(i, j);
					f_white.DetermineWhite(i, j);

					Solve(f_black);
					Solve(f_white);

					if ((f_black.GetStatus() & HYField::INCONSISTENT) && (f_white.GetStatus() & HYField::INCONSISTENT)) {
						return ret |= HYField::INCONSISTENT;
					}

					if ((f_black.GetStatus() & HYField::INCONSISTENT)) {
						ret |= field.DetermineWhite(i, j);
						update = true;
					}

					if ((f_white.GetStatus() & HYField::INCONSISTENT)) {
						ret |= field.DetermineBlack(i, j);
						update = true;
					}

					for (int y = 0; y < height; ++y) {
						for (int x = 0; x < width; ++x) {
							HYField::Status t_black = f_black.CellStatus(y, x), t_white = f_white.CellStatus(y, x);

							if (t_black == HYField::BLACK && t_white == HYField::BLACK) {
								ret |= field.DetermineBlack(y, x);
							}
							if (t_black == HYField::WHITE && t_white == HYField::WHITE) {
								ret |= field.DetermineWhite(y, x);
							}
						}
					}
				}
			}
		}

		if (update) Solve(field);
	} while (update);

	return ret;
}
