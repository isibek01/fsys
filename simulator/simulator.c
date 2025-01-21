// simulator.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <windows.h>
//#include "fsys.h"
#include "fmm_interface.h"

wchar_t driverName[] = SOLUTION_NAME;

int __cdecl _tmain(int argc, TCHAR * argv[])
{
	FMM_start_simulated_app(argv, driverName, argc);

	return(0);
}

