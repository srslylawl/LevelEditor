#include "DPIScale.h"
#include <Windows.h>

void SetWindowsDPIScaleAware() {
	SetProcessDPIAware();
}
