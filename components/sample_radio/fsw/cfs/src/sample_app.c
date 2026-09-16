/*******************************************************************************
** File: sample_radio_app.c
**
** Purpose:
**   This file contains the source code for the SAMPLE_RADIO application.
**
*******************************************************************************/

/*
** Include Files
*/
#include <arpa/inet.h>
#include "sample_radio_app.h"

/*
** Global Data
*/
SAMPLE_RADIO_AppData_t SAMPLE_RADIO_AppData;

/*
** Application entry point and main process loop
*/
void SAMPLE_RADIO_AppMain(void)
{
    int32 status = OS_SUCCESS;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(SAMPLE_RADIO_PERF_ID);

    /*
    ** Perform application initialization
    */
    status = SAMPLE_RADIO_AppInit();
    if (status != CFE_SUCCESS)
    {
        SAMPLE_RADIO_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** Main loop
    */
    while (CFE_ES_RunLoop(&SAMPLE_RADIO_AppData.RunStatus) == true)
    {
        /*
        ** Performance log exit stamp
        */
        CFE_ES_PerfLogExit(SAMPLE_RADIO_PERF_ID);

        /*
        ** Pend on the arrival of the next Software Bus message
        ** Note that this is the standard, but timeouts are available
        */
        status = CFE_SB_ReceiveBuffer((CFE_SB_Buffer_t **)&SAMPLE_RADIO_AppData.MsgPtr, SAMPLE_RADIO_AppData.CmdPipe,
                                      CFE_SB_PEND_FOREVER);

        /*
        ** Begin performance metrics on anything after this line. This will help to determine
        ** where we are spending most of the time during this app execution.
        */
        CFE_ES_PerfLogEntry(SAMPLE_RADIO_PERF_ID);

        /*
        ** If the CFE_SB_ReceiveBuffer was successful, then continue to process the command packet
        ** If not, then exit the application in error.
        ** Note that a SB read error should not always result in an app quitting.
        */
        if (status == CFE_SUCCESS)
        {
            SAMPLE_RADIO_ProcessCommandPacket();
        }
        else
        {
            CFE_EVS_SendEvent(SAMPLE_RADIO_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "SAMPLE_RADIO: SB Pipe Read Error = %d",
                              (int)status);
            SAMPLE_RADIO_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Disable component, which cleans up the interface, upon exit
    */
    SAMPLE_RADIO_Disable();

    /*
    ** Performance log exit stamp
    */
    CFE_ES_PerfLogExit(SAMPLE_RADIO_PERF_ID);

    /*
    ** Exit the application
    */
    CFE_ES_ExitApp(SAMPLE_RADIO_AppData.RunStatus);
}

/*
** Initialize application
*/
int32 SAMPLE_RADIO_AppInit(void)
{
    int32 status = OS_SUCCESS;

    SAMPLE_RADIO_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY); /* as default, no filters are used */
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("SAMPLE_RADIO: Error registering for event services: 0x%08X\n", (unsigned int)status);
        return status;
    }

    /*
    ** Create the Software Bus command pipe
    */
    status = CFE_SB_CreatePipe(&SAMPLE_RADIO_AppData.CmdPipe, SAMPLE_RADIO_PIPE_DEPTH, "SAMPLE_RADIO_CMD_PIPE");
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_RADIO_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "Error Creating SB Pipe,RC=0x%08X",
                          (unsigned int)status);
        return status;
    }

    /*
    ** Subscribe to ground commands
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SAMPLE_RADIO_CMD_MID), SAMPLE_RADIO_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_RADIO_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Error Subscribing to HK Gnd Cmds, MID=0x%04X, RC=0x%08X", SAMPLE_RADIO_CMD_MID,
                          (unsigned int)status);
        return status;
    }

    /*
    ** Subscribe to housekeeping (hk) message requests
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(SAMPLE_RADIO_REQ_HK_MID), SAMPLE_RADIO_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_RADIO_SUB_REQ_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Error Subscribing to HK Request, MID=0x%04X, RC=0x%08X", SAMPLE_RADIO_REQ_HK_MID,
                          (unsigned int)status);
        return status;
    }

    /*
    ** Subscribe to MGR HK for Science Pass Information
    ** TODO: This is specific to the sample_radio application, remove if using template generator
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MGR_HK_TLM_MID), SAMPLE_RADIO_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_RADIO_SUB_REQ_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Error Subscribing to HK Request, MID=0x%04X, RC=0x%08X", MGR_HK_TLM_MID,
                          (unsigned int)status);
        return status;
    }

    /*
    ** TODO: Subscribe to any other messages here
    */

    /*
    ** Initialize the published HK message - this HK message will contain the
    ** telemetry that has been defined in the SAMPLE_RADIO_HkTelemetryPkt for this app.
    */
    CFE_MSG_Init(CFE_MSG_PTR(SAMPLE_RADIO_AppData.HkTelemetryPkt.TlmHeader), CFE_SB_ValueToMsgId(SAMPLE_RADIO_HK_TLM_MID),
                 SAMPLE_RADIO_HK_TLM_LNGTH);

    /*
    ** Initialize the device packet message
    ** This packet is specific to your application
    */
    CFE_MSG_Init(CFE_MSG_PTR(SAMPLE_RADIO_AppData.DevicePkt.TlmHeader), CFE_SB_ValueToMsgId(SAMPLE_RADIO_DEVICE_TLM_MID),
                 SAMPLE_RADIO_DEVICE_TLM_LNGTH);

    /*
    ** TODO: Initialize any other messages that this app will publish
    */

    /*
    ** Always reset all counters during application initialization
    */
    SAMPLE_RADIO_ResetCounters();

    /*
    ** Initialize application data
    ** Note that counters are excluded as they were reset in the previous code block
    */
    SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceEnabled          = SAMPLE_RADIO_DEVICE_DISABLED;
    SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceHK.DeviceCounter = 0;
    SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceHK.DeviceConfig  = 0;
    SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceHK.DeviceStatus  = 0;

    /*
     ** Send an information event that the app has initialized.
     ** This is useful for debugging the loading of individual applications.
     */
    status = CFE_EVS_SendEvent(SAMPLE_RADIO_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION,
                               "SAMPLE_RADIO App Initialized. Version %d.%d.%d.%d", SAMPLE_RADIO_MAJOR_VERSION,
                               SAMPLE_RADIO_MINOR_VERSION, SAMPLE_RADIO_REVISION, SAMPLE_RADIO_MISSION_REV);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("SAMPLE_RADIO: Error sending initialization event: 0x%08X\n", (unsigned int)status);
    }
    return status;
}

/*
** Process packets received on the SAMPLE_RADIO command pipe
*/
void SAMPLE_RADIO_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_GetMsgId(SAMPLE_RADIO_AppData.MsgPtr, &MsgId);
    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        /*
        ** Ground Commands with command codes fall under the SAMPLE_RADIO_CMD_MID (Message ID)
        */
        case SAMPLE_RADIO_CMD_MID:
            SAMPLE_RADIO_ProcessGroundCommand();
            break;

        /*
        ** Housekeeping requests with command codes fall under the SAMPLE_RADIO_REQ_HK_MID (Message ID)
        */
        case SAMPLE_RADIO_REQ_HK_MID:
            SAMPLE_RADIO_ProcessTelemetryRequest();
            break;

        /*
        ** Update science pass information
        ** TODO: This is specific to the sample_radio application, remove if using template generator
        */
        case MGR_HK_TLM_MID:
            SAMPLE_RADIO_ProcessMgrHk();
            break;

        /*
        ** TODO: Add additional message IDs as needed
        */

        /*
        ** All other invalid messages that this app doesn't recognize,
        ** increment the command error counter and log as an error event.
        */
        default:
            /* Increment the command error counter upon receipt of an invalid command packet */
            SAMPLE_RADIO_AppData.HkTelemetryPkt.CommandErrorCount++;

            /* Send event failure to the console*/
            CFE_EVS_SendEvent(SAMPLE_RADIO_PROCESS_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE_RADIO: Invalid command packet, MID = 0x%x", CFE_SB_MsgIdToValue(MsgId));
            break;
    }
    return;
}

/*
** Process ground commands
** TODO: Add additional commands required by the specific component
*/
void SAMPLE_RADIO_ProcessGroundCommand(void)
{
    CFE_SB_MsgId_t    MsgId       = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode = 0;

    /*
    ** MsgId is only needed if the command code is not recognized. See default case
    */
    CFE_MSG_GetMsgId(SAMPLE_RADIO_AppData.MsgPtr, &MsgId);

    /*
    ** Ground Commands have a command code (_CC) associated with them
    ** Pull this command code from the message and then process
    */
    CFE_MSG_GetFcnCode(SAMPLE_RADIO_AppData.MsgPtr, &CommandCode);
    switch (CommandCode)
    {
        /*
        ** NOOP Command
        */
        case SAMPLE_RADIO_NOOP_CC:
            /*
            ** Verify the command length immediately after CC identification
            */
            if (SAMPLE_RADIO_VerifyCmdLength(SAMPLE_RADIO_AppData.MsgPtr, sizeof(SAMPLE_RADIO_NoArgs_cmd_t)) == OS_SUCCESS)
            {
#ifdef SAMPLE_RADIO_CFG_DEBUG
                OS_printf("SAMPLE_RADIO: SAMPLE_RADIO_NOOP_CC received \n");
#endif

                /* Do any necessary checks, none for a NOOP */

                /* Increment command success or error counter, NOOP can only be successful */
                SAMPLE_RADIO_AppData.HkTelemetryPkt.CommandCount++;

                /* Do the action, none for a NOOP */

                /* Increment device success or error counter, none for NOOP as application only */

                /* Send event success or failure to the console, NOOP can only be successful */
                CFE_EVS_SendEvent(SAMPLE_RADIO_CMD_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                                  "SAMPLE_RADIO: NOOP command received");
            }
            break;

        /*
        ** Reset Counters Command
        */
        case SAMPLE_RADIO_RESET_COUNTERS_CC:
            if (SAMPLE_RADIO_VerifyCmdLength(SAMPLE_RADIO_AppData.MsgPtr, sizeof(SAMPLE_RADIO_NoArgs_cmd_t)) == OS_SUCCESS)
            {
#ifdef SAMPLE_RADIO_CFG_DEBUG
                OS_printf("SAMPLE_RADIO: SAMPLE_RADIO_RESET_COUNTERS_CC received \n");
#endif
                SAMPLE_RADIO_ResetCounters();
            }
            break;

        /*
        ** Enable Command
        */
        case SAMPLE_RADIO_ENABLE_CC:
            if (SAMPLE_RADIO_VerifyCmdLength(SAMPLE_RADIO_AppData.MsgPtr, sizeof(SAMPLE_RADIO_NoArgs_cmd_t)) == OS_SUCCESS)
            {
#ifdef SAMPLE_RADIO_CFG_DEBUG
                OS_printf("SAMPLE_RADIO: SAMPLE_RADIO_ENABLE_CC received \n");
#endif
                SAMPLE_RADIO_Enable();
            }
            break;

        /*
        ** Disable Command
        */
        case SAMPLE_RADIO_DISABLE_CC:
            if (SAMPLE_RADIO_VerifyCmdLength(SAMPLE_RADIO_AppData.MsgPtr, sizeof(SAMPLE_RADIO_NoArgs_cmd_t)) == OS_SUCCESS)
            {
#ifdef SAMPLE_RADIO_CFG_DEBUG
                OS_printf("SAMPLE_RADIO: SAMPLE_RADIO_DISABLE_CC received \n");
#endif
                SAMPLE_RADIO_Disable();
            }
            break;

        /*
        ** Set Configuration Command
        ** Note that this is an example of a command that has additional arguments
        */
        case SAMPLE_RADIO_CONFIG_CC:
            if (SAMPLE_RADIO_VerifyCmdLength(SAMPLE_RADIO_AppData.MsgPtr, sizeof(SAMPLE_RADIO_Config_cmd_t)) == OS_SUCCESS)
            {
#ifdef SAMPLE_RADIO_CFG_DEBUG
                OS_printf("SAMPLE_RADIO: SAMPLE_RADIO_CONFIG_CC received \n");
#endif
                SAMPLE_RADIO_Configure();
            }
            break;

        /*
        ** TODO: Edit and add more command codes as appropriate for the application
        */

        /*
        ** Invalid Command Codes
        */
        default:
            /* Increment the command error counter upon receipt of an invalid command */
            SAMPLE_RADIO_AppData.HkTelemetryPkt.CommandErrorCount++;

            /* Send invalid command code failure to the console */
            CFE_EVS_SendEvent(SAMPLE_RADIO_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE_RADIO: Invalid command code for packet, MID = 0x%x, cmdCode = 0x%x",
                              CFE_SB_MsgIdToValue(MsgId), CommandCode);
            break;
    }
    return;
}

/*
** Process Telemetry Request - Triggered in response to a telemetry request
*/
void SAMPLE_RADIO_ProcessTelemetryRequest(void)
{
    CFE_SB_MsgId_t    MsgId       = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t CommandCode = 0;

    /* MsgId is only needed if the command code is not recognized. See default case */
    CFE_MSG_GetMsgId(SAMPLE_RADIO_AppData.MsgPtr, &MsgId);

    /* Pull this command code from the message and then process */
    CFE_MSG_GetFcnCode(SAMPLE_RADIO_AppData.MsgPtr, &CommandCode);
    switch (CommandCode)
    {
        case SAMPLE_RADIO_REQ_HK_TLM:
            SAMPLE_RADIO_ReportHousekeeping();
            break;

        case SAMPLE_RADIO_REQ_DATA_TLM:
            SAMPLE_RADIO_ReportDeviceTelemetry();
            break;

        /*
        ** TODO: Edit, add, or remove telemetry request codes appropriate for the application
        */

        /*
        ** Invalid Command Codes
        */
        default:
            /* Increment the error counter upon receipt of an invalid command */
            SAMPLE_RADIO_AppData.HkTelemetryPkt.CommandErrorCount++;

            /* Send invalid command code failure to the console */
            CFE_EVS_SendEvent(SAMPLE_RADIO_DEVICE_TLM_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE_RADIO: Invalid command code for packet, MID = 0x%x, cmdCode = 0x%x",
                              CFE_SB_MsgIdToValue(MsgId), CommandCode);
            break;
    }
    return;
}

/*
** Report Application Housekeeping
*/
void SAMPLE_RADIO_ReportHousekeeping(void)
{
    int32 status = OS_SUCCESS;

    /* Check that device is enabled */
    if (SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceEnabled == SAMPLE_RADIO_DEVICE_ENABLED)
    {
        status = SAMPLE_RADIO_RequestHK(&SAMPLE_RADIO_AppData.Sample_radioUart,
                                  (SAMPLE_RADIO_Device_HK_tlm_t *)&SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceHK);
        if (status == OS_SUCCESS)
        {
            SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceCount++;
        }
        else
        {
            SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceErrorCount++;
            CFE_EVS_SendEvent(SAMPLE_RADIO_REQ_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE_RADIO: Request device HK reported error %d", status);
        }
    }
    /* Intentionally do not report errors if disabled */

    /* Time stamp and publish housekeeping telemetry */
    CFE_SB_TimeStampMsg((CFE_MSG_Message_t *)&SAMPLE_RADIO_AppData.HkTelemetryPkt);
    CFE_SB_TransmitMsg((CFE_MSG_Message_t *)&SAMPLE_RADIO_AppData.HkTelemetryPkt, true);
    return;
}

/*
** Collect and Report Device Telemetry
*/
void SAMPLE_RADIO_ReportDeviceTelemetry(void)
{
    int32 status = OS_SUCCESS;

    /* Check that device is enabled */
    if (SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceEnabled == SAMPLE_RADIO_DEVICE_ENABLED)
    {
        status = SAMPLE_RADIO_RequestData(&SAMPLE_RADIO_AppData.Sample_radioUart,
                                    (SAMPLE_RADIO_Device_Data_tlm_t *)&SAMPLE_RADIO_AppData.DevicePkt.Sample_radio);
        if (status == OS_SUCCESS)
        {
            /* Update packet count */
            SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceCount++;

            /* Time stamp and publish data telemetry */
            CFE_SB_TimeStampMsg((CFE_MSG_Message_t *)&SAMPLE_RADIO_AppData.DevicePkt);
            CFE_SB_TransmitMsg((CFE_MSG_Message_t *)&SAMPLE_RADIO_AppData.DevicePkt, true);
        }
        else
        {
            SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceErrorCount++;
            CFE_EVS_SendEvent(SAMPLE_RADIO_REQ_DATA_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE_RADIO: Request device data reported error %d", status);
        }

        /* Check device status and act on error */
        if (SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceHK.DeviceStatus != 0)
        {
            /* Any bit is an error, halting communication until device power cycled */
            SAMPLE_RADIO_Disable();

            /* Send device status error to the console */
            CFE_EVS_SendEvent(SAMPLE_RADIO_REQ_DATA_STATUS_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE_RADIO: Request device data reported status error %d",
                              SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceHK.DeviceStatus);
        }
    }
    /* Intentionally do not report errors if device disabled */
    return;
}

/*
** Ingest science MGR data and save it
** TODO: This is specific to the sample_radio application, remove if using template generator
*/
void SAMPLE_RADIO_ProcessMgrHk(void)
{
    MGR_Hk_tlm_t *pMsg = (MGR_Hk_tlm_t *)SAMPLE_RADIO_AppData.MsgPtr;

    SAMPLE_RADIO_AppData.DevicePkt.PassNumber   = pMsg->SciPassCount;
    SAMPLE_RADIO_AppData.DevicePkt.RegionStatus = pMsg->ScienceStatus;
    return;
}

/*
** Reset all global counter variables
*/
void SAMPLE_RADIO_ResetCounters(void)
{
    /* Do any necessary checks, none for reset counters */

    /* Increment command success or error counter, omitted as action is to reset */

    /* Do the action, clear all global counter variables */
    SAMPLE_RADIO_AppData.HkTelemetryPkt.CommandErrorCount = 0;
    SAMPLE_RADIO_AppData.HkTelemetryPkt.CommandCount      = 0;
    SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceErrorCount  = 0;
    SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceCount       = 0;

    /* Increment device success or error counter, none as application only */

    /* Send event success to the console */
    CFE_EVS_SendEvent(SAMPLE_RADIO_CMD_RESET_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "SAMPLE_RADIO: RESET counters command received");
    return;
}

/*
** Enable Component
** TODO: Edit for your specific component implementation
*/
void SAMPLE_RADIO_Enable(void)
{
    int32 status = OS_SUCCESS;

    /* Do any necessary checks, confirm that device is currently disabled */
    if (SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceEnabled == SAMPLE_RADIO_DEVICE_DISABLED)
    {
        /* Increment command success counter */
        SAMPLE_RADIO_AppData.HkTelemetryPkt.CommandCount++;

        /*
        ** Do the action, initialize hardware interface and set enabled
        ** TODO: Make specific to your application depending on protocol in use
        ** Note that other components provide examples for the different protocols
        */
        SAMPLE_RADIO_AppData.Sample_radioUart.deviceString  = SAMPLE_RADIO_CFG_STRING;
        SAMPLE_RADIO_AppData.Sample_radioUart.handle        = SAMPLE_RADIO_CFG_HANDLE;
        SAMPLE_RADIO_AppData.Sample_radioUart.isOpen        = PORT_CLOSED;
        SAMPLE_RADIO_AppData.Sample_radioUart.baud          = SAMPLE_RADIO_CFG_BAUDRATE_HZ;
        SAMPLE_RADIO_AppData.Sample_radioUart.access_option = uart_access_flag_RDWR;

        status = uart_init_port(&SAMPLE_RADIO_AppData.Sample_radioUart);
        if (status == OS_SUCCESS)
        {
            SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceEnabled = SAMPLE_RADIO_DEVICE_ENABLED;

            /* Increment device success counter */
            SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceCount++;

            /* Send device event success to the console */
            CFE_EVS_SendEvent(SAMPLE_RADIO_ENABLE_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "SAMPLE_RADIO: Device enabled successfully");
        }
        else
        {
            /* Increment device error counter */
            SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceErrorCount++;

            /* Send device event failure to the console */
            CFE_EVS_SendEvent(SAMPLE_RADIO_UART_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE_RADIO: Device UART port initialization error %d", status);
        }
    }
    else
    {
        /* Increment command error count */
        SAMPLE_RADIO_AppData.HkTelemetryPkt.CommandErrorCount++;

        /* Send command event failure to the console */
        CFE_EVS_SendEvent(SAMPLE_RADIO_ENABLE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SAMPLE_RADIO: Device enable failed, already enabled");
    }
    return;
}

/*
** Disable Component
** TODO: Edit for your specific component implementation
*/
void SAMPLE_RADIO_Disable(void)
{
    int32 status = OS_SUCCESS;

    /* Do any necessary checks, confirm that device is currently enabled */
    if (SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceEnabled == SAMPLE_RADIO_DEVICE_ENABLED)
    {
        /* Increment command success counter */
        SAMPLE_RADIO_AppData.HkTelemetryPkt.CommandCount++;

        /*
        ** Do the action, close hardware interface and set disabled
        ** TODO: Make specific to your application depending on protocol in use
        ** Note that other components provide examples for the different protocols
        */
        status = uart_close_port(&SAMPLE_RADIO_AppData.Sample_radioUart);
        if (status == OS_SUCCESS)
        {
            SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceEnabled = SAMPLE_RADIO_DEVICE_DISABLED;

            /* Increment device success counter */
            SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceCount++;

            /* Send device event success to the console */
            CFE_EVS_SendEvent(SAMPLE_RADIO_DISABLE_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "SAMPLE_RADIO: Device disabled successfully");
        }
        else
        {
            /* Increment device error counter */
            SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceErrorCount++;

            /* Send device event failure to the console */
            CFE_EVS_SendEvent(SAMPLE_RADIO_UART_CLOSE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE_RADIO: Device UART port close error %d", status);
        }
    }
    else
    {
        /* Increment command error count */
        SAMPLE_RADIO_AppData.HkTelemetryPkt.CommandErrorCount++;

        /* Send command event failure to the console */
        CFE_EVS_SendEvent(SAMPLE_RADIO_DISABLE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SAMPLE_RADIO: Device disable failed, already disabled");
    }
    return;
}

/*
** Configure Component
** TODO: Edit for your specific component implementation
*/
void SAMPLE_RADIO_Configure(void)
{
    int32                status        = OS_SUCCESS;
    int32                device_status = OS_SUCCESS;
    SAMPLE_RADIO_Config_cmd_t *config_cmd    = (SAMPLE_RADIO_Config_cmd_t *)SAMPLE_RADIO_AppData.MsgPtr;

    /* Do any necessary checks, confirm that device is currently enabled */
    if (SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceEnabled != SAMPLE_RADIO_DEVICE_ENABLED)
    {
        status = OS_ERROR;
        /* Increment command error count */
        SAMPLE_RADIO_AppData.HkTelemetryPkt.CommandErrorCount++;

        /* Send event logging failure of check to the console */
        CFE_EVS_SendEvent(SAMPLE_RADIO_CMD_CONFIG_EN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SAMPLE_RADIO: Configuration command invalid when device disabled");
    }

    /* Do any necessary checks, confirm valid configuration value */
    if (config_cmd->DeviceCfg == 0xFFFFFFFF) // 4294967295
    {
        status = OS_ERROR;
        /* Increment command error count */
        SAMPLE_RADIO_AppData.HkTelemetryPkt.CommandErrorCount++;

        /* Send event logging failure of check to the console */
        CFE_EVS_SendEvent(SAMPLE_RADIO_CMD_CONFIG_VAL_ERR_EID, CFE_EVS_EventType_ERROR,
                          "SAMPLE_RADIO: Configuration command with value %u is invalid", config_cmd->DeviceCfg);
    }

    if (status == OS_SUCCESS)
    {
        /* Increment command success counter */
        SAMPLE_RADIO_AppData.HkTelemetryPkt.CommandCount++;

        /* Do the action, command device to with a new configuration */
        device_status = SAMPLE_RADIO_CommandDevice(&SAMPLE_RADIO_AppData.Sample_radioUart, SAMPLE_RADIO_DEVICE_CFG_CMD, config_cmd->DeviceCfg);
        if (device_status == OS_SUCCESS)
        {
            /* Increment device success counter */
            SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceCount++;

            /* Send device event success to the console */
            CFE_EVS_SendEvent(SAMPLE_RADIO_CMD_CONFIG_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "SAMPLE_RADIO: Configuration command received: %u", config_cmd->DeviceCfg);
        }
        else
        {
            /* Increment device error counter */
            SAMPLE_RADIO_AppData.HkTelemetryPkt.DeviceErrorCount++;

            /* Send device event failure to the console */
            CFE_EVS_SendEvent(SAMPLE_RADIO_CMD_CONFIG_DEV_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE_RADIO: Configuration command received: %u", config_cmd->DeviceCfg);
        }
    }
    return;
}

/*
** Verify command packet length matches expected
*/
int32 SAMPLE_RADIO_VerifyCmdLength(CFE_MSG_Message_t *msg, uint16 expected_length)
{
    int32             status        = OS_SUCCESS;
    CFE_SB_MsgId_t    msg_id        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t cmd_code      = 0;
    size_t            actual_length = 0;

    CFE_MSG_GetSize(msg, &actual_length);
    if (expected_length != actual_length)
    {
        CFE_MSG_GetMsgId(msg, &msg_id);
        CFE_MSG_GetFcnCode(msg, &cmd_code);

        CFE_EVS_SendEvent(SAMPLE_RADIO_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid msg length: ID = 0x%X,  CC = %d, Len = %ld, Expected = %d",
                          CFE_SB_MsgIdToValue(msg_id), cmd_code, actual_length, expected_length);

        status = OS_ERROR;

        /* Increment the command error counter upon receipt of an invalid command length */
        SAMPLE_RADIO_AppData.HkTelemetryPkt.CommandErrorCount++;
    }
    return status;
}
