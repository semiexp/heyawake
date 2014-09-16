
#include "testing.h"

#include <iostream>
#include <string>

int n_success, n_difficult, n_inconsistent;

bool Report(std::string &pid, int stat, int level)
{
	if (stat & HYField::INCONSISTENT) {
		std::cout << "Problem " << pid << ": Inconsistent (bug)" << std::endl;
		std::cerr << "Problem " << pid << ": Inconsistent (bug)" << std::endl;

		++n_inconsistent;
		return true;
	} else if (stat & HYField::SOLVED) {
		if (level == 0) {
			std::cout << "Problem " << pid << ": Successfully solved without assumptions" << std::endl;
			std::cerr << "Problem " << pid << ": Successfully solved without assumptions" << std::endl;
		} else if (level == 1) {
			std::cout << "Problem " << pid << ": Successfully solved with assumptions" << std::endl;
			std::cerr << "Problem " << pid << ": Successfully solved with assumptions" << std::endl;
		}
		++n_success;
		return true;
	}
	return false;
}

void ConsecutiveSolver()
{
	std::string buf;
	n_success = n_difficult = n_inconsistent = 0;

	HYRoomDatabase::Initialize();

	while (!std::cin.eof()) {
		std::cin >> buf;
		std::cin >> buf;
		std::cin >> buf;
		if (std::cin.eof()) break;

		std::cerr << "problem " << buf << std::endl;

		int H, W, R;
		int ty, tx, rh, rw, h;
		std::cin >> H >> W >> R;

		HYProblem prob(H, W);
		for (int i = 0; i < R; ++i) {
			std::cin >> ty >> tx >> rh >> rw >> h;
			prob.AddHint(ty, tx, rh, rw, h);
		}

		HYField field(prob);
		HYSolver::Solve(field);

		if (!Report(buf, field.GetStatus(), 0)) {
			HYSolver::Assume(field);
			if (!Report(buf, field.GetStatus(), 1)) {
				++n_difficult;
				std::cout << "Problem " << buf << ": Too difficult" << std::endl;
				std::cerr << "Problem " << buf << ": Too difficult" << std::endl;
			}
		}
	}

	std::cout << "Solved: " << n_success << " / Too difficult: " << n_difficult << " / Inconsistent (bug): " << n_inconsistent << std::endl;
	return;
}
