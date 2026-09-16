require 'cosmos'
require 'cosmos/script'
require "sample_radio_lib.rb"

##
## This script tests the cFS component device functionality.
## Currently this includes: 
##   Enable / disable, control hardware communications
##   Configuration, reconfigure sample_radio instrument register
##


##
## Enable / disable, control hardware communications
##
SAMPLE_RADIO_TEST_LOOP_COUNT.times do |n|
    # Get to known state
    safe_sample_radio()

    # Manually command to disable when already disabled
    cmd_cnt = tlm("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_ERR_COUNT")
    cmd("SAMPLE_RADIO SAMPLE_RADIO_DISABLE_CC")
    get_sample_radio_hk()
    check("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Enable
    enable_sample_radio()

    # Confirm device counters increment without errors
    confirm_sample_radio_data_loop()

    # Manually command to enable when already enabled
    cmd_cnt = tlm("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_ERR_COUNT")
    cmd("SAMPLE_RADIO SAMPLE_RADIO_ENABLE_CC")
    get_sample_radio_hk()
    check("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")

    # Reconfirm data remains as expected
    confirm_sample_radio_data_loop()

    # Disable
    disable_sample_radio()
end


##
##   Configuration, reconfigure sample_radio instrument register
##
SAMPLE_RADIO_TEST_LOOP_COUNT.times do |n|
    # Get to known state
    safe_sample_radio()

    # Confirm configuration command denied if disabled
    cmd_cnt = tlm("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_COUNT")
    cmd_err_cnt = tlm("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_ERR_COUNT")
    cmd("SAMPLE_RADIO SAMPLE_RADIO_CONFIG_CC with DEVICE_CONFIG 10")
    get_sample_radio_hk()
    check("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_COUNT == #{cmd_cnt}")
    check("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_ERR_COUNT == #{cmd_err_cnt+1}")
    
    # Enable
    enable_sample_radio()

    # Set configuration
    sample_radio_cmd("SAMPLE_RADIO SAMPLE_RADIO_CONFIG_CC with DEVICE_CONFIG #{n+1}")
    check("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM DEVICE_CONFIG == #{n+1}")
end
