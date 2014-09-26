
#include "testing.h"
#include <iostream>
#include <string>

void BruteTest(int pid)
{
	HYRoomDatabase::Initialize();

	if (pid == -1) {
		FILE *fp;
//		freopen_s(&fp, "402.txt", "r", stdin);

		std::string buf;
		std::cin >> buf;
		std::cin >> buf;
		std::cin >> buf;

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

		printf("%d\n", HYSolver::BruteForce(field));
		field.Debug();
	} else {
		HYProblem prob = GetTestProblem(pid);
		HYField field(prob);

		printf("%d\n", HYSolver::BruteForce(field));
		field.Debug();
	}
}
