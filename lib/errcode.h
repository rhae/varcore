
#pragma once


typedef enum {
	kErrNone      = 0,
	kErrGeneric   = -1,

	kErrBase               = 0x2000,
	kErrUpperLimit        = (kErrBase + 1),
	kErrLowerLimit        = (kErrBase + 2),

	kErrNoVector          = (kErrBase + 3),
	kErrInvalidChan       = (kErrBase + 4),

	kErrAccessDenied      = (kErrBase + 5),
	kErrInvalidValue      = (kErrBase + 6),
	kErrInvalidType       = (kErrBase + 7),

	
} ErrCode;
