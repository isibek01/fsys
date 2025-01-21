/*******************************************************************************************
*                              NOMBREFICH												   *
********************************************************************************************
* AUTOR:				Fagor Automation
* DESCRIPCION:			Example of user application code
* IMPLEMENTACION:
* MODIFICACIONES:
*******************************************************************************************/

/*----------------------------------------------------------------------------------------*/
/*                              INCLUDES												  */
/*----------------------------------------------------------------------------------------*/
#include "optitwin_structs.h"
#include "fsyscom.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>			//getch y _kbhit

//Para sockets que se usaran para comunicarse con el gestor de proyectos 
//de OptiTwin, de momento solo se usa para obtener la fecha y hora actual del sistema
#include <winsock2.h>
/*----------------------------------------------------------------------------------------*/
/*                             CONSTANTES												  */
/*----------------------------------------------------------------------------------------*/
const wchar_t driverName[] = L"V" SOLUTION_NAME L"D";
#define ESCAPE_KEY				27

/*----------------------------------------------------------------------------------------*/
/*                               TIPOS													  */
/*----------------------------------------------------------------------------------------*/
typedef struct rxInfo_st {
	struct EventInfo_st		infoRxViaPipe[MAX_VARS];	//Estructura de datos recibida - leida del pipe - stream
	size_t					infoRxViaPipeSize;
	size_t					infoRxViaPipeItemSize;
	size_t					infoRxViaPipeElementCount;
	size_t					nBytesRx;
	size_t					nItemsRx;
} rxInfo_t;
/*----------------------------------------------------------------------------------------*/
/*                             VARIABLES												  */
/*----------------------------------------------------------------------------------------*/
TCHAR algorithmEnable[MAX_STRING] = _T("");
TCHAR closeFile[MAX_STRING] = _T("");
TCHAR warningMessage[MAX_STRING] = _T("");
TCHAR folderCSV[MAX_STRING] = _T("");
var_t vars;
int timestamp = 0;
int vb = 0;
char optitwinPort[64] = "";
volatile int escapePressed = 0;

streamHandlers_t streamHnds;
rxInfo_t rxInfo;

//Para sockets que se usaran para comunicarse con el gestor de proyectos 
//de OptiTwin, de momento no se usa
WSADATA wsaData;
int iResult;
int server_socket;
SOCKADDR_IN server_address;

/*----------------------------------------------------------------------------------------*/
/*              PROTOTIPOS DE FUNCIONES Y MACROS LOCALES								  */
/*----------------------------------------------------------------------------------------*/
int				keyboardHitGet						(void);
fmmRetCode_t	getConfig							(TCHAR* pConfigFileName, BOOL isSimu);
/*----------------------------------------------------------------------------------------*/
/*                         FUNCIONES PUBLICAS											  */
/*----------------------------------------------------------------------------------------*/

/* -----------------------------------------------------------------------------------------
 * -- Description:		Entry point of the application
 * -- Parameters:		argc: number of arguments
						argv: array of pointer pointing to the arguments
						pCtrl: pointer to a control variable
							when the value pointed by pCtrl is FSYS_APP_CTRL_STOP
							rutine should end and return to the caller
 * -- Returned Value: 
 * -- Asunciones: 
 * -- Postconditions: 
 * -- Implementation: 
 */
FAGOR_LINK_FMM int main(int argc, _TCHAR * argv[], FSYS_applicationControl_t * pCtrl) {
	int keyboardChar = 0;

	printf("===================================\nStart App\n===================================\n");
	 
	fmmRetCode_t isError;
	struct datos_st pDatos = FSYS_VOID_VAR_HND;
	int i;
	fmmStatus_t	retStatus = FMM_STATUS_RUNNING;
	FILE* csvFile;
	TCHAR filePath[MAX_PATH];

	//Handlers que se usarán para lectura de datos del módulo RT
	FSYS_varHnd_t hndRdEnable = FSYS_VOID_VAR_HND;
	FSYS_varHnd_t hndRdClose = FSYS_VOID_VAR_HND;

	//Handlers que se usarán para escritura de datos al módulo RT
	//FSYS_varHnd_t hndWrVB = FSYS_VOID_VAR_HND;
	FSYS_varHnd_t hndWrWarn = FSYS_VOID_VAR_HND;

	FSYS_VarRdWr_t rd = FSYS_VAR_OPEN_FOR_READING;
	FSYS_VarRdWr_t wr = FSYS_VAR_OPEN_FOR_WRITTING;
	int retCode = 0;

	//Init
	memset(&streamHnds, 0, sizeof(streamHnds));
	memset(&rxInfo, 0, sizeof(rxInfo));
	rxInfo.infoRxViaPipeSize = sizeof(rxInfo.infoRxViaPipe);
	rxInfo.infoRxViaPipeItemSize = sizeof(rxInfo.infoRxViaPipe[0]);
	rxInfo.infoRxViaPipeElementCount = NItemsArray(rxInfo.infoRxViaPipe);

	//Get Configuration
	isError = getConfig(_T("optitwinConfig.xml"), TRUE);
	if (isError != FMM_RET_CODE_OK) {
		printf("\nERROR: CONFIG WRONG INIT");
		return -1;
	}
	printf("Algorithm enable: %ws\n", &algorithmEnable);
	printf("Close file: %ws\n", &closeFile);
	printf("Optitwin Port: %s\n", optitwinPort);

	//Llamar a FMM_initialize del driver
	FMM_InitializaRealTime();

	//Requests port
	createServer(SOCKETPORT);
	//Init Streams
	retCode = streamManagement(&streamHnds);
	if (retCode != FSYS_RET_CODE_OK) {
		printf("\nERROR: STREAM WRONG INIT");
		return -1;
	}

	//abrimos las variables de lectura. solo se puede abrir desde aplicacion
	for (i = 0; i < MAX_VARS; ++i) {
		if (vars.var[i].varName != NULL && //Si la variable está definida y
			vars.var[i].varName != '\0' && wcslen(vars.var[i].varName) > 0) { //Si la variable no es vacia (Since C-style strings are always terminated with the null character '\0')
			retCode = FSYS_OpenVariable(&(pDatos.var[i]), vars.var[i].varName, rd);
			printf("\nReading var: %ws - name length: %d", &vars.var[i].varName, wcslen(vars.var[i].varName));
		}
		else
			break;
	}
	retCode = FSYS_OpenVariable(&hndRdEnable, algorithmEnable, rd);
	retCode = FSYS_OpenVariable(&hndRdClose, closeFile, rd);

	//Abrimos la variable de escritura, solo se puede abrir desde aplicacion
	//retCode = FSYS_OpenVariable(&hndWrVB, vb, wr); //Desgaste en um
	retCode = FSYS_OpenVariable(&hndWrWarn, warningMessage, wr); //Código de warning configurado en el PLC

	// Get current date and time
	time_t now = time(NULL);
	struct tm* timeInfo = localtime(&now);

	printf("\nCSV will be saved in: %ws\n", folderCSV);

	printf("\n===============================\nPress any key to continue...\n\n");
	getchar();

	_stprintf(filePath, _T("%s%d-%d.csv"), folderCSV, getDate(), getTime());  // Construct full file path
	csvFile = _tfopen(&filePath, _T("a"));  // Open CSV file in append mode
	if (csvFile == NULL) {
		printf("Error opening CSV file\n");
		return 1;  // Exit with error if opening fails
	}
	fprintf(csvFile, "time");  // Print headers for common fields
	for (int j = 0; j < MAX_VARS; j++) {
		if (vars.var[j].varName != NULL && vars.var[j].varName != '\0' && wcslen(vars.var[j].varName) > 0) {
			fprintf(csvFile, ",%ws", vars.var[j].varName);  // Add headers for variable names
		}
	}

	//escribimos los handlers obtenidos anteriormente en el driver de usuario
	//para que el driver ejectue las lecturas y escrituras
	retCode = FSYS_WriteVariableEx(L"vars", &vars, sizeof(var_t));  // Con esto mandas la estructura a la parte de real time.
	retCode = FSYS_WriteVariableEx(L"varStruct", &pDatos, sizeof(struct datos_st));  // Con esto mandas la estructura a la parte de real time.
	retCode = FSYS_WriteVariableEx(L"hndInputEnable", &hndRdEnable, sizeof(hndRdEnable));
	retCode = FSYS_WriteVariableEx(L"hndInputClose", &hndRdClose, sizeof(hndRdClose));
	//retCode = FSYS_WriteVariableEx(L"hndOutputVB", &hndWrVB, sizeof(hndWrVB));
	retCode = FSYS_WriteVariableEx(L"hndOutputWarning", &hndWrWarn, sizeof(hndWrWarn));
	
	//inicializamos el tiempo real
	FMM_StartRealTime();
	
	//Definimos una manera de salir de la aplicación 
	//para que se cierren los handlers con la tecla escape
	//como está hecho en el datalogger
	printf("\nPress ESCAPE to exit.");
	while(escapePressed == 0 && retStatus != FMM_STATUS_CLOSING)
	{
		keyboardChar = keyboardHitGet();
		if (keyboardChar == ESCAPE_KEY)
			escapePressed = 1;
		else
		{
			retCode = FSYS_ReadStream(rxInfo.infoRxViaPipe, rxInfo.infoRxViaPipeItemSize, rxInfo.infoRxViaPipeElementCount,
				&streamHnds.streamHndRdApp, &rxInfo.nBytesRx);
			if (retCode != FSYS_RET_CODE_OK)
			{
				printf("\nERROR while reading data stream. retcode = %d", retCode);
				continue;
			}

			rxInfo.nItemsRx = rxInfo.nBytesRx / rxInfo.infoRxViaPipeItemSize;
			/*printf("\n===================\nTotal bytes received %u", rxInfo.nBytesRx);
			printf("\nItem size %u", rxInfo.infoRxViaPipeItemSize);
			printf("\nNum items %u", rxInfo.nItemsRx);
			printf("\n===================");*/

			if ((rxInfo.nItemsRx > 0) && (rxInfo.nItemsRx <= rxInfo.infoRxViaPipeElementCount)) {
				for (i = 0; i < rxInfo.nItemsRx; i++) {
					if (rxInfo.infoRxViaPipe[i].appId == APP_ID) {
						printf("\nScan Number %u:", rxInfo.infoRxViaPipe[i].nScan);
						//Si recibimos el valor 1 en enable, podemos hacer lectura
						if (rxInfo.infoRxViaPipe[i].enable == 1)
						{
							//Si el fichero CSV es NULL, quiere decir que fue cerrado y hay que volverlo a abrir
							//TODO: Este if está pendiente de probar
							if (csvFile == NULL)
							{
								_stprintf(filePath, _T("%s%d-%d.csv"), folderCSV, getDate(), getTime());  // Construct full file path
								csvFile = _tfopen(&filePath, _T("a"));  // Open CSV file in append mode
								if (csvFile == NULL) {
									printf("Error opening CSV file\n");
									return 1;  // Exit with error if opening fails
								}
								fprintf(csvFile, "time");  // Print headers for common fields
								for (int j = 0; j < MAX_VARS; j++) {
									if (vars.var[j].varName != NULL && vars.var[j].varName != '\0' && wcslen(vars.var[j].varName) > 0) {
										fprintf(csvFile, ",%ws", vars.var[j].varName);  // Add headers for variable names
									}
								}
							}
							// Append to CSV file
							fprintf(csvFile, "\n%u", rxInfo.infoRxViaPipe[i].nScan);
							for (int j = 0; j < MAX_VARS; j++) {
								if (vars.var[j].varName != NULL && vars.var[j].varName != '\0' && wcslen(vars.var[j].varName) > 0) {
									printf(" %f", rxInfo.infoRxViaPipe[i].v[j].iValue);

									// Append to CSV file
									fprintf(csvFile, ",%f", rxInfo.infoRxViaPipe[i].v[j].iValue);
								}
							}
						}
						//Si se recibe el valor 2 en enable, debemos cerrar el fichero
						//TODO: Esto hace falta probarlo
						else if (rxInfo.infoRxViaPipe[i].enable == 2 && csvFile != NULL)
						{
							fclose(csvFile);  // Close the CSV file
							csvFile = NULL;
						}
						//Si enable tiene cualquier otro valor, no hacemos nada, no hay que leer
					}
					else {
						printf("\nERROR: wrong app id. Possible buffer overflow - ");
						printf("AppId %x Scan Number %u: %f", rxInfo.infoRxViaPipe[i].appId, rxInfo.infoRxViaPipe[i].nScan, rxInfo.infoRxViaPipe[i].v[0].iValue);
						if (csvFile != NULL)
						{
							fclose(csvFile);  // Close the CSV file
							csvFile = NULL;
						}
					}
				}
				memset(&rxInfo.infoRxViaPipe, 0, sizeof(rxInfo.infoRxViaPipe));
			}
			Sleep(1);

			retStatus = FMM_GetStatus();
		}
	}
	printf("\nServers pre-close. ");
	closeServer();

	if (csvFile != NULL)
	{
		fclose(csvFile);  // Close the CSV file
		csvFile = NULL;
	}

	FMM_StopRealTime();
	FMM_TerminateRealTime();

	for (i = 0; i < MAX_VARS; ++i) {
		if (vars.var[i].varName != NULL && //Si la variable está definida y
			vars.var[i].varName != '\0' &&
			wcslen(vars.var[i].varName) > 0) { //Si la variable no es vacia (Since C-style strings are always terminated with the null character '\0')
			FSYS_CloseVariable(&(pDatos.var[i]));
		}
		else
			break;
	}

	FSYS_CloseVariable(&hndRdEnable);
	FSYS_CloseVariable(&hndRdClose);

	//FSYS_CloseVariable(&hndWrVB);
	FSYS_CloseVariable(&hndWrWarn);
	return(0);
}

/*----------------------------------------------------------------------------------------*/
/*                         FUNCIONES LOCALES											  */
/*----------------------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//FUNCION no bloqueante que lee un caracter pulsado desde el teclado
//Parámetros entrada.						
//Parámetros salida				caracter ASCII pulsado en teclado. TECLA ESCAPE = 27
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int keyboardHitGet()
{
    if (_kbhit())			//Detecta pulsacion teclado
        return _getch();	//Devuelve tecla pulsada
    else
        return -1;
}

int getDate()
{
	int date = 0;

	SYSTEMTIME st;
	GetLocalTime(&st);

	//date = aaaammdd
	date = (int)((st.wYear * 10000) + (st.wMonth * 100) + st.wDay);

	return date;
}

int getTime()
{
	int date = 0;

	SYSTEMTIME st;
	GetLocalTime(&st);

	// date = HHMMSS
	date = (int)((st.wHour * 10000) + (st.wMinute * 100) + (st.wSecond));

	return date;
}

/******************************************************************************************/
/*                                FIN						                              */
/******************************************************************************************/
