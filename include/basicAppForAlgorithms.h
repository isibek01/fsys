/*******************************************************************************************
*                              NOMBREFICH												   *
********************************************************************************************
* AUTOR:			Fagor Automation
* DESCRIPCION:
* IMPLEMENTACION:
* MODIFICACIONES:
*******************************************************************************************/

/*----------------------------------------------------------------------------------------*/
/*                              INCLUDES												  */
#include <tchar.h>
#include "fsys.h"
#include "fmm_interface.h"
/*----------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------*/
/*                             CONSTANTES												  */
/*----------------------------------------------------------------------------------------*/
#ifndef FALSE
#define FALSE               0
#endif
#ifndef TRUE
#define TRUE                1
#endif

#define APP_ID				0XFEDC
/*----------------------------------------------------------------------------------------*/
/*                               TIPOS													  */
/*----------------------------------------------------------------------------------------*/
typedef struct streamHandlers_st {
	FSYS_streamHnd_t		streamHndRdApp;		//Handler del STREAM para leer desde aplicacion
	FSYS_streamHnd_t		streamHndWrDriver;	//Handler del STREAM para escribir desde el driver.
} streamHandlers_t;

struct EventInfo_st {
	unsigned short		appId;	//Identificacin del datalogger, para compartir el pipe con otros
	unsigned short		varId;		//Identificacin de la variable registrada. Qu id es, de la definicin de eventos, puede ser un ordinal. 
	unsigned long		nScan;	//Identificacin - Nmero de scan del evento
	union v_u {
		int iValue;				//Valor entero de la variables monitorizada
	} v;
};
/*----------------------------------------------------------------------------------------*/
/*														FUNCIONES PUBLICAS				  */
/*----------------------------------------------------------------------------------------*/
//Init
int streamManagement(streamHandlers_t* pStreamHnds);
FSYS_retCode_t nScanHandlerInitialize(FSYS_varHnd_t* pHndNScan);
