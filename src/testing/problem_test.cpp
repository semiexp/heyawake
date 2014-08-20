
#include "testing.h"

void ProblemTest()
{
	HYProblem prob = GetTestProblem(1);

	HYField field(prob);
	field.Debug();

	printf("%d\n", (int)HYSolver::AssureConnectivity(field));
	printf("%d\n", (int)HYSolver::AssureConnectivity(field));
	field.Debug();
}
