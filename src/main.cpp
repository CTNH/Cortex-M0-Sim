#include "cortex-m0p_core.h"
#include <iostream>
using namespace std;

int main() {
	CM0P_Core core;
	core.step_inst();

	return 0;
}
