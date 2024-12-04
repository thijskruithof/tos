// UEFI bare bones example, from https://wiki.osdev.org/UEFI_App_Bare_Bones#hello.c


#include <efi.h>
#include <efilib.h>
#include <elf.h>



/**
@brief Verify the provided status, and if it is an error, wait for a keystroke before returning.
@return FALSE on error, otherwise TRUE
**/
BOOLEAN VerifyStatus(EFI_STATUS Status)
{
    if (!EFI_ERROR(Status))
        return TRUE;

    EFI_STATUS ConStatus = ST->ConOut->OutputString(ST->ConOut, L"Error detected while initializing Tos. Press any key to continue.");
    if (EFI_ERROR(ConStatus))
        return FALSE;

    // Wait for a keystroke before continuing, otherwise your
    // message will flash off the screen before you see it.

    // First, we need to empty the console input buffer to flush
    // out any keystrokes entered before this point
    ConStatus = ST->ConIn->Reset(ST->ConIn, FALSE);
    if (EFI_ERROR(ConStatus))
        return FALSE;

    EFI_INPUT_KEY Key;

    // Now wait until a key becomes available.
    while (ST->ConIn->ReadKeyStroke(ST->ConIn, &Key) == EFI_NOT_READY);        

    return FALSE;
}



/** 
@brief Load a file from disk
**/
EFI_STATUS LoadFile(CHAR16* Filename, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, EFI_FILE** OutFile)
{
	EFI_STATUS Status;

	// Get the Loaded Image protocol
	EFI_LOADED_IMAGE_PROTOCOL* LoadedImage = NULL;
	Status = SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&LoadedImage);
	if (EFI_ERROR(Status))
		return Status;
	if (LoadedImage == NULL)
		return EFI_LOAD_ERROR;		

	// Get its File System protocol
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem = NULL;
	Status = SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&FileSystem);
	if (EFI_ERROR(Status))
		return Status;
	if (FileSystem == NULL)
		return EFI_LOAD_ERROR;		

	// Open our File System volume (in the root directory)
	EFI_FILE* Directory = NULL;
	Status = FileSystem->OpenVolume(FileSystem, &Directory);
	if (EFI_ERROR(Status))
		return Status;
	if (Directory == NULL)
		return EFI_LOAD_ERROR;		

	// Open the specified file by its filename
	EFI_FILE* LoadedFile = NULL;
	Status = Directory->Open(Directory, &LoadedFile, Filename, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if (EFI_ERROR(Status))
		return Status;
	
	if (LoadedFile == NULL)
		return EFI_LOAD_ERROR;

	// Success!
	*OutFile = LoadedFile;
	return EFI_SUCCESS;
}



/**
@brief Load an ELF file from disk
**/
EFI_STATUS LoadELFFile(CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, void** OutKernelEntry)
{
	EFI_STATUS Status;
	EFI_FILE* Kernel;
	
	// Load the file to memory
	Status = LoadFile(Path, ImageHandle, SystemTable, &Kernel);
	if (EFI_ERROR(Status))
		return Status;

	

	return EFI_SUCCESS;
}



/**
@brief Main entry point of our bootloader
**/
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    // Init our EFI lib
    InitializeLib(ImageHandle, SystemTable);

    EFI_STATUS Status;
    void* KernelEntry = NULL;

    Status = LoadELFFile(L"kernel.elf", ImageHandle, SystemTable, &KernelEntry);
    if (!VerifyStatus(Status))
        return Status;

    return Status;
}
