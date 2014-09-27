
#include "testing.h"

void EvaluateTest(int pid)
{
	HYRoomDatabase::Initialize();

	HYProblem prob = GetTestProblem(pid);

	printf("%f\n", HYEvaluator::Evaluate(prob));
}
