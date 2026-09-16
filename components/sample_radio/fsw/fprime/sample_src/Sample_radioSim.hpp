// ======================================================================
// \title  Sample_radioSim.hpp
// \author jstar
// \brief  hpp file for Sample_radioSim component implementation class
// ======================================================================

#ifndef Components_Sample_radioSim_HPP
#define Components_Sample_radioSim_HPP

#include "sample_radio_src/Sample_radioSimComponentAc.hpp"
#include "sample_radio_src/Sample_radioSim_ActiveStateEnumAc.hpp"

extern "C"{
#include "sample_radio_device.h"
#include "libuart.h"
}
  

#define SAMPLE_RADIO_DEVICE_DISABLED 0
#define SAMPLE_RADIO_DEVICE_ENABLED  1

typedef struct
{
    uint8_t                     CommandErrorCount;
    uint8_t                     CommandCount;
    uint8_t                     DeviceErrorCount;
    uint8_t                     DeviceCount;
    uint8_t                     DeviceEnabled;
} __attribute__((packed)) SAMPLE_RADIO_Hk_tlm_t;
#define SAMPLE_RADIO_HK_TLM_LNGTH sizeof(SAMPLE_RADIO_Hk_tlm_t)


namespace Components {

  class Sample_radioSim :
    public Sample_radioSimComponentBase
  {

    public:

    uart_info_t Sample_radioUart; 
    SAMPLE_RADIO_Device_HK_tlm_t Sample_radioHK; 
    SAMPLE_RADIO_Device_Data_tlm_t Sample_radioData;
    int32_t status = OS_SUCCESS;

    SAMPLE_RADIO_Hk_tlm_t HkTelemetryPkt;

      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct Sample_radioSim object
      Sample_radioSim(
          const char* const compName //!< The component name
      );

      //! Destroy Sample_radioSim object
      ~Sample_radioSim();

    private:

      // ----------------------------------------------------------------------
      // Handler implementations for commands
      // ----------------------------------------------------------------------


      void REQUEST_HOUSEKEEPING_cmdHandler(
        FwOpcodeType opCode, 
        U32 cmdSeq
      ) override;

      void NOOP_cmdHandler(
        FwOpcodeType opCode, 
        U32 cmdSeq
      )override;

       void SAMPLE_RADIO_SEQ_cmdHandler(
        FwOpcodeType opCode, 
        U32 cmdSeq
      )override;

      void ENABLE_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      )override;

      void DISABLE_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      )override;

      void RESET_COUNTERS_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
      )override;

      void CONFIGURE_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq,
        const U32 config
      )override;

      inline Sample_radioSim_ActiveState get_active_state(uint8_t DeviceEnabled);

  };

}

#endif
