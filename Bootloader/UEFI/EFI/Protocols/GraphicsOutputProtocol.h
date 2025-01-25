#ifndef __GRAPHICS_OUTPUT_PROTOCOL_H__
#define __GRAPHICS_OUTPUT_PROTOCOL_H__

#include "EFI/DataTypes.h"
#include "EFI/Memory.h"

// Protocols/GraphicsOutputProtocol.h: GOP definitions (12.9)

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID	{0x9042a9de,0x23dc,0x4a38,{0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a}}
#define EFI_EDID_DISCOVERED_PROTOCOL_GUID	{0x1c0c34f6,0xd380,0x41fa,{0xa0,0x49,0x8a,0xd0,0x6c,0x1a,0x66,0xaa}}
#define EFI_EDID_ACTIVE_PROTOCOL_GUID		{0xbd8c1056,0x9f36,0x44ec,{0x92,0xa8,0xa6,0x33,0x7f,0x81,0x79,0x86}}
#define EFI_EDID_OVERRIDE_PROTOCOL_GUID		{0x48ecb431,0xfb72,0x45c0,{0xa9,0x22,0xf4,0x58,0xfe,0x04,0x0b,0xd5}}

typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL EFI_GRAPHICS_OUTPUT_PROTOCOL; // foward declaration
typedef struct _EFI_GRAPHICS_OUTPUT_MODE_INFORMATION EFI_GRAPHICS_OUTPUT_MODE_INFORMATION; // foward declaration
typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE; // foward declaration

typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE) \
	(IN EFI_GRAPHICS_OUTPUT_PROTOCOL* This, IN UINT32 ModeNumber, OUT UINTN* SizeOfInfo, OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** Info);

typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE)(IN EFI_GRAPHICS_OUTPUT_PROTOCOL* This, IN UINT32 ModeNumber);

// 12.9.2.3
typedef struct _EFI_GRAPHICS_OUTPUT_BLT_PIXEL {
	UINT8 Blue;
	UINT8 Green;
	UINT8 Red;
	UINT8 Reserved;
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;

// 12.9.2.3
typedef enum _EFI_GRAPHICS_OUTPUT_BLT_OPERATION {
	EfiBltVideoFill,
	EfiBltVideoToBltBuffer,
	EfiBltBufferToVideo,
	EfiBltVideoToVideo,
	EfiGraphicsOutputBltOperationMax
} EFI_GRAPHICS_OUTPUT_BLT_OPERATION;

// Block Transfer a rectangle of pixels on the graphics screen
typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT) (
	IN EFI_GRAPHICS_OUTPUT_PROTOCOL* This,
	IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL* BltBuffer, OPTIONAL
	IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation,
	IN UINTN SourceX,
	IN UINTN SourceY,
	IN UINTN DestinationX,
	IN UINTN DestinationY,
	IN UINTN Width,
	IN UINTN Height,
	IN UINTN Delta OPTIONAL
);

// ======== GOP structure ========

typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL {
	EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE QueryMode;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE SetMode;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT Blt;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

// 12.9.2
// If a bit is set in RedMask, GreenMask, or BlueMask then those bits of the pixel represent the corresponding color.
// Bits in RedMask, GreenMask, BlueMask, and ReserverdMask must not over lap bit positions.
// The values for the red, green, and blue components in the bit mask represent the color intensity.
// The color intensities must increase as the color values for a each color mask increase with a minimum
// intensity of all bits in a color mask clear to a maximum intensity of all bits in a color mask set.
typedef struct {
	UINT32 RedMask;
	UINT32 GreenMask;
	UINT32 BlueMask;
	UINT32 ReservedMask;
} EFI_PIXEL_BITMASK;

// 12.9.2
typedef enum _EFI_GRAPHICS_PIXEL_FORMAT {
	PixelRedGreenBlueReserved8BitPerColor,
	PixelBlueGreenRedReserved8BitPerColor,
	PixelBitMask,
	PixelBltOnly,
	PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

// 12.9.2
typedef struct _EFI_GRAPHICS_OUTPUT_MODE_INFORMATION {
	UINT32 Version;
	UINT32 HorizontalResolution;
	UINT32 VerticalResolution;
	EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;
	EFI_PIXEL_BITMASK PixelInformation;
	UINT32 PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

// 12.9.2
typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE {
	UINT32 MaxMode;
	UINT32 Mode;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info;
	UINTN  SizeOfInfo;
	EFI_PHYSICAL_ADDRESS FrameBufferBase;
	UINTN FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

// ================ EDID Discovery protocol (12.9.2.4) ================

typedef struct _EFI_EDID_DISCOVERED_PROTOCOL {
	UINT32 SizeOfEdid;
	UINT8* Edid; // Pointer to a readonly EDID structure
} EFI_EDID_DISCOVERED_PROTOCOL;

// ================ EDID Active protocol (12.9.2.5) ================

typedef struct _EFI_EDID_ACTIVE_PROTOCOL {
	UINT32 SizeOfEdid;
	UINT8* Edid;
} EFI_EDID_ACTIVE_PROTOCOL;

// ================ EDID Override protocol (12.9.2.6) ================

typedef struct _EFI_EDID_OVERRIDE_PROTOCOL EFI_EDID_OVERRIDE_PROTOCOL; // forward declaration

typedef EFI_STATUS (EFIAPI *EFI_EDID_OVERRIDE_PROTOCOL_GET_EDID) \
	(IN EFI_EDID_OVERRIDE_PROTOCOL* This, IN EFI_HANDLE* ChildHandle, OUT UINT32* Attributes, OUT UINTN* EdidSize, OUT UINT8** Edid);

typedef struct _EFI_EDID_OVERRIDE_PROTOCOL {
	EFI_EDID_OVERRIDE_PROTOCOL_GET_EDID GetEdid;
} EFI_EDID_OVERRIDE_PROTOCOL;

#define EFI_EDID_OVERRIDE_DONT_OVERRIDE		0x01
#define EFI_EDID_OVERRIDE_ENABLE_HOT_PLUG	0x02

#endif
