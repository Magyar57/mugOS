#ifndef __DEVICE_PATH_PROTOCOL_H__
#define __DEVICE_PATH_PROTOCOL_H__

#include "EFI/DataTypes.h"

// Protocols/DevicePathProtocol.h: EFI_DEVICE_PATH_PROTOCOL (10.2), 

#define EFI_DEVICE_PATH_PROTOCOL_GUID {0x09576e91 ,0x6d3f,0x11d2,{0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}}

// 10.2
typedef struct _EFI_DEVICE_PATH_PROTOCOL {
	UINT8 Type;
	UINT8 SubType;
	UINT8 Length[2];
} EFI_DEVICE_PATH_PROTOCOL;

#endif
