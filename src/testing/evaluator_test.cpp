
#include "testing.h"
#include "../frontend/frontend.h"

void EvaluateTest(int pid)
{
	HYRoomDatabase::Initialize();

	HYProblem prob;
	
	if (pid == -1) {
		prob = LoadPBProblem(std::cin);
	} else GetTestProblem(pid);

	printf("%f\n", HYEvaluator::Evaluate(prob));
}
