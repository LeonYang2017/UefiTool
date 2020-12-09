//**        (C)Copyright 1985-2019, American Megatrends, Inc.            **
#include <Uefi.h>
#include <Library/UefiLib.h>
//#include <UefiSpec.h>
//#include <Library/DebugLib.h>
//#include <Library/BaseMemoryLib.h>
//#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
//#include <Protocol/GraphicsOutput.h>
//#include <Guid/GlobalVariable.h>
//#include <Library/OemSioLib.h>
#include <Protocol/ShellParameters.h>
#include <Protocol/Shell.h>
//#include <Protocol/Smbios.h>
//#include <Library/HobLib.h>
//#include <Library/GpioConfig.h>
//#include <Library/GpioCheckConflictLib.h>
//#include <Pi/PiHob.h>
//#include <Setup.h>
//#include <stdlib.h>
//#include <PiDxe.h>
//#include <Guid/HobList.h>
//#include <Library/HobLib.h>
//#include <Library/UefiLib.h>
//#include <Library/DebugLib.h>
//#include <Library/BaseMemoryLib.h>
/*
#define GET_GUID_HOB_DATA(HobStart) \
  (VOID *)(*(UINT8 **)&(HobStart) + sizeof (EFI_HOB_GUID_TYPE))

#define GET_GUID_HOB_DATA_SIZE(HobStart) \
  (UINT16)(GET_HOB_LENGTH (HobStart) - sizeof (EFI_HOB_GUID_TYPE))
  */



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
    EFI_STATUS                          Status;
    EFI_RESET_TYPE                      shutdown=EfiResetShutdown;
    EFI_RESET_TYPE                      coldreset=EfiResetCold;
    EFI_RESET_TYPE                      warmreset=EfiResetWarm;
    EFI_RESET_TYPE                      type;
    UINTN                               DataSize = 0;
    UINT16                              Times=0;
    
    Status = GetArg(ImageHandle);
 
    if (EFI_ERROR(Status)) {
      Print(L"Please use UEFI Shell to run this application!\n", Status);
      return Status;
    }
    if (Argc == 1){
        Print(L"usage:\n shutdown -s  : shutdown\n shutdown -rc : cold reset\n shutdown -rw : warm reset\n");
        return 0;
    }
    
    if (StrCmp(Argv[1], L"-s") == 0) {
        type = shutdown;
        if (Argc == 3){
            Times = my_atoi(Argv[2]);
            Print(L"System Stall %d seconds",Times);
            SystemTable->BootServices->Stall(Times*1000000);  
        }
    }
    if (StrCmp(Argv[1], L"-rc") == 0) {
        type = coldreset;
        if (Argc == 3){
            Times = my_atoi(Argv[2]);
            Print(L"System Stall %d s",Times);
            SystemTable->BootServices->Stall(Times*1000000);  
        }        
    }
    if (StrCmp(Argv[1], L"-rw") == 0) {
        type = warmreset;
        if (Argc == 3){
            Times = my_atoi(Argv[2]);
            Print(L"System Stall %d s",Times);
            SystemTable->BootServices->Stall(Times*1000000);  
        }        
    }    

    SystemTable->RuntimeServices->ResetSystem(type,Status,DataSize,NULL);
            
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

