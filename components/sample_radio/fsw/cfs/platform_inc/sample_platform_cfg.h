/************************************************************************
** File:
**   $Id: sample_radio_platform_cfg.h  $
**
** Purpose:
**  Define sample_radio Platform Configuration Parameters
**
** Notes:
**
*************************************************************************/
#ifndef _SAMPLE_RADIO_PLATFORM_CFG_H_
#define _SAMPLE_RADIO_PLATFORM_CFG_H_

/*
** Default SAMPLE_RADIO Configuration
*/
#ifndef SAMPLE_RADIO_CFG
/* Notes:
**   NOS3 uart requires matching handle and bus number
*/
#define SAMPLE_RADIO_CFG_STRING      "usart_16"
#define SAMPLE_RADIO_CFG_HANDLE      16
#define SAMPLE_RADIO_CFG_BAUDRATE_HZ 115200
#define SAMPLE_RADIO_CFG_MS_TIMEOUT  50 /* Max 255 */
/* Note: Debug flag disabled (commented out) by default */
//#define SAMPLE_RADIO_CFG_DEBUG
#endif

#endif /* _SAMPLE_RADIO_PLATFORM_CFG_H_ */
