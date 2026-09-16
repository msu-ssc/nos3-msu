#ifndef _SAMPLE_RADIO_CHECKOUT_DEVICE_CFG_H_
#define _SAMPLE_RADIO_CHECKOUT_DEVICE_CFG_H_

/*
** SAMPLE_RADIO Checkout Configuration
*/
#define SAMPLE_RADIO_CFG
/* Note: NOS3 uart requires matching handle and bus number */
#define SAMPLE_RADIO_CFG_STRING      "/dev/usart_16"
#define SAMPLE_RADIO_CFG_HANDLE      16
#define SAMPLE_RADIO_CFG_BAUDRATE_HZ 115200
#define SAMPLE_RADIO_CFG_MS_TIMEOUT  250
#define SAMPLE_RADIO_CFG_DEBUG

#endif /* _SAMPLE_RADIO_CHECKOUT_DEVICE_CFG_H_ */
