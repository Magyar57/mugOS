#include <stdbool.h>
#include "EFI/EFI.h"
#include "EFI/Protocols/LoadedImageProtocol.h"
#include "EFI/Protocols/FileProtocol.h"
#include "EFI/Protocols/GraphicsOutputProtocol.h"
#include "stdlib.h"

#define PAGE_SIZE		4096 // 4KiB

#define SYSV_ABI		__attribute__((sysv_abi))
#define KERNEL_FILE		L"kernel.bin"
typedef void (SYSV_ABI *kmain_t)(EFI_GRAPHICS_OUTPUT_PROTOCOL*);

EFI_HANDLE g_imageHandle = NULL;
EFI_SYSTEM_TABLE* g_st = NULL;
EFI_BOOT_SERVICES* g_bs = NULL;
EFI_RUNTIME_SERVICES* g_rs = NULL;

#define DEBUG true

#if DEBUG
	#define exit(err_code) while(true);
#else
	#define exit(err_code) g_bs->Exit(g_imageHandle, err_code, 0, NULL);
#endif

static inline void waitKey(){
	// Empty console input buffer
	EFI_STATUS res = g_st->ConIn->Reset(g_st->ConIn, FALSE);
	// if (EFI_ERROR(res))
	// 	return res;

	g_st->ConOut->OutputString(g_st->ConOut, L"Press any key to continue...\r\n");

	// Wait for key before continuing
	EFI_INPUT_KEY key;
	do {
		res = g_st->ConIn->ReadKeyStroke(g_st->ConIn, &key);
	}
	while (res == EFI_NOT_READY);
}

static void checkSystemTableSignature(){
	if (g_st->Hdr.Signature != EFI_SYSTEM_TABLE_SIGNATURE){
		puts(L"Error: EFI System Table signature is wrong. Exiting");
		exit(-1);
	}
	puts(L"Successfully checked EFI System Table signature");
}

static VOID* loadKernel(){
	EFI_LOADED_IMAGE_PROTOCOL* lip; // lip: loaded image protocol
	EFI_GUID lipGUID = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	EFI_STATUS res;

	// Open the Loaded Image protocol
	res = g_bs->OpenProtocol(g_imageHandle, &lipGUID, (VOID**) &lip, g_imageHandle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	if (EFI_ERROR(res)){
		puts(L"Error: could not open the Loaded Image protocol");
		exit(-1);
	}

	// Open Simple File System protocol (for device handle, for this loaded image, to open the root directory)
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* sfsp; // sfsp: simple file system protocol
	EFI_GUID sfspGUID = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	res = g_bs->OpenProtocol(lip->DeviceHandle, &sfspGUID, (VOID**) &sfsp, g_imageHandle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	if (EFI_ERROR(res)){
		puts(L"Error: could not open the Simple File System protocol");
		exit(-1);
	}

	// Now we can open the EFI System partition's root directory
	EFI_FILE_PROTOCOL* rootDir;
	res = sfsp->OpenVolume(sfsp, &rootDir);
	if (EFI_ERROR(res)){
		puts(L"Error: could not open the root directory");
		exit(-1);
	}

	// Search for the kernel file in root directory
	bool foundKernel = false;
	EFI_FILE_INFO fileInfo;
	UINTN bufferSize = sizeof(EFI_FILE_INFO);
	rootDir->SetPosition(rootDir, 0); // Set to first entry
	rootDir->Read(rootDir, &bufferSize, (VOID*) &fileInfo);
	while (bufferSize > 0 && !foundKernel){
		// Check for kernel
		if (!(fileInfo.Attribute & EFI_FILE_DIRECTORY)){
			if (memcmp(fileInfo.FileName, KERNEL_FILE, sizeof(KERNEL_FILE)) == 0){
				foundKernel = true;
				break;
			}
		}

		// Read next
		bufferSize = sizeof(EFI_FILE_INFO);
		res = rootDir->Read(rootDir, &bufferSize, (VOID*) &fileInfo);
	}

	if (!foundKernel){
		puts_noCRLF(L"Error, kernel file '");
		puts_noCRLF(KERNEL_FILE);
		puts(L"' was not found in root directory");
		exit(-1);
	}

	// Now read the file into a buffer
	EFI_FILE_PROTOCOL* kernel;
	res = rootDir->Open(rootDir, &kernel, KERNEL_FILE, EFI_FILE_MODE_READ, 0);

	puts_noCRLF(L"Found kernel '");
	puts_noCRLF(KERNEL_FILE);
	puts_noCRLF(L"'(size=");
	putNumberUnsigned_noCRLF(fileInfo.FileSize, 10);
	puts(L" Bytes)");

	uint8_t* buffer = NULL;
	bufferSize = 0;
	res = g_bs->AllocatePool(EfiLoaderData, fileInfo.FileSize, (VOID**) &buffer);
	// res = g_bs->AllocatePages(EfiLoaderData, fileInfo.FileSize, (VOID**) &buffer);
	if (EFI_ERROR(res)){
		puts_noCRLF(L"Error: AllocatePool failed with error ");
		putNumberUnsigned(res, 16);
		exit(-1);
	}

	bufferSize = fileInfo.FileSize;
	res = kernel->Read(kernel, &bufferSize, buffer);
	if (EFI_ERROR(res)){
		puts_noCRLF(L"Error: Read failed with error ");
		putNumberUnsigned(res, 16);
		exit(-1);
	}
	if (bufferSize != fileInfo.FileSize){
		puts(L"Error: Read couldn't copy the whole file into the buffer");
		exit(-1);
	}

	puts_noCRLF(L"Loaded kernel into memory (");
	puts_noCRLF(L"");
	putNumberUnsigned_noCRLF(bufferSize, 10);
	puts_noCRLF(L" B / ");
	putNumberUnsigned_noCRLF(fileInfo.FileSize, 10);
	puts_noCRLF(L" B) at address ");
	putNumberUnsigned_noCRLF((EFI_PHYSICAL_ADDRESS) buffer, 16);
	puts(L"");

	// Exit
	// g_bs->FreePool(buffer);
	kernel->Close(kernel);
	rootDir->Close(rootDir);
	g_bs->CloseProtocol(lip->DeviceHandle, &sfspGUID, g_imageHandle, NULL);
	g_bs->CloseProtocol(g_imageHandle, &lipGUID, g_imageHandle, NULL);

	return buffer;
}

static void loadMemoryMap(UINTN* memoryMapSize, EFI_MEMORY_DESCRIPTOR** memoryMap, UINTN* mapKey, UINTN* descriptorSize, UINT32* descriptorVersion){
	EFI_STATUS res;
	bool memoryMapValid = false;

	*memoryMap = NULL;
	*memoryMapSize = 0; // First call to GetMemoryMap with a memoryMapSize of 0 so that we know what size we need to allocate

	for(int i=0 ; i<5 ; i++){
		res = g_bs->GetMemoryMap(memoryMapSize, *memoryMap, mapKey, descriptorSize, descriptorVersion);
		if (EFI_ERROR(res) && (res!=EFI_BUFFER_TOO_SMALL)){
			puts_noCRLF(L"Error: GetMemoryMap failed with error ");
			putNumberUnsigned(res, 16);
			exit(res);
		}

		// We did it !
		if (res == EFI_SUCCESS){
			memoryMapValid = true;
			break;
		}

		// Otherwise, we need to allocate [more] memory

		// Previous buffer already allocated
		// It can happen, for some reason, that GetMemoryMap asks for x bytes at first call,
		// then asks for more at the next call
		if (*memoryMap != NULL){
			g_bs->FreePool(*memoryMap);
			*memoryMap = NULL;
		}

		res = g_bs->AllocatePool(EfiLoaderData, *memoryMapSize, (VOID**) memoryMap);
		if (EFI_ERROR(res)){
			puts_noCRLF(L"Error: AllocatePool failed with error ");
			putNumberUnsigned(res, 16);
			exit(res);
		}
	}

	if (!memoryMapValid){
		puts(L"Couldn't read memory map in iteration limit (GetMemoryMap kept asking for more memory)");
		exit(-3);
	}
}

static EFI_GRAPHICS_OUTPUT_PROTOCOL* getGOP(){
	EFI_STATUS status;

	// Note:
	// We use g_bs->LocateProtocol to get any screen's GOP.
	// If we want to handle ALL screens, we should use g_bs->LocateHandleBuffer instead

	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	status = g_bs->LocateProtocol(&gopGuid, NULL, (VOID**) &gop);
	if (status == EFI_NOT_FOUND){
		puts(L"could not locate the GOP protocol");
		exit(-4);
	}

	puts(L"Located GOP protocol, here are available video modes:");

	// TODO check that the current video mode PixelFormat is NOT PixelBltOnly (it means that linear frame buffer not supported)
	for(UINTN i=0 ; i<gop->Mode->MaxMode ; i++){
		putNumberUnsigned_noCRLF(gop->Mode->Info->HorizontalResolution, 10);
		puts_noCRLF(L"x");
		putNumberUnsigned_noCRLF(gop->Mode->Info->VerticalResolution, 10);

		puts_noCRLF(L"-");
		putNumberUnsigned_noCRLF(gop->Mode->Info->PixelFormat, 10);
		puts_noCRLF(L" ");
	}
	puts(L"");

	// status = gop->SetMode(gop, gop->Mode->MaxMode-1);
	// status = gop->SetMode(gop, 0);
	// if (status == EFI_DEVICE_ERROR){
	// 	puts(L"Error: Screen device error when changing video mode");
	// }
	// if (status == EFI_UNSUPPORTED){
	// 	puts(L"Error: Tried to change to an unsupported video mode");
	// }
	// g_st->ConOut->ClearScreen(g_st->ConOut); // reset console out so that it outputs at top of (now cleared) screen

	return gop;
}

EFI_STATUS bmain(IN EFI_HANDLE imageHandle, IN EFI_SYSTEM_TABLE* st){
	EFI_STATUS res;
	g_imageHandle = imageHandle;
	g_st = st;
	g_bs = st->BootServices;
	g_rs = st->RuntimeServices;

	res = g_st->ConOut->ClearScreen(g_st->ConOut);
	if (EFI_ERROR(res)) return res;

	res = st->ConOut->OutputString(st->ConOut, L"Hello from mugOS's UEFI bootloader !!\r\n");

	checkSystemTableSignature();

	// Print some UEFI stuff
	puts_noCRLF(L"Firmware vendor: ");
	puts_noCRLF(st->FirmwareVendor);
	puts_noCRLF(L" (revision: ");
	putNumber_noCRLF(st->FirmwareRevision, 16);
	puts(L")\r\n");

	// Get a GOP framebuffer
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = getGOP();
	puts_noCRLF(L"GOP is located at adress ");
	putNumberUnsigned((uint64_t) gop, 16);
	puts(L"");

	// Load kernel
	puts(L"Loading the kernel...");
	kmain_t kmain = loadKernel();
	puts(L"");

	puts(L"Quitting BootServices, get memory map, and jump to the kernel.");
	// waitKey();

	// Get the memory map...
	UINTN memoryMapSize = 0;
	EFI_MEMORY_DESCRIPTOR* memoryMap = NULL;
	UINTN mapKey, descriptorSize;
	UINT32 descriptorVersion;
	loadMemoryMap(&memoryMapSize, &memoryMap, &mapKey, &descriptorSize, &descriptorVersion);

	// ... to exit boot services
	res = st->BootServices->ExitBootServices(imageHandle, mapKey);
	if (EFI_ERROR(res)){
		puts(L"ExitBootServices failed");
		waitKey();
		return res;
	}
	g_bs = NULL; // Now we don't have access to boot services !

	// We can now jump to the kernel
	kmain(gop);

	// The kernel should never return. If it happens, reset the system
	st->RuntimeServices->ResetSystem(EfiResetCold, 0, 0, NULL);

	// // This is reached if ResetSystem returned an EFI error, which we cannot handle...
	while(true){};
	__builtin_unreachable();
}
