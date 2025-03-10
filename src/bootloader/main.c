#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include <lib.h>


#define VERIFY(status) \
	if (EFI_ERROR(status)) TerminateOnError(status, "Error status", __FUNCTION__, __LINE__);

#define ASSURE(cond) \
	if (!(cond)) TerminateOnError(EFI_SUCCESS, #cond, __FUNCTION__, __LINE__);

/**
@brief Verify the provided status, and if it is an error, wait for a keystroke before returning.
@return FALSE on error, otherwise TRUE
**/
void TerminateOnError(EFI_STATUS inStatus, const char* inCond, const char* inErrorFunc, int inErrorLine)
{
	EFI_STATUS con_status;

    Print(L"Error while initializing Tos.\n\r");
	Print(L"Condition: %a\n\r", inCond);
	if (EFI_ERROR(inStatus))
		Print(L"EFI Error: %r\n\r", inStatus);
	Print(L"Location: %a (at line %d)\n\r", inErrorFunc, inErrorLine);

    // Wait for a keystroke before continuing, otherwise your
    // message will flash off the screen before you see it.

    // First, we need to empty the console input buffer to flush
    // out any keystrokes entered before this point
    con_status = gST->ConIn->Reset(gST->ConIn, FALSE);
    if (EFI_ERROR(con_status))
		return;

	while (TRUE)
	{
    	EFI_INPUT_KEY key;

    	// Now wait until a key becomes available.
    	while (gST->ConIn->ReadKeyStroke(gST->ConIn, &key) == EFI_NOT_READY);        
	}
}



/** 
@brief Open a file on disk
**/
EFI_FILE* OpenFile(CHAR16* inFilename)
{
	// Get the Loaded Image protocol
	EFI_LOADED_IMAGE_PROTOCOL* loaded_image = NULL;
	VERIFY(gST->BootServices->HandleProtocol(LibImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&loaded_image));
	ASSURE(loaded_image != NULL);	

	// Get its File System protocol
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* filesystem = NULL;
	VERIFY(gST->BootServices->HandleProtocol(loaded_image->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&filesystem));
	ASSURE(filesystem != NULL);		

	// Open our File System volume (in the root directory)
	EFI_FILE* directory = NULL;
	VERIFY(filesystem->OpenVolume(filesystem, &directory));
	ASSURE(directory != NULL);		

	// Open the specified file by its inFilename
	EFI_FILE* file = NULL;
	VERIFY(directory->Open(directory, &file, inFilename, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY));
	ASSURE(file != NULL);

	// Success!
	return file;
}



/**
@brief Load our ELF from an opened file into memory and return its entry point
**/
void* LoadELFFromFile(EFI_FILE* inFile)
{
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
	VERIFY(inFile->Read(inFile, &hdr_size, &hdr));
	ASSURE(hdr_size > 0);

	// Check first 4 characters (\177ELF)
	ASSURE(hdr.e_ident[EI_MAG0] == ELFMAG0);
	ASSURE(hdr.e_ident[EI_MAG1] == ELFMAG1);
	ASSURE(hdr.e_ident[EI_MAG2] == ELFMAG2);
	ASSURE(hdr.e_ident[EI_MAG3] == ELFMAG3);

	// 64 bit object?
	// Executable
	// 2's complement, little endian?
	// 64bit x86?
	// Currently support elf format version?
	ASSURE(hdr.e_ident[EI_CLASS] == ELFCLASS64);
	ASSURE(hdr.e_type == ET_EXEC);
	ASSURE(hdr.e_ident[EI_DATA] == ELFDATA2LSB);
	ASSURE(hdr.e_machine == EM_X86_64);
	ASSURE(hdr.e_version == EV_CURRENT);

	Elf64_Phdr* phdrs;
	
	// Seek to the page headers offset in the file
	VERIFY(inFile->SetPosition(inFile, hdr.e_phoff));

	// Calculate the size of the page headers
	UINTN phdrs_size = hdr.e_phnum * hdr.e_phentsize;
	VERIFY(gST->BootServices->AllocatePool(EfiLoaderData, phdrs_size, (void**)&phdrs));

	// Read all the page headers
	VERIFY(inFile->Read(inFile, &phdrs_size, phdrs));

	// Read all relevant page headers
	char* phdr8 = (char*)phdrs;
	char* phdrs_end8 = phdr8 + phdrs_size;

	while (phdr8 < phdrs_end8)
	{
		Elf64_Phdr* phdr = (Elf64_Phdr*)phdr8;

		// Is this a loadable page?	
		if (phdr->p_type == PT_LOAD)
		{
			Print(L"Loading program header: MemSiz: %d, PhysAddr: %llx\n\r", phdr->p_memsz, phdr->p_paddr);

			// Calculate number of 4096 byte pages
			int num_mem_pages = (phdr->p_memsz + 4096 - 1) / 4096;

			// Allocate memory pages at the preferred address
			Elf64_Addr mem_addr = phdr->p_paddr;
			VERIFY(gST->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, num_mem_pages, &mem_addr));

			// Seek to the start offset in the file
			VERIFY(inFile->SetPosition(inFile, phdr->p_offset));

			// Read the data from disk
			UINTN disk_size = phdr->p_filesz;
			VERIFY(inFile->Read(inFile, &disk_size, (void*)mem_addr));
		}		

		// Move our ptr forward
		phdr8 += hdr.e_phentsize;
	}

	// Return the entry point address
	return (void*)hdr.e_entry;
}



/**
@brief Load an ELF from a file with the given name and return its entry point
**/
void* LoadELF(CHAR16* inPath)
{
	// Load the file to memory
	EFI_FILE* file = OpenFile(inPath);

    // Load the ELF into memory and get the entry point address
    return LoadELFFromFile(file);
}



/**
@brief Main entry point of our bootloader
**/
EFI_STATUS efi_main(EFI_HANDLE inImageHandle, EFI_SYSTEM_TABLE* inSystemHandle)
{
    // Init the EFI lib
    InitializeLib(inImageHandle, inSystemHandle);

	// Load our kernel's elf from disk and get its entry point address
    // void* kernel_entry = 
	LoadELF(L"kernel.elf");
    
	// If get here it means we succeeded actually!
	gST->ConOut->OutputString(gST->ConOut, L"Successfully loaded the kernel! Ignore the next error.\n\r");
	ASSURE(FALSE);

    return EFI_SUCCESS;
}
