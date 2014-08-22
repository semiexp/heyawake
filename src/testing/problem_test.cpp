
#include "testing.h"

void ProblemTest()
{
	HYRoomDatabase::Initialize();

	HYProblem prob = GetTestProblem(3);

	HYField field(prob);
	field.Debug();

	HYSolver::Solve(field);
	printf("%d\n", (int)field.GetStatus());
	field.Debug();
}
