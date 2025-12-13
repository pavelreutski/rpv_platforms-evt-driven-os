#pragma once

#include "events.h"

/// @brief executes kernel processing pipeline
/// @param none
void _kernel_pipeline(void);

void _kernel_publish_evt(uint8_t evtId, evt_data_t* evtData);

void _kernel_subscribe_evt(uint8_t evtId, evt_subscriber subscriber);
void _kernel_subscribe_kernel_evt(uint8_t evtId, evt_subscriber subscriber);