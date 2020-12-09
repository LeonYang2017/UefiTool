#include <Efi.h>
#include <AmiDxeLib.h>
#include <Protocol/SmbiosDynamicData.h>
#include <Protocol/SmBus.h>
#include <Protocol/AmiSmbios.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Guid/GlobalVariable.h>
#include <Protocol/Shell.h>
#include <Protocol/Smbios.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseMemoryLib/MemLibInternals.h>

UINT8 Strlenleon(char *string) {
    UINT8 length=0;
    while(*string++) length++;
    return length;
}

CHAR8 *
MB_Get_String_By_Num (
  IN VOID   *SmBiosPtr,
  IN UINT8  StringNum
  ) 
{
  EFI_SMBIOS_TABLE_HEADER   *SmbiosTableHeader;
  CHAR8                     *StringPtr;
  CHAR8                     *String;
  UINTN                     StrSize;
  
  SmbiosTableHeader = (EFI_SMBIOS_TABLE_HEADER *) SmBiosPtr;
  if (SmBiosPtr == NULL) {
    return NULL;
  }
  StringPtr = (CHAR8 *)((UINTN)SmBiosPtr + SmbiosTableHeader->Length);
  while (--StringNum) {
    while (*StringPtr != 0) {
      StringPtr++;
    }
    StringPtr++;
  }

  StrSize = AsciiStrSize (StringPtr);
  String = AllocateZeroPool (StrSize);
  if (String == NULL) {
    return NULL;
  }

  CopyMem (String, StringPtr, StrSize);
  return String;
}

EFI_STATUS 
EFIAPI
MB_Get_Smbios_By_Type (
  IN  EFI_SMBIOS_PROTOCOL             *This,
  IN  UINT8                           Type,
  IN  OUT UINTN                       **SmbiosTableHeader,
  IN  OUT UINTN                       *MatchedCount
  )
{
  EFI_STATUS                Status;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  EFI_SMBIOS_TABLE_HEADER   *Record;
  UINTN                     Count;
  UINTN                     *SmBiosAddr;
  UINTN                     Index;
  
  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Record       = NULL;
  Status       = EFI_NOT_FOUND;
  Count        = 0;
  
  do {
    Status = This->GetNext (This, &SmbiosHandle, NULL, &Record, NULL);
    if (EFI_ERROR (Status)) {
      break;
    }

    if (SmbiosHandle == SMBIOS_HANDLE_PI_RESERVED) {
      break;
    }
    //
    // found matched type count;
    //
    if (Record->Type == Type) {
      Count ++;
    }
  } while (!EFI_ERROR (Status));
    
  if (!Count) {
    return EFI_NOT_FOUND;
  } 

  SmBiosAddr = (UINTN *) AllocateZeroPool (sizeof (UINTN) * Count);
  if (SmBiosAddr == NULL) {
    return EFI_NOT_FOUND;
  }
  
  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Index = 0;
  do {
    Status = This->GetNext (This, &SmbiosHandle, NULL, &Record, NULL);
    if (EFI_ERROR (Status)) {
      break;
    }
    if (SmbiosHandle == SMBIOS_HANDLE_PI_RESERVED) {
      break;
    }
    //
    // Matched smbios type, than break;
    //
    if (Record->Type == Type) {
      SmBiosAddr[Index] = (UINTN) Record;
      Index ++;
    }
  } while (!EFI_ERROR (Status));

  *SmbiosTableHeader = SmBiosAddr;
  *MatchedCount      = Count;
  return EFI_SUCCESS;
}

CHAR8 *
MB_Get_Bios_Version (
  IN EFI_SMBIOS_PROTOCOL             *This
  )
{
  EFI_STATUS            Status;
  UINTN                 *Addr;
  UINTN                 Count;
  CHAR8                 *t;
  SMBIOS_TABLE_TYPE0    *Type0;

  //
  // Get Smbios type0 
  //
  Status = MB_Get_Smbios_By_Type (
             This, 
             SMBIOS_TYPE_BIOS_INFORMATION, 
             &Addr, 
             &Count
             );
  if (EFI_ERROR (Status)) {
//    MB_DEBUG ((EFI_D_ERROR, "%a(%d) load smbios type 0 failed\n", __FUNCTION__, __LINE__));
    return NULL;    
  }

  //
  // Update SMBIOS type0 bios version info into mainboard sw info
  //
  Type0 = (SMBIOS_TABLE_TYPE0 *)(UINTN) Addr[0];
  t = MB_Get_String_By_Num ((VOID *)Type0, Type0->BiosVersion); 
  if (t == NULL) {
    //MB_DEBUG ((EFI_D_ERROR, "%a(%d): Can't found bios version string\n", __FUNCTION__, __LINE__));
    return NULL;
  }
  if (Addr != NULL) {
    FreePool (Addr);
  }
  return t;
}

/**
  ME Disable Entry point

  @param[in]  ImageHandle     The image handle.
  @param[in]  SystemTable     The system table.

  @retval EFI_SUCCESS            Command completed successfully.
  @retval EFI_INVALID_PARAMETER  Command usage error.
  @retval EFI_NOT_FOUND          The input file can't be found.
**/
EFI_STATUS
EFIAPI
MeDisableEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
    //EFI_GUID              gAmiSmbiosProtocolGuid={0x5e90a50d, 0x6955, 0x4a49, { 0x90, 0x32, 0xda, 0x38, 0x12, 0xf8, 0xe8, 0xe5 }};
    EFI_GUID              gEfiSmbiosProtocolGuid = {0x3583ff6, 0xcb36, 0x4940, { 0x94, 0x7e, 0xb9, 0xb3, 0x9f, 0x4a, 0xfa, 0xf7}};
    //AMI_SMBIOS_PROTOCOL   *gAmiSmbiosProtocol = NULL;
    EFI_STATUS            Status;
    //UINT8                 BIOSVersion = 0;
    //UINT8                 Type0Data[sizeof(SMBIOS_BIOS_INFO)];
    //UINT16                Type0DataSize = sizeof(SMBIOS_BIOS_INFO);
    //UINT8                 *Type0DataPtr = &Type0Data[0];
    UINT8                 len = 0;
    char*                 p = '\0';
    char                  Test = 'B';
//    UINT8                 MajorVersion;
//    UINT8                 MinorVersion;
    CHAR8                 *t;
    EFI_SMBIOS_PROTOCOL   *EfiSmbiosProtocol = NULL;
    
    Print(L"Start to locate protocol\n");  
    Status = SystemTable->BootServices->LocateProtocol(&gEfiSmbiosProtocolGuid, NULL, (VOID **)&EfiSmbiosProtocol);
    if (Status == EFI_SUCCESS) {
  // Read Smbios Type 0 structure, update EC version and write it back
                        Print(L"Start to read SMBIOS protocol\n");                         
                        t = MB_Get_Bios_Version (EfiSmbiosProtocol);
                        Print(L"Operation Successfully!\n");     
                        Print(L"BIOS Version is: %c%c%c%c%c%c%c%c\n", *t,*(t+1),*(t+2),*(t+3),*(t+4),*(t+5),*(t+6),*(t+7));
                        p = t;
                        len = Strlenleon(t);
                        t = &p[len-1];
                        Print(L"Major Version is: %c\n",*t);
                        if (*t == Test) {
                             return 0;
                        };
                        return 1;         
    }else{  
        Print(L"EFI_ERROR!Can not LocateProtocol : gAmiSmbiosProtocolGuid\n");
    }
    
    return 2;
}
