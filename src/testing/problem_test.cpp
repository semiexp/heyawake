
#include "testing.h"

void ProblemTest()
{
	HYRoomDatabase::Initialize();

	HYProblem prob = GetTestProblem(2);

	HYField field(prob);
	field.Debug();

	printf("%d\n", (int)HYSolver::AssureConnectivity(field));
	printf("%d\n", (int)HYSolver::AssureConnectivity(field));
	printf("%d\n", (int)HYSolver::CheckAllRoom(field));
	printf("%d\n", (int)HYSolver::AssureConnectivity(field));
	printf("%d\n", (int)HYSolver::CheckAllRoom(field));
	printf("%d\n", (int)HYSolver::AssureConnectivity(field));
	printf("%d\n", (int)field.GetStatus());
	field.Debug();
}
