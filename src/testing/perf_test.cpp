
#include "testing.h"

#include <ctime>

void PerformanceTest()
{
	clock_t start, end;
	const int trial = 20000;

	HYRoomDatabase::Initialize();

	start = clock();

	for (int i = 0; i < trial; i++) {
		HYProblem prob = GetTestProblem(2);

		HYField field(prob);
		HYSolver::Solve(field);
	}

	end = clock();

	printf("%.3f[s]\n", (double)(end - start) / CLOCKS_PER_SEC);
}
