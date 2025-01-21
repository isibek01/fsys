/*******************************************************************************************
*                              NOMBREFICH												   *
********************************************************************************************
* AUTOR:			Fagor Automation
* DESCRIPCION:		Example of user real time code
* IMPLEMENTACION:
* MODIFICACIONES:
*******************************************************************************************/

/*----------------------------------------------------------------------------------------*/
/*                              INCLUDES												  */
/*----------------------------------------------------------------------------------------*/
#include "optitwin_structs.h"
//#include <wtypes.h>
#include <stdio.h>

#if SIMU == 0

#ifndef _X86_
#define _X86_ 1
#endif

#include "ntddk.h"
//#include <ntdef.h>

#endif

/*----------------------------------------------------------------------------------------*/
/*                             CONSTANTES												  */
/*----------------------------------------------------------------------------------------*/

const wchar_t* varDNames[MAX_D_VARS] = {
	_T("A.ATIPPOS.X"),
	_T("A.ATIPPOS.Y"),
	_T("A.ATIPPOS.Z"),
	_T("A.POS.S"),
	_T("A.SREAL.S"),
	_T("G.FREAL")
};

/*----------------------------------------------------------------------------------------*/
/*                               TIPOS													  */
/*----------------------------------------------------------------------------------------*/

typedef struct _SYSTEMTIME {
	unsigned short wYear;
	unsigned short wMonth;
	unsigned short wDayOfWeek;
	unsigned short wDay;
	unsigned short wHour;
	unsigned short wMinute;
	unsigned short wSecond;
	unsigned short wMilliseconds;

} SYSTEMTIME, PSYSTEMTIME, LPSYSTEMTIME;

long getDate()
{
	long date = 0;

#if SIMU
	SYSTEMTIME st;
	GetLocalTime(&st);

	//unsigned short divisor = 100;
	//st.wYear = (unsigned short)(st.wYear % divisor);

	//date = aaaammdd
	date = (long)((st.wYear * 10000) + (st.wMonth * 100) + st.wDay);
#endif

	return date;
}
long getTime()
{
	long date = 0;

#if SIMU
	SYSTEMTIME st;
	GetLocalTime(&st);

	// date = HHMMSSsss
	date = (long)((st.wHour * 10000000) + (st.wMinute * 100000) + (st.wSecond * 1000) + st.wMilliseconds);
#else
	//https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-rtltimetosecondssince1970?redirectedfrom=MSDN
	LARGE_INTEGER SystemTime;
	KeQuerySystemTime(&SystemTime);
	date = (long)SystemTime.QuadPart;
#endif

	return date;
}

/*----------------------------------------------------------------------------------------*/
/*                             VARIABLES												  */
/*----------------------------------------------------------------------------------------*/
static FSYS_varHnd_t hndInputEnable = FSYS_VOID_VAR_HND;
static FSYS_varHnd_t hndInputClose = FSYS_VOID_VAR_HND;

//static FSYS_varHnd_t hndOutputVB = FSYS_VOID_VAR_HND;
static FSYS_varHnd_t hndOutputWarning = FSYS_VOID_VAR_HND;


static	FSYS_streamHnd_t RTStreamHndWrite = FSYS_VOID_STREAM_HND;

struct datos_st varStruct = FSYS_VOID_VAR_HND;
struct valores_st valoresStruct;
var_t vars;

int InputEnable = -1;
int InputClose = -1;

int i;
int j;

const wchar_t driverName[] = L"V" SOLUTION_NAME L"D";
unsigned long RTnscans = 0;
/*----------------------------------------------------------------------------------------*/
/*              PROTOTIPOS DE FUNCIONES Y MACROS LOCALES								  */
/*----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------*/
/*                         FUNCIONES PUBLICAS											  */
/*----------------------------------------------------------------------------------------*/
/* -----------------------------------------------------------------------------------------
 * -- Description:			It is called once every time the cyclic proccess is going to
 *							start.
 * -- Parameters:
 * -- Returned Value:		return code
 * -- Assumptions:			No real time.
 * -- Postconditions:
 * -- Implementation:
 */
fmmRetCode_t FMM_initialize(void) {
	InputEnable = 0;
	InputClose = 0;

	return(FMM_RET_CODE_OK);
}

/* -----------------------------------------------------------------------------------------
 * -- Description:			When the cyclic proccess is running, it is called every cycle
 *							before the step function.
 * -- Parameters:
 * -- Returned Value:		return code
 * -- Assumptions:			Real time.
 * -- Postconditions:
 * -- Implementation:
 */
fmmRetCode_t FMM_readInputs(void) {
	//lectura de las variables. Ojo los drivers deben estar inicialziados desde la aplicacion
	FSYS_ReadVariable(&hndInputEnable, &InputEnable, sizeof(InputEnable));
	FSYS_ReadVariable(&hndInputClose, &InputClose, sizeof(InputClose));

	if (InputEnable != 0)
	{
		for (i = 0; i < MAX_VARS; ++i) {
			if (vars.var[i].varName != NULL && //Si la variable está definida y
				vars.var[i].varName != '\0') { //Si la variable no es vacia (Since C-style strings are always terminated with the null character '\0')
				j = 0;
				int isDouble = 0;
				do {
					//TODO if var es double c 
					if (wcscmp(vars.var[j].varName, varDNames[j]) == 0) {
						FSYS_ReadVariable(&(varStruct.var[i]), &(valoresStruct.vard[i]), sizeof(valoresStruct.vard[i]));
						valoresStruct.varf[i] = 0;
						isDouble = 1;
					}
					j++;
				} while (j < MAX_D_VARS && isDouble == 0);

				//else es float
				if (!isDouble)
				{
					FSYS_ReadVariable(&(varStruct.var[i]), &(valoresStruct.varf[i]), sizeof(valoresStruct.varf[i]));
					valoresStruct.vard[i] = 0;
				}
			}
		}
	}

	return(FMM_RET_CODE_OK);
}

/* -----------------------------------------------------------------------------------------
 * -- Description:			It is called every cicle when the cyclic proccess is running
 * -- Parameters:
 * -- Returned Value:		return code
 * -- Assumptions:			Real time.
 * -- Postconditions:
 * -- Implementation:
 */
fmmRetCode_t FMM_step(void) {  //algoritmo
	//Output1 = Input1 + Input2;

	return(FMM_RET_CODE_OK);
}

/* -----------------------------------------------------------------------------------------
 * -- Description:			When the cyclic proccess is running, it is called every cycle
 *							after the step function.
 * -- Parameters:
 * -- Returned Value:		return code
 * -- Assumptions:			Real time.
 * -- Postconditions:
 * -- Implementation:
 */
fmmRetCode_t FMM_writeOutputs(void) {
	FSYS_retCode_t ringWriteReturnCode = FSYS_RET_CODE_OK;

	struct EventInfo_st	event;
	int numElements = 0;
	short i;
	long date = getDate(), time = getTime();

	event.appId = APP_ID;
	//event.varId = 1;
	//event.date = date;
	event.nScan = time;
	event.enable = InputEnable != 0 ? 1 : (InputClose != 0 ? 2 : 0);

	if (InputEnable != 0)
	{
		for (i = 0; i < MAX_VARS; ++i) {
			if (vars.var[i].varName != NULL && //Si la variable está definida y
				vars.var[i].varName != '\0' &&
				wcslen(vars.var[i].varName) > 0) //Si la variable no es vacia (Since C-style strings are always terminated with the null character '\0')
			{
				event.v[i].dValue = valoresStruct.vard[i];
				event.v[i].fValue = valoresStruct.varf[i];

				++numElements;
			}
			else
				break;
		}
	}

	// Escritura en Pipe desde driver hacia aplicacion
	ringWriteReturnCode = FSYS_WriteStream(&event, sizeof(event), 1, &RTStreamHndWrite);

	RTnscans++;

	//escritura del resultado del algoritmo
	//FSYS_WriteVariable(&hndOutput1, &Output1, sizeof(Output1));

	return(FMM_RET_CODE_OK);
}

/* -----------------------------------------------------------------------------------------
 * -- Description:			It is called once when the cyclic proccess is stopped.
 * -- Parameters:
 * -- Returned Value:		return code
 * -- Assumptions:			No real time.
 * -- Postconditions:
 * -- Implementation:
 */
fmmRetCode_t FMM_terminate(void) {
	return(FMM_RET_CODE_OK);
}
/*----------------------------------------------------------------------------------------*/
/*                         FUNCIONES LOCALES											  */
/*----------------------------------------------------------------------------------------*/

/******************************************************************************************/
/*                                FIN						                              */
/******************************************************************************************/
