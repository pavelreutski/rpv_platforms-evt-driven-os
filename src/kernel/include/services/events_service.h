#pragma once

#include <stdint.h>

typedef enum {
	SERVICE_OK,
	SERVICE_ERR

} service_status_codes_t;

void _evt_service(void);
service_status_codes_t _service_push_events(void);