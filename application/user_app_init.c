/*******************************************************************************************
*																Fsys App Template		   *
********************************************************************************************
* AUTOR:				Fagor Automation
* DESCRIPCION:			Example of user application code
* IMPLEMENTACION:
* MODIFICACIONES:
*******************************************************************************************/
/*----------------------------------------------------------------------------------------*/
/*																			INCLUDES	  */
/*----------------------------------------------------------------------------------------*/
#include "optitwin_structs.h"
#include <stdio.h>
#include "stdlib.h"
/*----------------------------------------------------------------------------------------*/
/*																	CONSTANTES			  */
/*----------------------------------------------------------------------------------------*/
#define MAX_REPETITIONS						1024
#define STREAM_SIZE							0x10000		//64K
/*----------------------------------------------------------------------------------------*/
/*																		TIPOS			  */
/*----------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------*/
/*																	VARIABLES			  */
/*----------------------------------------------------------------------------------------*/
TCHAR algorithmVersion[MAX_STRING] = _T("");
extern TCHAR algorithmEnable[MAX_STRING];
extern TCHAR closeFile[MAX_STRING];
extern TCHAR warningMessage[MAX_STRING];
extern TCHAR folderCSV[MAX_STRING];
extern var_t vars;
extern char optitwinPort[64];
/*----------------------------------------------------------------------------------------*/
/*											PROTOTIPOS DE FUNCIONES Y MACROS LOCALES	  */
/*----------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------*/
/*														FUNCIONES PUBLICAS				  */
/*----------------------------------------------------------------------------------------*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//FUNCION que recibe la información del fichero de configuración. Utiliza un JS para leer 
// el fichero de configuracion XML
//Parámetros entrada.
//Parámetros salida
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
fmmRetCode_t getConfig(TCHAR* pConfigFileName, BOOL isSimu) {
	const TCHAR jsFileName[] = _T("optitwinGetConfig.js");
	TCHAR	cScriptCmd[MAX_STRING] = _T(""); 			//Linea comandos para ejecutar script /D = DEBUG
	TCHAR	exeFolder[MAX_STRING] = _T("");				//El js y el XML deben estar en el mismo path que la aplicacin de usuario
	TCHAR	jsFullFileName[MAX_STRING] = _T("");	//Full path
	FILE* pPipe;
	int		retcode = 0, totalParams = 0, car = 0;		//caracter rx

	//printf("Javascript name: %ws\n", &jsFileName);

	if (isSimu == TRUE)
		retcode = FMM_GetPath(exeFolder, _T("optitwin_app_sim.dll")); //Path donde está cargada la DLL
	else
		retcode = FMM_GetPath(exeFolder, NULL);	//Obtener path donde se ejecuta la aplicacin .exe
	if (retcode != FSYS_RET_CODE_OK)
		return (fmmRetCode_t)retcode;

	//printf("Exe folder: %ws\n", &exeFolder);

	//Crear el path completo del javascript. Debe estar en la misma carpeta que el xml. y ambos en la misma carpeta que el user_app.exe.
	_stprintf_s(jsFullFileName, NItemsArray(jsFullFileName), _T("\"%s\\%s\" \""), exeFolder, jsFileName);
	if (isSimu == TRUE) {
		_tcscat_s(jsFullFileName, NItemsArray(jsFullFileName), exeFolder);
		_tcscat_s(jsFullFileName, NItemsArray(jsFullFileName), _T("\\"));
	}
	_tcscat_s(jsFullFileName, NItemsArray(jsFullFileName), pConfigFileName);
	_tcscat_s(jsFullFileName, NItemsArray(jsFullFileName), _T("\""));

	//printf("Javascript full path: %ws\n", &jsFullFileName);

	//...preparamos linea de comandos...
	_tcscpy_s(cScriptCmd, NItemsArray(cScriptCmd), _T("cscript.exe //E:jscript /D "));
	_tcscat_s(cScriptCmd, NItemsArray(cScriptCmd), jsFullFileName);

	printf("Command: %ws\n", &cScriptCmd);

	//Ejecucion del javascript. Recibimos parmetros mediante un PIPE.
	if ((pPipe = _tpopen(cScriptCmd, _T("rt"))) == NULL)
		return(FMM_RET_CODE_ERR_CONFIG_NOT_OBTAINED);

	/* Me salto los caracteres iniciales hasta encontrar la marca de inicio "#####"*/
	do {
		car = fgetwc(pPipe);
	} while ((car != L'#') && (ferror(pPipe) == 0) && (feof(pPipe) == 0));
	//Antes de buscar el siguiente #, verificar que no hay error y aún no se ha llegado al fin del archivo
	if ((ferror(pPipe) == 0) && (feof(pPipe) == 0)) {
		do {
			car = fgetwc(pPipe);
		} while ((car == L'#') && (ferror(pPipe) == 0) && (feof(pPipe) == 0));
	}

	//Por si hay un error tras la marca de inicio por algn fallo al abrir el fichero XML en el javascript.
	if ((car == 'E') && (fwscanf_s(pPipe, L"RROR") != 0)) {
		_pclose(pPipe);
		return(FMM_RET_CODE_ERR_CONFIG_NOT_OBTAINED);
	}

	//Primera linea del PIPE. Version del algoritmo y Numero total de parametros.
	fwscanf_s(pPipe, L"\nalgorithmVersion = \'%[^']\'", algorithmVersion, _countof(algorithmVersion));
	fwscanf_s(pPipe, L"\nalgorithmEnable = \'%[^']\'", algorithmEnable, _countof(algorithmEnable));
	fwscanf_s(pPipe, L"\ncloseFile = \'%[^']\'", closeFile, _countof(closeFile));
	fwscanf_s(pPipe, L"\nwarningMessage = \'%[^']\'", warningMessage, _countof(warningMessage));
	fscanf_s(pPipe, "\noptitwinPort = \'%[^']\'", optitwinPort, _countof(optitwinPort));
	fwscanf_s(pPipe, L"\nfolderCSV = \'%[^']\'", folderCSV, _countof(folderCSV));
	if (_tcscmp(folderCSV, _T("null")) == 0)
		_tcscpy(folderCSV, _T(""));
	fwscanf_s(pPipe, L"\nTOTAL_PARAMS = %d", &totalParams);

	//Obtener la información de cada parámetro
	if (totalParams > 0 && totalParams < MAX_VARS) {
		int i;
		for (i = 0; i < totalParams; i++) {
			fwscanf_s(pPipe, L"\nvarName = \'%[^']\'", vars.var[i].varName, _countof(vars.var[i].varName)); //STRINGS
			fwscanf_s(pPipe, L"\nfactor = %f", &vars.var[i].factor); //float
		}
	}
	else {
		_pclose(pPipe);
		return FMM_RET_CODE_ERR_CONFIG_NOT_OBTAINED;
	}
	// Escribir Linea de final de fichero.
	fwscanf_s(pPipe, L"\nEOF %d", &retcode); //END OF FILE

	_pclose(pPipe);
	return FMM_RET_CODE_OK;
}
/* -----------------------------------------------------------------------------------------
 * -- Descripcion:		Gestin de los handlers de los pipes.
						Si hay 1 pipe de escritura en driver y lectura en aplicacion se necesitan 2 hnd
						El primero se abre para leer valores desde aplicacin
						El segundo se abre para escribir valores desde el driver y por lo tanto se
						debe escribir dicho valor en el driver antes de que empiece el real time "StartRealTime"
 * -- Parametros:	Punteros: Handlers de los pipes.
 * -- Valor devuelto:
 */
int	streamManagement(streamHandlers_t* pStreamHnds) {
	wchar_t pipeName[] = L"Pipe" SOLUTION_NAME;
	unsigned short nTimes = 0;
	FSYS_retCode_t retCode = FSYS_RET_CODE_OK;
	do {
		retCode = FSYS_OpenStream(&pStreamHnds->streamHndWrDriver, pipeName, L"w", (STREAM_SIZE));
		if (retCode != FSYS_RET_CODE_OK) {
			wchar_t buffer[16];
			_itow_s(nTimes, buffer, 16, 10);
			wcscat_s(pipeName, NItemsArray(pipeName), buffer);
		}
		nTimes++;
	} while ((retCode != FSYS_RET_CODE_OK) && (nTimes < MAX_REPETITIONS));
	if (retCode != FSYS_RET_CODE_OK) {
		printf("\nPlease, Restart Windows. Max write Streams Reached");
		printf("\nInternal ERROR. write Stream not opened. retcode = %d", retCode);
		return(retCode);
	}

	retCode = FSYS_OpenStream(&pStreamHnds->streamHndRdApp, pipeName, L"r", (STREAM_SIZE));
	if (retCode != FSYS_RET_CODE_OK) {
		printf("\nInternal ERROR. read Stream not opened. retcode = %d", retCode);
		return(retCode);
	}

	//Escribir el valor del handler de escritura en el handler del stream del driver, para FCIO_WRITE
	retCode = FSYS_WriteVariableEx(L"RTStreamHndWrite", &pStreamHnds->streamHndWrDriver, sizeof(pStreamHnds->streamHndWrDriver));
	if (retCode != FSYS_RET_CODE_OK) {
		printf("\nInternal ERROR: RTStreamHndWrite not initialized. retcode = %d", retCode);
		return(retCode);
	}
	return(FSYS_RET_CODE_OK);
}
/* -----------------------------------------------------------------------------------------
 * -- Descripcion:		Inicializar el numero de scan sincronizado con el ciclo del CNC
 * -- Parametros:	Punteros: Handlers de los pipes.
 * -- Valor devuelto:
 */
FSYS_retCode_t nScanHandlerInitialize(FSYS_varHnd_t* pHndNScan)
{
	FSYS_retCode_t retCode = FSYS_RET_CODE_OK;

	retCode = FSYS_OpenVariable(pHndNScan, L"KNL_SCAN_NUMBER", FSYS_VAR_OPEN_FOR_READING);
	if (retCode != FSYS_RET_CODE_OK)
		printf("\nInternal ERROR: KNL_SCAN_NUMBER not initialized. retcode = %d", retCode);
	retCode = FSYS_WriteVariableEx(L"hndRTNScan", pHndNScan, sizeof(FSYS_varHnd_t));
	if (retCode != FSYS_RET_CODE_OK)
		printf("\nInternal ERROR: RTNSCANS not initialized. retcode = %d", retCode);

	return(retCode);
}
/*----------------------------------------------------------------------------------------*/
/*																FUNCIONES LOCALES		  */
/*----------------------------------------------------------------------------------------*/
/******************************************************************************************/
/*                                FIN						                              */
/******************************************************************************************/
