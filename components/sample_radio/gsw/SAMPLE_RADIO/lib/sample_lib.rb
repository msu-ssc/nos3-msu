# Library for SAMPLE_RADIO Target
require 'cosmos'
require 'cosmos/script'

#
# Definitions
#
SAMPLE_RADIO_CMD_SLEEP = 0.25
SAMPLE_RADIO_RESPONSE_TIMEOUT = 5
SAMPLE_RADIO_TEST_LOOP_COUNT = 1
SAMPLE_RADIO_DEVICE_LOOP_COUNT = 5

#
# Functions
#
def get_sample_radio_hk()
    cmd("SAMPLE_RADIO SAMPLE_RADIO_REQ_HK")
    wait_check_packet("SAMPLE_RADIO", "SAMPLE_RADIO_HK_TLM", 1, SAMPLE_RADIO_RESPONSE_TIMEOUT)
    sleep(SAMPLE_RADIO_CMD_SLEEP)
end

def get_sample_radio_data()
    cmd("SAMPLE_RADIO SAMPLE_RADIO_REQ_DATA")
    wait_check_packet("SAMPLE_RADIO", "SAMPLE_RADIO_DATA_TLM", 1, SAMPLE_RADIO_RESPONSE_TIMEOUT)
    sleep(SAMPLE_RADIO_CMD_SLEEP)
end

def sample_radio_cmd(*command)
    count = tlm("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_COUNT") + 1

    if (count == 256)
        count = 0
    end

    cmd(*command)
    get_sample_radio_hk()
    current = tlm("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_COUNT")
    if (current != count)
        # Try again
        cmd(*command)
        get_sample_radio_hk()
        current = tlm("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_COUNT")
        if (current != count)
            # Third times the charm
            cmd(*command)
            get_sample_radio_hk()
            current = tlm("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_COUNT")
        end
    end
    check("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM CMD_COUNT >= #{count}")
end

def enable_sample_radio()
    # Send command
    sample_radio_cmd("SAMPLE_RADIO SAMPLE_RADIO_ENABLE_CC")
    # Confirm
    check("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM DEVICE_ENABLED == 'ENABLED'")
end

def disable_sample_radio()
    # Send command
    sample_radio_cmd("SAMPLE_RADIO SAMPLE_RADIO_DISABLE_CC")
    # Confirm
    check("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM DEVICE_ENABLED == 'DISABLED'")
end

def safe_sample_radio()
    get_sample_radio_hk()
    state = tlm("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM DEVICE_ENABLED")
    if (state != "DISABLED")
        disable_sample_radio()
    end
end

def confirm_sample_radio_data()
    dev_cmd_cnt = tlm("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM DEVICE_ERR_COUNT")
    
    get_sample_radio_data()
    # Note these checks assume default simulator configuration
    raw_x = tlm("SAMPLE_RADIO SAMPLE_RADIO_DATA_TLM RAW_SAMPLE_RADIO_X")
    check("SAMPLE_RADIO SAMPLE_RADIO_DATA_TLM RAW_SAMPLE_RADIO_Y >= #{raw_x*2}")
    check("SAMPLE_RADIO SAMPLE_RADIO_DATA_TLM RAW_SAMPLE_RADIO_Z >= #{raw_x*3}")

    get_sample_radio_hk()
    check("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM DEVICE_COUNT >= #{dev_cmd_cnt}")
    check("SAMPLE_RADIO SAMPLE_RADIO_HK_TLM DEVICE_ERR_COUNT == #{dev_cmd_err_cnt}")
end

def confirm_sample_radio_data_loop()
    SAMPLE_RADIO_DEVICE_LOOP_COUNT.times do |n|
        confirm_sample_radio_data()
    end
end

#
# Simulator Functions
#
def sample_radio_prepare_ast()
    # Get to known state
    safe_sample_radio()

    # Enable
    enable_sample_radio()

    # Confirm data
    confirm_sample_radio_data_loop()
end

def sample_radio_sim_enable()
    cmd("SIM_CMDBUS_BRIDGE SAMPLE_RADIO_SIM_ENABLE")
end

def sample_radio_sim_disable()
    cmd("SIM_CMDBUS_BRIDGE SAMPLE_RADIO_SIM_DISABLE")
end

def sample_radio_sim_set_status(status)
    cmd("SIM_CMDBUS_BRIDGE SAMPLE_RADIO_SIM_SET_STATUS with STATUS #{status}")
end
