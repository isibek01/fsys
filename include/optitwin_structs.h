/*******************************************************************************************
*                              NOMBREFICH	optitwin_structs							   *
********************************************************************************************
* AUTOR:			José Joaquín Peralta Abadía (MU)
*
* DESCRIPCION: Structs para comunicación entre app y RT
*
*******************************************************************************************/
/*----------------------------------------------------------------------------------------*/
/*																			INCLUDES	  */
/*----------------------------------------------------------------------------------------*/
//#include <tchar.h>
#include "fsys.h"
#include "fmm_interface.h"

/*----------------------------------------------------------------------------------------*/
/*																	CONSTANTES			  */
/*----------------------------------------------------------------------------------------*/
#define SIMU  0

/* define NULL pointer value */
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#define MAX_STRING			512
#define MAX_VARS			100
#define SOCKETPORT			"12589"
#define APP_ID				0X0F71

#ifndef FALSE
#define FALSE               0
#endif
#ifndef TRUE
#define TRUE                1
#endif
/*----------------------------------------------------------------------------------------*/
/*                               TIPOS													  */
/*----------------------------------------------------------------------------------------*/
typedef int                 BOOL;

typedef struct datos_st {
    FSYS_varHnd_t var[MAX_VARS];
}datos_t;

typedef struct valores_st {
    double var[MAX_VARS];
}valores_t;

typedef struct var_info_st {
	wchar_t			varName[MAX_STRING];
	float			factor;
}var_info_t;

typedef struct var_st {
	var_info_t		var[MAX_VARS];
}var_t;

struct EventInfo_st {
	unsigned short		appId;	//Identificacin del datalogger, para compartir el pipe con otros
	unsigned short		enable;		//Identificacin de la variable registrada. Qu id es, de la definicin de eventos, puede ser un ordinal. 
	unsigned long		nScan;	//Identificacin - Nmero de scan del evento
	//unsigned long		date; //Fecha del evento
	union v_u {
		double iValue;				//Valor entero de la variables monitorizada
	} v[MAX_VARS];
}EventInfo_t;

typedef struct streamHandlers_st {
	FSYS_streamHnd_t		streamHndRdApp;		//Handler del STREAM para leer desde aplicacion
	FSYS_streamHnd_t		streamHndWrDriver;	//Handler del STREAM para escribir desde el driver.
} streamHandlers_t;

/*----------------------------------------------------------------------------------------*/
/*											PROTOTIPOS DE FUNCIONES Y MACROS LOCALES	  */
/*----------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------*/
/*														FUNCIONES PUBLICAS				  */
/*----------------------------------------------------------------------------------------*/
//Init
int streamManagement(streamHandlers_t* pStreamHnds);
FSYS_retCode_t nScanHandlerInitialize(FSYS_varHnd_t* pHndNScan);
/*----------------------------------------------------------------------------------------*/
/*																FUNCIONES LOCALES		  */
/*----------------------------------------------------------------------------------------*/
/******************************************************************************************/
/*                                FIN						                              */
/******************************************************************************************/
