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
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Guid/GlobalVariable.h>
#include <Library/OemSioLib.h>
#include <Protocol/ShellParameters.h>
#include <Protocol/Shell.h>
#include <Protocol/Smbios.h>
//#include <Setup.h>
#include <stdlib.h>

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

EFI_STATUS EFIAPI UefiMain(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
)
{
    EFI_STATUS            Status;
    int                   Times;
    UINT8                 i;
    
    Status = GetArg(ImageHandle);
 
    if (EFI_ERROR(Status)) {
      Print(L"Please use UEFI SHELL to run this application!\n", Status);
      return Status;
   }
    if (Argc == 1){
        Print(L"No Parameter!\n"); 
        Print(L"usage: HelloWorld.efi -2\n"); 
        return 1;
    }
    if(*Argv[1] == '-') *Argv[1]++;
    Times = atoi((char *)Argv[1]);
    for (i=0;i<Times;i++){
        Print(L"HelloWorld\n"); 
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
