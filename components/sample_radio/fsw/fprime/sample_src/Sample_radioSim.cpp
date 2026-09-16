// ======================================================================
// \title  Sample_radioSim.cpp
// \author jstar
// \brief  cpp file for Sample_radioSim component implementation class
// ======================================================================

#include "sample_radio_src/Sample_radioSim.hpp"
#include <Fw/Logger/Logger.hpp>
#include <Fw/Log/LogString.hpp>
// #include "FpConfig.hpp"
#include "Fw/FPrimeBasicTypes.hpp"

namespace Components {

  // ----------------------------------------------------------------------
  // Component construction and destruction
  // ----------------------------------------------------------------------

  Sample_radioSim ::
    Sample_radioSim(const char *const compName) : Sample_radioSimComponentBase(compName)
{
    Sample_radioUart.deviceString = SAMPLE_RADIO_CFG_STRING;
    Sample_radioUart.handle = SAMPLE_RADIO_CFG_HANDLE;
    Sample_radioUart.isOpen = PORT_CLOSED;
    Sample_radioUart.baud = SAMPLE_RADIO_CFG_BAUDRATE_HZ;
    status = uart_init_port(&Sample_radioUart);
    status = uart_close_port(&Sample_radioUart);

    HkTelemetryPkt.DeviceEnabled = SAMPLE_RADIO_DEVICE_DISABLED;
    HkTelemetryPkt.CommandCount = 0;
    HkTelemetryPkt.CommandErrorCount = 0;
    HkTelemetryPkt.DeviceCount = 0;
    HkTelemetryPkt.DeviceErrorCount = 0;
}
  
  Sample_radioSim ::
    ~Sample_radioSim()
  {
      status = uart_close_port(&Sample_radioUart);
  }

  // ----------------------------------------------------------------------
  // Handler implementations for commands
  // ----------------------------------------------------------------------

  void Sample_radioSim :: NOOP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

    status = SAMPLE_RADIO_CommandDevice(&Sample_radioUart, SAMPLE_RADIO_DEVICE_NOOP_CMD, 0);
    Fw::LogStringArg log_msg("NOOP SENT");
    this->log_ACTIVITY_HI_TELEM(log_msg);
    // OS_printf("NOOP SENT\n");

    this->tlmWrite_CommandCount(++HkTelemetryPkt.CommandCount);

    this->tlmWrite_ReportedComponentCount(Sample_radioHK.DeviceCounter);
    this->tlmWrite_DeviceConfig(Sample_radioHK.DeviceConfig);
    this->tlmWrite_DeviceStatus(Sample_radioHK.DeviceStatus);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Sample_radioSim :: REQUEST_HOUSEKEEPING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    
    if(HkTelemetryPkt.DeviceEnabled == SAMPLE_RADIO_DEVICE_ENABLED)
    {
      HkTelemetryPkt.CommandCount++;
      status = SAMPLE_RADIO_RequestHK(&Sample_radioUart, &Sample_radioHK);
      if (status == OS_SUCCESS)
      {
          HkTelemetryPkt.DeviceCount++;
          Fw::LogStringArg log_msg("RequestHK command success\n");
          this->log_ACTIVITY_HI_TELEM(log_msg);
          // OS_printf("Request Housekeeping Successful\n");
      }
      else
      {
          HkTelemetryPkt.DeviceErrorCount++;
          Fw::LogStringArg log_msg("RequestHK command failed!\n");
          this->log_ACTIVITY_HI_TELEM(log_msg);;
          // OS_printf("Request Housekeeping Failed\n");
      }

    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      Fw::LogStringArg log_msg("RequestHK failed: Device Disabled\n");
      this->log_ACTIVITY_HI_TELEM(log_msg);
      // OS_printf("Request Housekeeping failed, Device Disabled\n");
    }
    
    this->tlmWrite_ReportedComponentCount(Sample_radioHK.DeviceCounter);
    this->tlmWrite_DeviceConfig(Sample_radioHK.DeviceConfig);
    this->tlmWrite_DeviceStatus(Sample_radioHK.DeviceStatus);
    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

    // Tell the fprime command system that we have completed the processing of the supplied command with OK status
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Sample_radioSim :: SAMPLE_RADIO_SEQ_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    
  // seq_toggle = 1;
  
    for(int i=0;i<20;i++){
    // while(1){
      sleep(1);
      // printf("seq toggle is equal to %d \n", seq_toggle);
      // if(seq_toggle==0){
      //   break;
      // }

      if(HkTelemetryPkt.DeviceEnabled == SAMPLE_RADIO_DEVICE_ENABLED)
      {
        HkTelemetryPkt.CommandCount++;
        status = SAMPLE_RADIO_RequestHK(&Sample_radioUart, &Sample_radioHK);
        if (status == OS_SUCCESS)
        {
            HkTelemetryPkt.DeviceCount++;
            Fw::LogStringArg log_msg("RequestHK command success\n");
            this->log_ACTIVITY_HI_TELEM(log_msg);
        }
        else
        {
            HkTelemetryPkt.DeviceErrorCount++;
            Fw::LogStringArg log_msg("RequestHK command failed!\n");
            this->log_ACTIVITY_HI_TELEM(log_msg);
        }

      }
      else
      {
        HkTelemetryPkt.CommandErrorCount++;
        Fw::LogStringArg log_msg("RequestHK failed: Device Disabled\n");
        this->log_ACTIVITY_HI_TELEM(log_msg);
      }
      
      this->tlmWrite_ReportedComponentCount(Sample_radioHK.DeviceCounter);
      this->tlmWrite_DeviceConfig(Sample_radioHK.DeviceConfig);
      this->tlmWrite_DeviceStatus(Sample_radioHK.DeviceStatus);
      this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
      this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
      this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
      this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
      this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));

    }

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Sample_radioSim :: ENABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

    if(HkTelemetryPkt.DeviceEnabled == SAMPLE_RADIO_DEVICE_DISABLED)
    {

      HkTelemetryPkt.CommandCount++;
      
      Sample_radioUart.deviceString  = SAMPLE_RADIO_CFG_STRING;
      Sample_radioUart.handle        = SAMPLE_RADIO_CFG_HANDLE;
      Sample_radioUart.isOpen        = PORT_CLOSED;
      Sample_radioUart.baud          = SAMPLE_RADIO_CFG_BAUDRATE_HZ;
      Sample_radioUart.access_option = uart_access_flag_RDWR;

      status = uart_init_port(&Sample_radioUart);
      if(status == OS_SUCCESS)
      {

        HkTelemetryPkt.DeviceEnabled = SAMPLE_RADIO_DEVICE_ENABLED;
        HkTelemetryPkt.DeviceCount++;
        
        Fw::LogStringArg log_msg("Successfully Enabled");
        this->log_ACTIVITY_HI_TELEM(log_msg); 
        // OS_printf("Sample_radioSim Enable Succeeded\n");  
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        Fw::LogStringArg log_msg("Enable failed, failed to init UART port");
        this->log_ACTIVITY_HI_TELEM(log_msg);   
        // OS_printf("Sample_radioSim Enable Failed to init UART port\n");  
      }
    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      Fw::LogStringArg log_msg("Failed, Already Enabled");
      this->log_ACTIVITY_HI_TELEM(log_msg); 
      // OS_printf("Sample_radioSim Enable Failed, Already Enabled\n");
    }

    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));
    this->tlmWrite_ReportedComponentCount(Sample_radioHK.DeviceCounter);
    this->tlmWrite_DeviceConfig(Sample_radioHK.DeviceConfig);
    this->tlmWrite_DeviceStatus(Sample_radioHK.DeviceStatus);

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Sample_radioSim :: DISABLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {

    if(HkTelemetryPkt.DeviceEnabled == SAMPLE_RADIO_DEVICE_ENABLED)
    {

      HkTelemetryPkt.CommandCount++;

      status = uart_close_port(&Sample_radioUart);
      if (status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceEnabled = SAMPLE_RADIO_DEVICE_DISABLED;
        HkTelemetryPkt.DeviceCount++;

        Fw::LogStringArg log_msg("Disabled Successfully");
        this->log_ACTIVITY_HI_TELEM(log_msg);  
        // OS_printf("Sample_radioSim Disable Succeeded\n");
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        Fw::LogStringArg log_msg("Disable Failed to close UART port");
        this->log_ACTIVITY_HI_TELEM(log_msg);   
        // OS_printf("Sample_radioSim Disable Failed to close UART port\n");
      }
    }
    else
    {
      HkTelemetryPkt.CommandErrorCount++;
      Fw::LogStringArg log_msg("Failed, Already Disabled");
      this->log_ACTIVITY_HI_TELEM(log_msg); 
      // OS_printf("Sample_radioSim Disable Failed, device already disabled\n");
    }

    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_DeviceEnabled(get_active_state(HkTelemetryPkt.DeviceEnabled));
    this->tlmWrite_ReportedComponentCount(Sample_radioHK.DeviceCounter);
    this->tlmWrite_DeviceConfig(Sample_radioHK.DeviceConfig);
    this->tlmWrite_DeviceStatus(Sample_radioHK.DeviceStatus);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void Sample_radioSim :: RESET_COUNTERS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    HkTelemetryPkt.CommandCount = 0;
    HkTelemetryPkt.CommandErrorCount = 0;
    HkTelemetryPkt.DeviceCount = 0;
    HkTelemetryPkt.DeviceErrorCount = 0;

    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);

    Fw::LogStringArg log_msg("Counters have been Reset");
    this->log_ACTIVITY_HI_TELEM(log_msg);
    // OS_printf("Counters have been Reset\n");

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);

  }

  void Sample_radioSim :: CONFIGURE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, const U32 config){

    status = OS_SUCCESS;

    if(HkTelemetryPkt.DeviceEnabled != SAMPLE_RADIO_DEVICE_ENABLED)
    {
      status = OS_ERROR;

      HkTelemetryPkt.CommandErrorCount++;

      Fw::LogStringArg log_msg("Configure Failed, Device Disabled");
      this->log_ACTIVITY_HI_TELEM(log_msg);
      // OS_printf("Configure Failed, Device Disabled\n");
    }

    if(config == 0xFFFFFFFF) // 4294967295
    {
      status = OS_ERROR;

      HkTelemetryPkt.CommandErrorCount++;

      Fw::LogStringArg log_msg("Configure Failed, Invalid Configuration");
      this->log_ACTIVITY_HI_TELEM(log_msg);
      // OS_printf("Configure Failed, Invalid Configuration Given\n");
    }

    if(status == OS_SUCCESS)
    {
      HkTelemetryPkt.CommandCount++;

      status = SAMPLE_RADIO_CommandDevice(&Sample_radioUart, SAMPLE_RADIO_DEVICE_CFG_CMD, config);
      if(status == OS_SUCCESS)
      {
        HkTelemetryPkt.DeviceCount++;
        Fw::LogStringArg log_msg("Successfully Configured Device");
        this->log_ACTIVITY_HI_TELEM(log_msg);
        // OS_printf("Device Successfully Configured\n");
      }
      else
      {
        HkTelemetryPkt.DeviceErrorCount++;
        Fw::LogStringArg log_msg("Failed to Configure Device");
        this->log_ACTIVITY_HI_TELEM(log_msg);
        // OS_printf("Device Configuration Failed\n");
      }
    }

    this->tlmWrite_DeviceCount(HkTelemetryPkt.DeviceCount);
    this->tlmWrite_DeviceErrorCount(HkTelemetryPkt.DeviceErrorCount);
    this->tlmWrite_CommandCount(HkTelemetryPkt.CommandCount);
    this->tlmWrite_CommandErrorCount(HkTelemetryPkt.CommandErrorCount);
    this->tlmWrite_ReportedComponentCount(Sample_radioHK.DeviceCounter);
    this->tlmWrite_DeviceConfig(Sample_radioHK.DeviceConfig);
    this->tlmWrite_DeviceStatus(Sample_radioHK.DeviceStatus);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);

  }

  inline Sample_radioSim_ActiveState Sample_radioSim :: get_active_state(uint8_t DeviceEnabled)
  {
    Sample_radioSim_ActiveState state;

    if(DeviceEnabled == SAMPLE_RADIO_DEVICE_ENABLED)
    {
      state.e = Sample_radioSim_ActiveState::ENABLED;
    }
    else
    {
      state.e = Sample_radioSim_ActiveState::DISABLED;
    }

    return state;
  }

  //  void Sample_radioSim :: SAMPLE_RADIO_SEQ_CANCEL_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    
  //   seq_toggle = 0;
  //   printf("seq toggle is equal to %d\n", seq_toggle);

  //   this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  // }

}