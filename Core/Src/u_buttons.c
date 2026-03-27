#include "u_buttons.h"
#include "u_statemachine.h"
#include "u_pedals.h"
#include "u_tx_debug.h"

/* Processes incoming button presses. */
void buttons_process(button_t button) {
    switch(button) {
        case BUTTON_ESC:
            PRINTLN_INFO("Button BUTTON_ESC pressed.");
            if (fabs(dti_get_mph()) <= 0.1) {
			    set_home_mode();
		    }
            break;
        case BUTTON_LEFT:
            PRINTLN_INFO("Button BUTTON_LEFT pressed.");
		    decrement_nero_index();
            break;
        case BUTTON_LAUNCH_CONTROL_OFF:
            PRINTLN_INFO("Button BUTTON_LAUNCH_CONTROL_OFF pressed.");
            pedals_disableLaunchControl();
            break;
        case BUTTON_UP_REGEN:
            PRINTLN_INFO("Button BUTTON_UP_REGEN pressed.");
            if (get_func_state() == F_EFFICIENCY || (fabs(dti_get_mph()) <= 0.1 && pedals_getBrakeState() && get_func_state() == F_PERFORMANCE)) {
			    pedals_increaseRegenLimit();
		    }
            break;
        case BUTTON_DOWN_REGEN:
            PRINTLN_INFO("Button BUTTON_DOWN_REGEN pressed.");
            if (get_func_state() == F_EFFICIENCY || (fabs(dti_get_mph()) <= 0.1 && pedals_getBrakeState() && get_func_state() == F_PERFORMANCE)) {
			    pedals_decreaseRegenLimit();
		    }
            break;
        case BUTTON_ENTER:
            PRINTLN_INFO("Button BUTTON_ENTER pressed.");
            select_nero_index();
            break;
        case BUTTON_RIGHT:
            PRINTLN_INFO("Button BUTTON_RIGHT pressed.");
		    increment_nero_index();
            break;
        case BUTTON_LAUNCH_CONTROL_ON:
            PRINTLN_INFO("Button BUTTON_LAUNCH_CONTROL_ON pressed.");
            pedals_enableLaunchControl();
            break;
        case BUTTON_UP_TORQUE:
            PRINTLN_INFO("Button BUTTON_UP_TORQUE pressed.");
            if (get_func_state() == F_EFFICIENCY) {
			    pedals_increaseTorqueLimit();
		    }
            break;
        case BUTTON_DOWN_TORQUE:
            PRINTLN_INFO("Button BUTTON_DOWN_TORQUE pressed.");
            if (get_func_state() == F_EFFICIENCY) {
			    pedals_decreaseTorqueLimit();
		    }
            break;
        default:
            break;
    }
}