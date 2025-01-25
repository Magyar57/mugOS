#ifndef __SERVICES_H__
#define __SERVICES_H__

#include "EFI/DataTypes.h"
#include "EFI/Memory.h"
#include "EFI/Variables.h"
#include "EFI/Time.h"
#include "EFI/Capsule.h"
#include "EFI/Protocols/Protocol.h"

// Services.h: UEFI services (BootServices and SystemServices) definitions

// ================ BootServices functions prototypes ================

typedef EFI_TPL (EFIAPI* EFI_RAISE_TPL)(IN EFI_TPL NewTpl);
typedef VOID (EFIAPI* EFI_RESTORE_TPL)(IN EFI_TPL OldTpl);
#define TPL_APPLICATION 4
#define TPL_CALLBACK 8
#define TPL_NOTIFY 16
#define TPL_HIGH_LEVEL 31

typedef EFI_STATUS (EFIAPI* EFI_ALLOCATE_PAGES)(IN EFI_ALLOCATE_TYPE Type, IN EFI_MEMORY_TYPE MemoryType, IN UINTN Pages, IN OUT EFI_PHYSICAL_ADDRESS* Memory);
typedef EFI_STATUS (EFIAPI* EFI_FREE_PAGES)(IN EFI_PHYSICAL_ADDRESS Memory, IN UINTN Pages);
typedef EFI_STATUS (EFIAPI* EFI_GET_MEMORY_MAP)
	(IN OUT UINTN* MemoryMapSize, OUT EFI_MEMORY_DESCRIPTOR* MemoryMap, OUT UINTN* MapKey, OUT UINTN* DescriptorSize, OUT UINT32* DescriptorVersion);
typedef EFI_STATUS (EFIAPI* EFI_ALLOCATE_POOL)(IN EFI_MEMORY_TYPE PoolType, IN UINTN Size, OUT VOID** Buffer);
typedef EFI_STATUS (EFIAPI* EFI_FREE_POOL)(IN VOID* Buffer);

typedef EFI_STATUS (EFIAPI* EFI_CREATE_EVENT) (IN UINT32 Type, IN EFI_TPL NotifyTpl, IN EFI_EVENT_NOTIFY NotifyFunction, OPTIONAL IN VOID* NotifyContext, OPTIONAL OUT EFI_EVENT* Event);
typedef void (*EFI_SET_TIMER)(void); // UN-IMPLEMENTED
typedef void (*EFI_WAIT_FOR_EVENT)(void); // UN-IMPLEMENTED
typedef void (*EFI_SIGNAL_EVENT)(void); // UN-IMPLEMENTED
typedef void (*EFI_CLOSE_EVENT)(void); // UN-IMPLEMENTED
typedef void (*EFI_CHECK_EVENT)(void); // UN-IMPLEMENTED

typedef void (*EFI_IMAGE_START)(void); // UN-IMPLEMENTED
typedef EFI_STATUS (EFIAPI* EFI_EXIT)(IN EFI_HANDLE ImageHandle, IN EFI_STATUS ExitStatus, IN UINTN ExitDataSize, IN CHAR16* ExitData OPTIONAL);
typedef EFI_STATUS (EFIAPI* EFI_IMAGE_UNLOAD)(IN EFI_HANDLE ImageHandle);
typedef EFI_STATUS (EFIAPI* EFI_EXIT_BOOT_SERVICES)(IN EFI_HANDLE ImageHandle, IN UINTN MapKey);

typedef EFI_STATUS (EFIAPI* EFI_GET_NEXT_MONOTONIC_COUNT)(OUT UINT64* Count);
typedef EFI_STATUS (EFIAPI* EFI_STALL)(IN UINTN Microseconds);
typedef EFI_STATUS (EFIAPI* EFI_SET_WATCHDOG_TIMER)(IN UINTN Timeout, IN UINT64 WatchdogCode, IN UINTN DataSize, IN CHAR16* WatchdogData OPTIONAL);

typedef EFI_STATUS (EFIAPI* EFI_CALCULATE_CRC32)(IN VOID* Data, IN UINTN DataSize, OUT UINT32* Crc32);

typedef VOID (EFIAPI* EFI_COPY_MEM)(IN VOID* Destination, IN VOID* Source, IN UINTN Length);
typedef VOID EFIAPI (*EFI_SET_MEM)(IN VOID* Buffer, IN UINTN Size, IN UINT8 Value);
typedef EFI_STATUS (EFIAPI* EFI_CREATE_EVENT_EX)
	(IN UINT32 Type, IN EFI_TPL NotifyTpl, IN EFI_EVENT_NOTIFY NotifyFunction OPTIONAL, IN CONST VOID* NotifyContext OPTIONAL, IN CONST EFI_GUID* EventGroup OPTIONAL, OUT EFI_EVENT* Event);

// ================ RuntimeServices functions prototypes ================

typedef EFI_STATUS (*EFI_GET_TIME)(OUT EFI_TIME* Time, OUT EFI_TIME_CAPABILITIES* Capabilities OPTIONAL);
typedef EFI_STATUS (*EFI_SET_TIME)(IN EFI_TIME* Time);
typedef EFI_STATUS (*EFI_GET_WAKEUP_TIME)(OUT BOOLEAN* Enabled, OUT BOOLEAN*Pending, OUT EFI_TIME*Time);
typedef EFI_STATUS (*EFI_SET_WAKEUP_TIME)(IN BOOLEAN Enable, IN EFI_TIME* Time OPTIONAL);

typedef EFI_STATUS (*EFI_SET_VIRTUAL_ADDRESS_MAP)(IN UINTN MemoryMapSize, IN UINTN DescriptorSize, IN UINT32 DescriptorVersion, IN EFI_MEMORY_DESCRIPTOR* VirtualMap);
typedef EFI_STATUS (*EFI_CONVERT_POINTER)(IN UINTN DebugDisposition, IN VOID** Address);

typedef EFI_STATUS (*EFI_GET_VARIABLE)(IN CHAR16* VariableName, IN EFI_GUID* VendorGuid, OUT UINT32* Attributes OPTIONAL, IN OUT UINTN* DataSize, OUT VOID* Data OPTIONAL);
typedef EFI_STATUS (*EFI_GET_NEXT_VARIABLE_NAME)(IN OUT UINTN* VariableNameSize, IN OUT CHAR16* VariableName, IN OUT EFI_GUID* VendorGuid);
typedef EFI_STATUS (*EFI_SET_VARIABLE)(IN CHAR16* VariableName, IN EFI_GUID* VendorGuid, IN UINT32 Attributes, IN UINTN DataSize, IN VOID* Data);

typedef EFI_STATUS (*EFI_GET_NEXT_HIGH_MONO_COUNT)(OUT UINT32* HighCount);
typedef VOID (EFIAPI* EFI_RESET_SYSTEM)(IN EFI_RESET_TYPE ResetType, IN EFI_STATUS ResetStatus, IN UINTN DataSize, IN VOID* ResetData OPTIONAL);

typedef EFI_STATUS (*EFI_UPDATE_CAPSULE)(IN EFI_CAPSULE_HEADER** CapsuleHeaderArray, IN UINTN CapsuleCount, IN EFI_PHYSICAL_ADDRESS ScatterGatherList OPTIONAL);
typedef EFI_STATUS (*EFI_QUERY_CAPSULE_CAPABILITIES)(IN EFI_CAPSULE_HEADER** CapsuleHeaderArray, IN UINTN CapsuleCount, OUT UINT64* MaximumCapsuleSize, OUT EFI_RESET_TYPE* ResetType);

typedef EFI_STATUS (*EFI_QUERY_VARIABLE_INFO)(IN UINT32 Attributes, OUT UINT64* MaximumVariableStorageSize, OUT UINT64* RemainingVariableStorageSize, OUT UINT64* MaximumVariableSize);

// ================ BootServices & RuntimeServices table ================

#define EFI_BOOT_SERVICES_SIGNATURE 0x56524553544f4f42
#define EFI_BOOT_SERVICES_REVISION EFI_SPECIFICATION_VERSION

typedef struct s_EFI_BOOT_SERVICES {
	EFI_TABLE_HEADER Hdr;
	// Task priority services
	EFI_RAISE_TPL RaiseTPL;
	EFI_RESTORE_TPL RestoreTPL;
	// Memory services
	EFI_ALLOCATE_PAGES AllocatePages;
	EFI_FREE_PAGES FreePages;
	EFI_GET_MEMORY_MAP GetMemoryMap;
	EFI_ALLOCATE_POOL AllocatePool;
	EFI_FREE_POOL FreePool;
	// Event & timer services
	EFI_CREATE_EVENT CreateEvent;
	EFI_SET_TIMER SetTimer;
	EFI_WAIT_FOR_EVENT WaitForEvent;
	EFI_SIGNAL_EVENT SignalEvent;
	EFI_CLOSE_EVENT CloseEvent;
	EFI_CHECK_EVENT CheckEvent;
	// Protocol handler services
	EFI_INSTALL_PROTOCOL_INTERFACE InstallProtocolInterface;
	EFI_REINSTALL_PROTOCOL_INTERFACE ReinstallProtocolInterface;
	EFI_UNINSTALL_PROTOCOL_INTERFACE UninstallProtocolInterface;
	EFI_HANDLE_PROTOCOL HandleProtocol;
 	VOID* Reserved;
	EFI_REGISTER_PROTOCOL_NOTIFY RegisterProtocolNotify;
	EFI_LOCATE_HANDLE LocateHandle;
	EFI_LOCATE_DEVICE_PATH LocateDevicePath;
	EFI_INSTALL_CONFIGURATION_TABLE InstallConfigurationTable;
	// Image services
	EFI_IMAGE_UNLOAD LoadImage;
	EFI_IMAGE_START StartImage;
	EFI_EXIT Exit;
	EFI_IMAGE_UNLOAD UnloadImage;
	EFI_EXIT_BOOT_SERVICES ExitBootServices;
	// Misc
	EFI_GET_NEXT_MONOTONIC_COUNT GetNextMonotonicCount;
	EFI_STALL Stall;
	EFI_SET_WATCHDOG_TIMER SetWatchdogTimer;
	// DriverSupport services
	EFI_CONNECT_CONTROLLER ConnectController;
	EFI_DISCONNECT_CONTROLLER DisconnectController;
	// Open and Close protocol services
	EFI_OPEN_PROTOCOL OpenProtocol;
	EFI_CLOSE_PROTOCOL CloseProtocol;
	EFI_OPEN_PROTOCOL_INFORMATION OpenProtocolInformation;
	// Library services
	EFI_PROTOCOLS_PER_HANDLE ProtocolsPerHandle;
	EFI_LOCATE_HANDLE_BUFFER LocateHandleBuffer;
	EFI_LOCATE_PROTOCOL LocateProtocol;
	EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES InstallMultipleProtocolInterfaces;
	EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES UninstallMultipleProtocolInterfaces;
	// 32-bit CRC service
	EFI_CALCULATE_CRC32 CalculateCrc32;
	// Misc
	EFI_COPY_MEM CopyMem;
	EFI_SET_MEM SetMem;
	EFI_CREATE_EVENT_EX CreateEventEx;
} EFI_BOOT_SERVICES;

#define EFI_RUNTIME_SERVICES_SIGNATURE 0x56524553544e5552
#define EFI_RUNTIME_SERVICES_REVISION EFI_SPECIFICATION_VERSION

typedef struct s_EFI_RUNTIME_SERVICES {
	EFI_TABLE_HEADER Hdr;
	// Time services
	EFI_GET_TIME GetTime;
	EFI_SET_TIME SetTime;
	EFI_GET_WAKEUP_TIME GetWakeupTime;
	EFI_SET_WAKEUP_TIME SetWakeupTime;
	// Virtual memory services
	EFI_SET_VIRTUAL_ADDRESS_MAP SetVirtualAddressMap;
	EFI_CONVERT_POINTER ConvertPointer;
	// Variable services
	EFI_GET_VARIABLE GetVariable;
	EFI_GET_NEXT_VARIABLE_NAME GetNextVariableName;
	EFI_SET_VARIABLE SetVariable;
	// Misc
	EFI_GET_NEXT_HIGH_MONO_COUNT GetNextHighMonotonicCount;
	EFI_RESET_SYSTEM ResetSystem;
	// UEFI 2.0 Capsule services
	EFI_UPDATE_CAPSULE UpdateCapsule;
	EFI_QUERY_CAPSULE_CAPABILITIES QueryCapsuleCapabilities;
	// Misc UEFI 2.0 service
	EFI_QUERY_VARIABLE_INFO QueryVariableInfo;
} EFI_RUNTIME_SERVICES;

#endif
