/*******************************************************************************
** File: sample_radio_app.h
**
** Purpose:
**   This is the main header file for the SAMPLE_RADIO application.
**
*******************************************************************************/
#ifndef _SAMPLE_RADIO_APP_H_
#define _SAMPLE_RADIO_APP_H_

/*
** Include Files
*/
#include "cfe.h"
#include "sample_radio_device.h"
#include "sample_radio_events.h"
#include "sample_radio_platform_cfg.h"
#include "sample_radio_perfids.h"
#include "sample_radio_msg.h"
#include "sample_radio_msgids.h"
#include "sample_radio_version.h"
#include "hwlib.h"

/* TODO: This is specific to the sample_radio application, remove if using template generator */
#include "mgr_msg.h"
#include "mgr_msgids.h"

/*
** Specified pipe depth - how many messages will be queued in the pipe
*/
#define SAMPLE_RADIO_PIPE_DEPTH 32

/*
** Enabled and Disabled Definitions
*/
#define SAMPLE_RADIO_DEVICE_DISABLED 0
#define SAMPLE_RADIO_DEVICE_ENABLED  1

/*
** SAMPLE_RADIO global data structure
** The cFE convention is to put all global app data in a single struct.
** This struct is defined in the `sample_radio_app.h` file with one global instance
** in the `.c` file.
*/
typedef struct
{
    /*
    ** Housekeeping telemetry packet
    ** Each app defines its own packet which contains its OWN telemetry
    */
    SAMPLE_RADIO_Hk_tlm_t HkTelemetryPkt; /* SAMPLE_RADIO Housekeeping Telemetry Packet */

    /*
    ** Operational data  - not reported in housekeeping
    */
    CFE_MSG_Message_t *MsgPtr;    /* Pointer to msg received on software bus */
    CFE_SB_PipeId_t    CmdPipe;   /* Pipe Id for HK command pipe */
    uint32             RunStatus; /* App run status for controlling the application state */

    /*
     ** Device data
     ** TODO: Make specific to your application
     */
    SAMPLE_RADIO_Device_tlm_t DevicePkt; /* Device specific data packet */

    /*
    ** Device protocol
    ** TODO: Make specific to your application
    */
    uart_info_t Sample_radioUart; /* Hardware protocol definition */

} SAMPLE_RADIO_AppData_t;

/*
** Exported Data
** Extern the global struct in the header for the Unit Test Framework (UTF).
*/
extern SAMPLE_RADIO_AppData_t SAMPLE_RADIO_AppData; /* SAMPLE_RADIO App Data */

/*
**
** Local function prototypes.
**
** Note: Except for the entry point (SAMPLE_RADIO_AppMain), these
**       functions are not called from any other source module.
*/
void  SAMPLE_RADIO_AppMain(void);
int32 SAMPLE_RADIO_AppInit(void);
void  SAMPLE_RADIO_ProcessCommandPacket(void);
void  SAMPLE_RADIO_ProcessGroundCommand(void);
void  SAMPLE_RADIO_ProcessTelemetryRequest(void);
void  SAMPLE_RADIO_ReportHousekeeping(void);
void  SAMPLE_RADIO_ReportDeviceTelemetry(void);
void  SAMPLE_RADIO_ResetCounters(void);
void  SAMPLE_RADIO_Enable(void);
void  SAMPLE_RADIO_Disable(void);
void  SAMPLE_RADIO_Configure(void);
int32 SAMPLE_RADIO_VerifyCmdLength(CFE_MSG_Message_t *msg, uint16 expected_length);

/* TODO: This is specific to the sample_radio application, remove if using template generator */
void SAMPLE_RADIO_ProcessMgrHk(void);

#endif /* _SAMPLE_RADIO_APP_H_ */
