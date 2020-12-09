//**        (C)Copyright 1985-2019, American Megatrends, Inc.            **
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
#include <Protocol/ShellParameters.h>


/**
  Application entry point.

  Arguments:
    ImageHandle    The image handle.
    SystemTable    The system table.

  Retruns:
    EFI_OUT_OF_RESOURCES   There is not enough pool memory to allocate.
    EFI_INVALID_PARAMETER  One of the parameters passed to EFI
                           functions has invalid value.
    EFI_NOT_FOUND          No instance of VROC device info protocol was found.
    EFI_SUCCESS            NVMe devices data was printed to console.
**/
UINTN  Argc;
CHAR16 **Argv;

GLOBAL_REMOVE_IF_UNREFERENCED EFI_GUID gEfiShellParametersProtocolGuid = { 0x752f3136, 0x4e16, 0x4fdc, {0xa2, 0x2a, 0xe5, 0xf4, 0x68, 0x12, 0xf4, 0xca }};
//GLOBAL_REMOVE_IF_UNREFERENCED EFI_GUID gGpioCheckConflictHobGuid = {0x5603f872, 0xefac, 0x40ae, {0xb9, 0x7e, 0x13, 0xb2, 0xf8, 0x07, 0x80, 0x21}};

EFI_STATUS
GetArg (
  EFI_HANDLE        ImageHandle
  )
{
  EFI_STATUS                    Status;
  EFI_SHELL_PARAMETERS_PROTOCOL *ShellParameters;

  Status = gBS->HandleProtocol (
                  gImageHandle,
                  &gEfiShellParametersProtocolGuid,
                  (VOID**)&ShellParameters
                  );
  if (EFI_ERROR(Status)) {
    Print(L"Can't handle the shell Parameter!\n");
    return Status;
  }

  Argc = ShellParameters->Argc;
  Argv = ShellParameters->Argv;
  return EFI_SUCCESS;
}

UINT16 my_atoi(const CHAR16 *str)
{
    if(NULL == str)
        return 0;
    UINT16 ret = 0;
    while(0 != *str)
    {
        //Print(L"string is %d\n",*str-'0');
        ret = ret * 10 + (*str-'0');
        str++;
    }
    return ret;
}

EFI_STATUS EFIAPI UefiMain(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
)
{
//    EFI_STATUS                          Status;
    UINT8                               data = 0;
    
    Print(L"The system enters debug mode,Please check IO 80 port...\n");
    while(1){
        SystemTable->BootServices->Stall(1000000);
        IoWrite8(0x80,data);
        data++;
        if(data == 0xA){
            data = 0;
        }
    }
    
    return EFI_SUCCESS;
}
//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2019, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
