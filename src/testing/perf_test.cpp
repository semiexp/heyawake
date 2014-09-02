
#include "testing.h"

#include <ctime>

void PerformanceTest(int pid, int rec)
{
	clock_t start, end;
	const int trial = rec;

	HYRoomDatabase::Initialize();

	start = clock();

	for (int i = 0; i < trial; i++) {
		HYProblem prob = GetTestProblem(pid);

		HYField field(prob);
		HYSolver::Solve(field);
	}

	end = clock();

	printf("%.3f[s]\n", (double)(end - start) / CLOCKS_PER_SEC);
}
