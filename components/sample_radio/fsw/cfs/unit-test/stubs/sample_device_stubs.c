#include "utgenstub.h"
#include "sample_radio_device.h"

int32_t SAMPLE_RADIO_ReadData(uart_info_t *device, uint8_t *read_data, uint8_t data_length)
{
    UT_GenStub_SetupReturnBuffer(SAMPLE_RADIO_ReadData, int32_t);

    UT_GenStub_AddParam(SAMPLE_RADIO_ReadData, uart_info_t *, device);
    UT_GenStub_AddParam(SAMPLE_RADIO_ReadData, uint8_t *, read_data);
    UT_GenStub_AddParam(SAMPLE_RADIO_ReadData, uint8_t, data_length);

    UT_GenStub_Execute(SAMPLE_RADIO_ReadData, Basic, NULL);

    return UT_GenStub_GetReturnValue(SAMPLE_RADIO_ReadData, int32_t);
}

int32_t SAMPLE_RADIO_CommandDevice(uart_info_t *device, uint8_t cmd, uint32_t payload)
{
    UT_GenStub_SetupReturnBuffer(SAMPLE_RADIO_CommandDevice, int32_t);

    UT_GenStub_AddParam(SAMPLE_RADIO_CommandDevice, uart_info_t *, device);
    UT_GenStub_AddParam(SAMPLE_RADIO_CommandDevice, uint8_t, cmd);
    UT_GenStub_AddParam(SAMPLE_RADIO_CommandDevice, uint32_t, payload);

    UT_GenStub_Execute(SAMPLE_RADIO_CommandDevice, Basic, NULL);

    return UT_GenStub_GetReturnValue(SAMPLE_RADIO_CommandDevice, int32_t);
}

int32_t SAMPLE_RADIO_RequestHK(uart_info_t *device, SAMPLE_RADIO_Device_HK_tlm_t *data)
{
    UT_GenStub_SetupReturnBuffer(SAMPLE_RADIO_RequestHK, int32_t);

    UT_GenStub_AddParam(SAMPLE_RADIO_RequestHK, uart_info_t *, device);
    UT_GenStub_AddParam(SAMPLE_RADIO_RequestHK, SAMPLE_RADIO_Device_HK_tlm_t *, data);

    UT_GenStub_Execute(SAMPLE_RADIO_RequestHK, Basic, NULL);

    return UT_GenStub_GetReturnValue(SAMPLE_RADIO_RequestHK, int32_t);
}

int32_t SAMPLE_RADIO_RequestData(uart_info_t *device, SAMPLE_RADIO_Device_Data_tlm_t *data)
{
    UT_GenStub_SetupReturnBuffer(SAMPLE_RADIO_RequestData, int32_t);

    UT_GenStub_AddParam(SAMPLE_RADIO_RequestData, uart_info_t *, device);
    UT_GenStub_AddParam(SAMPLE_RADIO_RequestData, SAMPLE_RADIO_Device_Data_tlm_t *, data);

    UT_GenStub_Execute(SAMPLE_RADIO_RequestData, Basic, NULL);

    return UT_GenStub_GetReturnValue(SAMPLE_RADIO_RequestData, int32_t);
}
