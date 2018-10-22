#include "report.h"

struct report_t {
	uint8_t modMask;
	uint8_t reserved;
	KeyUsage keys[6];
};
static_assert(sizeof(report_t) == 8, "Invalid report size");