// UEFI bare bones example, from https://wiki.osdev.org/UEFI_App_Bare_Bones#hello.c


#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include <lib.h>



/**
@brief Verify the provided status, and if it is an error, wait for a keystroke before returning.
@return FALSE on error, otherwise TRUE
**/
BOOLEAN VerifyStatus(EFI_STATUS inStatus)
{
    if (!EFI_ERROR(inStatus))
        return TRUE;

    EFI_STATUS con_status = gST->ConOut->OutputString(gST->ConOut, L"Error detected while initializing Tos. Press any key to continue.");
    if (EFI_ERROR(con_status))
        return FALSE;

    // Wait for a keystroke before continuing, otherwise your
    // message will flash off the screen before you see it.

    // First, we need to empty the console input buffer to flush
    // out any keystrokes entered before this point
    con_status = gST->ConIn->Reset(gST->ConIn, FALSE);
    if (EFI_ERROR(con_status))
        return FALSE;

    EFI_INPUT_KEY key;

    // Now wait until a key becomes available.
    while (gST->ConIn->ReadKeyStroke(gST->ConIn, &key) == EFI_NOT_READY);        

    return FALSE;
}



/** 
@brief Open a file on disk
**/
EFI_STATUS OpenFile(CHAR16* inFilename, EFI_FILE** outFile)
{
	EFI_STATUS status;

	// Get the Loaded Image protocol
	EFI_LOADED_IMAGE_PROTOCOL* loaded_image = NULL;
	status = gST->BootServices->HandleProtocol(LibImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&loaded_image);
	if (EFI_ERROR(status))
		return status;
	if (loaded_image == NULL)
		return EFI_LOAD_ERROR;		

	// Get its File System protocol
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* filesystem = NULL;
	status = gST->BootServices->HandleProtocol(loaded_image->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&filesystem);
	if (EFI_ERROR(status))
		return status;
	if (filesystem == NULL)
		return EFI_LOAD_ERROR;		

	// Open our File System volume (in the root directory)
	EFI_FILE* directory = NULL;
	status = filesystem->OpenVolume(filesystem, &directory);
	if (EFI_ERROR(status))
		return status;
	if (directory == NULL)
		return EFI_LOAD_ERROR;		

	// Open the specified file by its inFilename
	EFI_FILE* file = NULL;
	status = directory->Open(directory, &file, inFilename, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if (EFI_ERROR(status))
		return status;	
	if (file == NULL)
		return EFI_LOAD_ERROR;

	// Success!
	*outFile = file;
	return EFI_SUCCESS;
}



/**
@brief Load our ELF from an opened file into memory and return its entry point
**/
EFI_STATUS LoadELFFromFile(EFI_FILE* inFile, void** outEntry)
{
	EFI_STATUS status;

	// // First determine the size of our file info struct
	// UINTN fileinfo_size = 0;
	// status = inFile->GetInfo(inFile, &gEfiFileInfoGuid, &fileinfo_size, NULL);
	// if (!VerifyStatus(status))
	// 	return status;
	// if (fileinfo_size == 0)
	// 	return EFI_LOAD_ERROR;

	// // Allocate enough memory to store our file info struct in
	// EFI_FILE_INFO* fileinfo;
	// status = gST->BootServices->AllocatePool(EfiLoaderData, fileinfo_size, (void**)&fileinfo);
	// if (!VerifyStatus(status))
	// 	return status;
	// if (fileinfo == NULL)
	// 	return EFI_LOAD_ERROR;

	// // Extract the file info
	// status = inFile->GetInfo(inFile, &gEfiFileInfoGuid, &fileinfo_size, (void**)&fileinfo);
	// if (!VerifyStatus(status))
	// 	return status;
	// if (fileinfo_size == 0)
	// 	return EFI_LOAD_ERROR;		

	// Read our ELF header
	Elf64_Ehdr hdr;	
	UINTN hdr_size = sizeof(hdr);
	status = inFile->Read(inFile, &hdr_size, &hdr);
	if (!VerifyStatus(status))
		return status;	
	if (hdr_size == 0)
		return EFI_LOAD_ERROR;

	// Check first 4 characters (\177ELF)
	if (hdr.e_ident[EI_MAG0] != ELFMAG0 || hdr.e_ident[EI_MAG1] != ELFMAG1 ||
		hdr.e_ident[EI_MAG2] != ELFMAG2 || hdr.e_ident[EI_MAG3] != ELFMAG3)
	{
		return EFI_LOAD_ERROR;
	}

	// 64 bit object?
	// Executable?
	// 2's complement, little endian?
	// 64bit x86?
	// Currently support elf format version?
	if (hdr.e_ident[EI_CLASS] != ELFCLASS64 ||
		hdr.e_type != ET_EXEC ||
		hdr.e_ident[EI_DATA] != ELFDATA2LSB ||
		hdr.e_machine != EM_X86_64 ||
		hdr.e_version != EV_CURRENT)
	{
		return EFI_LOAD_ERROR;
	}

	Elf64_Phdr* phdrs;
	
	// Seek to the page headers offset in the file
	status = inFile->SetPosition(inFile, hdr.e_phoff);
	if (!VerifyStatus(status))
		return status;		

	// Calculate the size of the page headers
	UINTN phdrs_size = hdr.e_phnum * hdr.e_phentsize;
	status = gST->BootServices->AllocatePool(EfiLoaderData, phdrs_size, (void**)&phdrs);
	if (!VerifyStatus(status))
		return status;

	// Read all the page headers
	status = inFile->Read(inFile, &phdrs_size, phdrs);
	if (!VerifyStatus(status))
		return status;

	// Read all relevant page headers
	char* phdr8 = (char*)phdrs;

	while (phdr8 < phdr8 + phdrs_size)
	{
		Elf64_Phdr* phdr = (Elf64_Phdr*)phdr8;

		// Is this a loadable page?	
		if (phdr->p_type == PT_LOAD)
		{
			// Calculate number of 4096 byte pages
			int num_mem_pages = (phdr->p_memsz + 4096 - 1) / 4096;

			// Allocate memory pages at the preferred address
			Elf64_Addr mem_addr = phdr->p_paddr;
			status = gST->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, num_mem_pages, &mem_addr);
			if (!VerifyStatus(status))
				return status;

			// Seek to the start offset in the file
			status = inFile->SetPosition(inFile, phdr->p_offset);
			if (!VerifyStatus(status))
				return status;

			// Read the data from disk
			UINTN disk_size = phdr->p_filesz;
			status = inFile->Read(inFile, &disk_size, (void*)mem_addr);
			if (!VerifyStatus(status))
				return status;
		}		

		// Move our ptr forward
		phdr8 += hdr.e_phentsize;
	}

	// Store the entry point address
	*outEntry = (void*)hdr.e_entry;

    return EFI_SUCCESS;
}



/**
@brief Load an ELF from a file with the given name and return its entry point
**/
EFI_STATUS LoadELF(CHAR16* inPath, void** outEntry)
{
	EFI_STATUS status;
	EFI_FILE* file = NULL;
	
	// Load the file to memory
	status = OpenFile(inPath, &file);
	if (EFI_ERROR(status))
		return status;
	if (file == NULL)
		return EFI_LOAD_ERROR;

    // Load the ELF into memory and get the entry point address
    void* entry = NULL;
	status = LoadELFFromFile(file, &entry);
    if (EFI_ERROR(status))
        return status;

    *outEntry = entry;
	return EFI_SUCCESS;
}



/**
@brief Main entry point of our bootloader
**/
EFI_STATUS efi_main(EFI_HANDLE inImageHandle, EFI_SYSTEM_TABLE* inSystemHandle)
{
    // Init the EFI lib
    InitializeLib(inImageHandle, inSystemHandle);

    EFI_STATUS status;

	// Load our kernel's elf from disk and get its entry point address
    void* kernel_entry = NULL;
    status = LoadELF(L"kernel.elf", &kernel_entry);
    if (!VerifyStatus(status))
		return status;

	// If get here it means we succeeded actually!
	gST->ConOut->OutputString(gST->ConOut, L"Successfully loaded the kernel! Ignore the next error.\n\r");
	VerifyStatus(EFI_LOAD_ERROR);

    return status;
}
