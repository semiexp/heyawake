
#include "testing.h"

void ProblemTest()
{
	HYRoomDatabase::Initialize();

	HYProblem prob = GetTestProblem(0);

	HYField field(prob);
	field.Debug();

	printf("%d\n", (int)HYSolver::AssureConnectivity(field));
	printf("%d\n", (int)HYSolver::AssureConnectivity(field));
	field.Debug();
}
