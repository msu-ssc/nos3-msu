#include "cfe.h"
#include "libgpio.h"
#include "iris_fake_msgids.h"

/* The COSMOS definition uses the same header + 5-byte payload layout. */
typedef struct
{
    CFE_MSG_TelemetryHeader_t Header;
    uint32 IRIS_PIN_TRANSITION_COUNT;
    uint8 IRIS_PIN_CURRENT_STATE;
} __attribute__((packed)) IRIS_Hk_t;

static IRIS_Hk_t Hk;
static gpio_info_t Gpo1 = {.pin = 1, .direction = GPIO_INPUT};
static bool HaveInitialSample;

static void IRIS_FAKE_Poll(void)
{
    uint8 state;
    if (gpio_read(&Gpo1, &state) != GPIO_SUCCESS)
        return; /* A missing/unreadable pin is not a transition. */

    if (HaveInitialSample && state != Hk.IRIS_PIN_CURRENT_STATE)
    {
        ++Hk.IRIS_PIN_TRANSITION_COUNT;
        CFE_EVS_SendEvent(1, CFE_EVS_EventType_INFORMATION,
                         "IRIS GPO1 -> %u, transitions = %lu",
                         (unsigned int)state,
                         (unsigned long)Hk.IRIS_PIN_TRANSITION_COUNT);
    }
    Hk.IRIS_PIN_CURRENT_STATE = state;
    HaveInitialSample = true;
}

void IRIS_FAKE_AppMain(void)
{
    uint32 runStatus = CFE_ES_RunStatus_APP_RUN;
    CFE_SB_PipeId_t pipe;
    CFE_SB_Buffer_t *buffer;
    CFE_SB_MsgId_t mid;

    CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (gpio_init(&Gpo1) != GPIO_SUCCESS ||
        CFE_SB_CreatePipe(&pipe, 16, "IRIS_FAKE_PIPE") != CFE_SUCCESS ||
        CFE_SB_Subscribe(CFE_SB_ValueToMsgId(IRIS_FAKE_POLL_MID), pipe) != CFE_SUCCESS ||
        CFE_SB_Subscribe(CFE_SB_ValueToMsgId(IRIS_FAKE_SEND_HK_MID), pipe) != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("IRIS_FAKE initialization failed\n");
        CFE_ES_ExitApp(CFE_ES_RunStatus_APP_ERROR);
        return;
    }

    CFE_MSG_Init(CFE_MSG_PTR(Hk.Header), CFE_SB_ValueToMsgId(IRIS_HK_TLM_MID), sizeof(Hk));
    IRIS_FAKE_Poll(); /* First successful read establishes the baseline. */

    while (CFE_ES_RunLoop(&runStatus))
    {
        if (CFE_SB_ReceiveBuffer(&buffer, pipe, CFE_SB_PEND_FOREVER) != CFE_SUCCESS)
            break;
        CFE_MSG_GetMsgId(&buffer->Msg, &mid);
        switch (CFE_SB_MsgIdToValue(mid))
        {
            case IRIS_FAKE_POLL_MID:
                IRIS_FAKE_Poll();
                break;
            case IRIS_FAKE_SEND_HK_MID:
                CFE_SB_TimeStampMsg(CFE_MSG_PTR(Hk.Header));
                CFE_SB_TransmitMsg(CFE_MSG_PTR(Hk.Header), true);
                break;
        }
    }
    gpio_close(&Gpo1);
    CFE_ES_ExitApp(CFE_ES_RunStatus_APP_ERROR);
}
