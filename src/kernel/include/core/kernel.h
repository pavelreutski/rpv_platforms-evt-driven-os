#pragma once

#include "events.h"

/// @brief executes kernel processing pipeline
/// @param none
void _kernel_pipeline(void);

void _kernel_pubEvt(uint8_t evtId, evt_data_t* evtData);

void _kernel_subEvt(uint8_t evtId, evt_subscriber subscriber);
void _kernel_subkEvt(uint8_t evtId, evt_subscriber subscriber);