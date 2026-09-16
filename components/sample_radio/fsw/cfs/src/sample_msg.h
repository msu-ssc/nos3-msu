/*******************************************************************************
** File:
**   sample_radio_msg.h
**
** Purpose:
**  Define SAMPLE_RADIO application commands and telemetry messages
**
*******************************************************************************/
#ifndef _SAMPLE_RADIO_MSG_H_
#define _SAMPLE_RADIO_MSG_H_

#include "cfe.h"
#include "sample_radio_device.h"

/*
** Ground Command Codes
** TODO: Add additional commands required by the specific component
*/
#define SAMPLE_RADIO_NOOP_CC           0
#define SAMPLE_RADIO_RESET_COUNTERS_CC 1
#define SAMPLE_RADIO_ENABLE_CC         2
#define SAMPLE_RADIO_DISABLE_CC        3
#define SAMPLE_RADIO_CONFIG_CC         4

/*
** Telemetry Request Command Codes
** TODO: Add additional commands required by the specific component
*/
#define SAMPLE_RADIO_REQ_HK_TLM   0
#define SAMPLE_RADIO_REQ_DATA_TLM 1

/*
** Generic "no arguments" command type definition
*/
typedef struct
{
    /* Every command requires a header used to identify it */
    CFE_MSG_CommandHeader_t CmdHeader;

} SAMPLE_RADIO_NoArgs_cmd_t;

/*
** SAMPLE_RADIO write configuration command
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader;
    uint32                  DeviceCfg;

} SAMPLE_RADIO_Config_cmd_t;

/*
** SAMPLE_RADIO device telemetry definition
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    SAMPLE_RADIO_Device_Data_tlm_t  Sample_radio;

    /* TODO: This is specific to the sample_radio application, remove if using template generator */
    uint16 PassNumber;
    uint8  RegionStatus;

} __attribute__((packed)) SAMPLE_RADIO_Device_tlm_t;
#define SAMPLE_RADIO_DEVICE_TLM_LNGTH sizeof(SAMPLE_RADIO_Device_tlm_t)

/*
** SAMPLE_RADIO housekeeping type definition
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    uint8                     CommandErrorCount;
    uint8                     CommandCount;
    uint8                     DeviceErrorCount;
    uint8                     DeviceCount;

    /*
    ** TODO: Edit and add specific telemetry values to this struct
    */
    uint8                  DeviceEnabled;
    SAMPLE_RADIO_Device_HK_tlm_t DeviceHK;

} __attribute__((packed)) SAMPLE_RADIO_Hk_tlm_t;
#define SAMPLE_RADIO_HK_TLM_LNGTH sizeof(SAMPLE_RADIO_Hk_tlm_t)

#endif /* _SAMPLE_RADIO_MSG_H_ */
