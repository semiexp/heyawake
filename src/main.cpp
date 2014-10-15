// heyawake.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "testing/testing.h"

int main(int argc, char* argv[])
{
	//ProblemTest(7);
	//PerformanceTest(7, 300);
	//PseudoConnectionTest();
	//ConsecutiveSolver();
	//BruteTest(-1);
	//for(int i = 0; i <= 7; ++i) EvaluateTest(i);
	//EvaluateTest(-1);
	ConsecutiveEvaluator(argc, argv);

	return 0;
}

