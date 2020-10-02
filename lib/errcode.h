
#pragma once


typedef enum {
	ERR_NONE      = 0,
	ERR_GENERIC   = -1,

	ERR_BASE               = 0x2000,
	ERR_UPPER_LIMIT        = (ERR_BASE + 1),
	ERR_LOWER_LIMIT        = (ERR_BASE + 2),

	ERR_NOT_VECTOR         = (ERR_BASE + 3),
	ERR_INVALID_CHAN       = (ERR_BASE + 4),

	ERR_ACCESS_DENIED      = (ERR_BASE + 5),
	ERR_INVALID_VALUE      = (ERR_BASE + 6),
	ERR_INVALID_TYPE       = (ERR_BASE + 7),
} ERR_CODE;
