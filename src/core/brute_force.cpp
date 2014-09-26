
#include "heyawake.h"

HYField::Status HYSolver::BruteForce(HYField &field, int depth)
{
	Solve(field);
	if (depth <= 8) Assume(field, 1);

	if (field.GetStatus() & HYField::INCONSISTENT) return HYField::INCONSISTENT;
	if (field.GetStatus() & HYField::SOLVED) return HYField::SOLVED;

	int height = field.height, width = field.width;
	for (int i = 0; i < height; ++i) for (int j = 0; j < width; ++j) if (field.CellStatus(i, j) == HYField::UNDECIDED) {
		HYField white(field), black(field);
		white.DetermineWhite(i, j);
		black.DetermineBlack(i, j);

		HYField::Status s_black = BruteForce(black, depth + 1);

		if (s_black == HYField::MULTIPLE_ANSWER) return HYField::MULTIPLE_ANSWER;

		HYField::Status s_white = BruteForce(white, depth + 1);

		if (s_white == HYField::MULTIPLE_ANSWER) return HYField::MULTIPLE_ANSWER;

		if (s_white == HYField::INCONSISTENT && s_black == HYField::INCONSISTENT) return HYField::INCONSISTENT;
		if (s_white == HYField::SOLVED && s_black == HYField::SOLVED) return HYField::MULTIPLE_ANSWER;

		if (s_white == HYField::SOLVED && s_black == HYField::INCONSISTENT) {
			field.CopyStatus(white);
			return HYField::SOLVED;
		}
		if (s_white == HYField::INCONSISTENT && s_black == HYField::SOLVED) {
			field.CopyStatus(black);
			return HYField::SOLVED;
		}
	}

	return HYField::INCONSISTENT;
}
