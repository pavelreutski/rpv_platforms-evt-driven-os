#pragma once

#include <stdint.h>

#include "events.h"

void _kernel_publish_evt(uint8_t evtId, evt_data_t* evtData);

void _kernel_subscribe_evt(uint8_t evtId, evt_subscriber subscriber);
void _kernel_subscribe_kernel_evt(uint8_t evtId, evt_subscriber subscriber);