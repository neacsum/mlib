#pragma once

// DPRINTF.H
//	(c) Mircea Neacsu 1999-2000. All rights reserved.
//
// Definition of dprintf. This is a printf style function that will write messages
//	using OutputDebugString.
//
//	Message length is limited to 512 characters.
//
#ifdef __cplusplus
extern "C" {
#endif

void dprintf (const char *fmt, ... );

#ifdef __cplusplus
};
#endif
