// UEFI bare bones example, from https://wiki.osdev.org/UEFI_App_Bare_Bones#hello.c


#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include <lib.h>



/**
@brief Verify the provided status, and if it is an error, wait for a keystroke before returning.
@return FALSE on error, otherwise TRUE
**/
BOOLEAN VerifyStatus(EFI_STATUS Status)
{
    if (!EFI_ERROR(Status))
        return TRUE;

    EFI_STATUS ConStatus = gST->ConOut->OutputString(gST->ConOut, L"Error detected while initializing Tos. Press any key to continue.");
    if (EFI_ERROR(ConStatus))
        return FALSE;

    // Wait for a keystroke before continuing, otherwise your
    // message will flash off the screen before you see it.

    // First, we need to empty the console input buffer to flush
    // out any keystrokes entered before this point
    ConStatus = gST->ConIn->Reset(gST->ConIn, FALSE);
    if (EFI_ERROR(ConStatus))
        return FALSE;

    EFI_INPUT_KEY Key;

    // Now wait until a key becomes available.
    while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == EFI_NOT_READY);        

    return FALSE;
}



/** 
@brief Open a file on disk
**/
EFI_STATUS OpenFile(CHAR16* Filename, EFI_FILE** OutFile)
{
	EFI_STATUS Status;

	// Get the Loaded Image protocol
	EFI_LOADED_IMAGE_PROTOCOL* LoadedImage = NULL;
	Status = gST->BootServices->HandleProtocol(LibImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&LoadedImage);
	if (EFI_ERROR(Status))
		return Status;
	if (LoadedImage == NULL)
		return EFI_LOAD_ERROR;		

	// Get its File System protocol
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem = NULL;
	Status = gST->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&FileSystem);
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
@brief Load our ELF from an opened file into memory and return its entry point
**/
EFI_STATUS LoadELFFromFile(EFI_FILE* File, void** OutEntry)
{


    return EFI_SUCCESS;
}



/**
@brief Load an ELF from a file with the given name and return its entry point
**/
EFI_STATUS LoadELF(CHAR16* Path, void** OutEntry)
{
	EFI_STATUS Status;
	EFI_FILE* KernelFile;
	
	// Load the file to memory
	Status = OpenFile(Path, &KernelFile);
	if (EFI_ERROR(Status))
		return Status;

    // Load the ELF into memory and get the entry point address
    void* Entry = NULL;
	Status = LoadELFFromFile(KernelFile, &Entry);
    if (EFI_ERROR(Status))
        return Status;

    *OutEntry = Entry;
	return EFI_SUCCESS;
}



/**
@brief Main entry point of our bootloader
**/
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemHandle)
{
    // Init the EFI lib
    InitializeLib(ImageHandle, SystemHandle);

    EFI_STATUS Status;

	// Load our kernel's elf from disk and get its entry point address
    void* KernelEntry = NULL;
    Status = LoadELF(L"kernel.elf", &KernelEntry);
    if (!VerifyStatus(Status))
		return Status;

	// If get here it means we succeeded actually!
	gST->ConOut->OutputString(gST->ConOut, L"Successfully loaded the kernel! Ignore the next error.\n\r");
	VerifyStatus(EFI_LOAD_ERROR);

    return Status;
}
