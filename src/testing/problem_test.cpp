
#include "testing.h"

void ProblemTest(int pid)
{
	HYRoomDatabase::Initialize();

	HYProblem prob = GetTestProblem(pid);

	HYField field(prob);
	field.Debug();

	HYSolver::Solve(field);
	printf("%d\n", (int)field.GetStatus());
	field.Debug();
}
