#include "StateMachine.h"

#include "bp_sensor.h"

bool StateMachine_begin(DeviceStatus &device)
{
    device.state = DeviceState::BOOT;

    return true;
}

void StateMachine_setState(DeviceStatus &device,DeviceState newState)
{
    device.state = newState;
}

DeviceState StateMachine_getState(const DeviceStatus &device)
{
    return device.state;
}

void StateMachine_update(DeviceStatus &device)
{
    switch(device.state)
    {
        case DeviceState::BOOT:

            device.state =
                DeviceState::INIT;

            break;

        case DeviceState::INIT:

            if(BP_begin())
            {
                device.state =
                    DeviceState::WAITING;
            }

            break;

        case DeviceState::WAITING:

            break;

        case DeviceState::MEASURING:

            BP_measure(device);

            device.state =
                DeviceState::PROCESSING;

            break;

        case DeviceState::PROCESSING:

            device.state =
                DeviceState::DISPLAY_RESULT;

            break;

        case DeviceState::DISPLAY_RESULT:

            device.state =
                DeviceState::UPLOADING;

            break;

        case DeviceState::UPLOADING:

            device.state =
                DeviceState::WAITING;

            break;

        case DeviceState::ALERT:

            break;

        case DeviceState::ERROR_STATE:

            break;
    }
}