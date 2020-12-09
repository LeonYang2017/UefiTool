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
    EFI_TIME                            Now;
    EFI_TIME                            Waketime;
    EFI_TIME                            SetTime;
    BOOLEAN                             Enabled;
    BOOLEAN                             Pending;
    EFI_TIME_CAPABILITIES               *Capabilities=NULL;
    UINT16                              Year=2021;
    UINT8                               Month=1;
    UINT8                               Day=1;
    UINT8                               Hour=0;
    UINT8                               Minute=0;
    UINT8                               Second=0;
    CHAR16                              *String = NULL;
    UINT16                              val;
    BOOLEAN                             ShowOnly = FALSE;
    
    Status = GetArg(ImageHandle);
 
    if (EFI_ERROR(Status)) {
      Print(L"Please use UEFI Shell to run this application!\n", Status);
      return Status;
    }
    if (Argc == 1){
        goto exit;
    }
    if (Argc == 2){
        ShowOnly = TRUE;
    }
    //Print(L"Argc = %d\n",Argc);
    Status = SystemTable->RuntimeServices->GetTime(&SetTime,Capabilities);
    if(!ShowOnly){
        if (StrCmp(Argv[1], L"-alarm") == 0){
            Print(L"Reading Wakeup Alarm Clock...\n");
        }else{
            String = Argv[2];
            val = my_atoi(String);
        }
    }
    //Print(L"val is :%d\n",val);
    if (StrCmp(Argv[1], L"-y") == 0) {
        if (!ShowOnly){       
            Year = val;
            SetTime.Year = Year;
            Status = SystemTable->RuntimeServices->SetTime(&SetTime);
        }else{
            Print(L"%d\n",SetTime.Year);
            return 0;
        }
        goto exit;
    }
    if (StrCmp(Argv[1], L"-m") == 0) {
        if (!ShowOnly){       
            Month = (UINT8)val;
            SetTime.Month = Month;
            Status = SystemTable->RuntimeServices->SetTime(&SetTime);
        }else{
            Print(L"%d\n",SetTime.Month);
            return 0;
        }
        goto exit;
    }
    if (StrCmp(Argv[1], L"-d") == 0) {
        if (!ShowOnly){       
            Day = (UINT8)val;
            SetTime.Day = Day;
            Status = SystemTable->RuntimeServices->SetTime(&SetTime);
        }else{
            Print(L"%d\n",SetTime.Day);
            return 0;
        }
        goto exit;
    }    
    if (StrCmp(Argv[1], L"-h") == 0) {
        if (!ShowOnly){       
            Hour = (UINT8)val;
            SetTime.Hour = Hour;
            Status = SystemTable->RuntimeServices->SetTime(&SetTime);
        }else{
            Print(L"%d\n",SetTime.Hour);
            return 0;
        }
        goto exit;
    } 
    if (StrCmp(Argv[1], L"-min") == 0) {
        if (!ShowOnly){       
            Minute = (UINT8)val;
            SetTime.Minute = Minute;
            Status = SystemTable->RuntimeServices->SetTime(&SetTime);
        }else{
            Print(L"%d\n",SetTime.Minute);
            return 0;
        }
        goto exit;
    } 
    if (StrCmp(Argv[1], L"-s") == 0) {
        if (!ShowOnly){       
            Second = (UINT8)val;
            SetTime.Second = Second;
            Status = SystemTable->RuntimeServices->SetTime(&SetTime);
        }else{
            Print(L"%d\n",SetTime.Second);
            return 0;
        }
        goto exit;
    } 
    if (StrCmp(Argv[1], L"-alarm") == 0) {
        if(StrCmp(Argv[2], L"-h") == 0){
            if (Argc == 4){
                String = Argv[3];
                val = my_atoi(String);
                SetTime.Hour += val;
            }else{
                SetTime.Hour += 1;
            }
            Enabled = TRUE;
            Status = SystemTable->RuntimeServices->SetWakeupTime(Enabled,&SetTime);
            goto alarm; 
        }
        if(StrCmp(Argv[2], L"-m") == 0){
            if (Argc == 4){
                String = Argv[3];
                val = my_atoi(String);
                SetTime.Minute += val;
            }else{
                SetTime.Minute += 5;
            }
            Enabled = TRUE;
            Status = SystemTable->RuntimeServices->SetWakeupTime(Enabled,&SetTime);
            goto alarm;       
        }
        if(StrCmp(Argv[2], L"-s") == 0){
            if (Argc == 4){
                String = Argv[3];
                val = my_atoi(String);
                SetTime.Second += val;
            }else{
                SetTime.Second += 60;
            }
            Enabled = TRUE;
            Status = SystemTable->RuntimeServices->SetWakeupTime(Enabled,&SetTime);
            goto alarm;                            
        }
        
        //SetTime.Minute += 5;
        //Enabled = TRUE;
        //Print(L"The computer will wake up after 5 minutes by default\n");
        //Status = SystemTable->RuntimeServices->SetWakeupTime(Enabled,&SetTime);
        
        goto alarm;          
    }
        
    exit:
    Status = SystemTable->RuntimeServices->GetTime(&Now,Capabilities);    
    Print(L"Current System Time: %d-%d-%d %d:%d:%d\n",Now.Year,Now.Month,Now.Day,Now.Hour,Now.Minute,Now.Second);
    
    alarm:
    if (StrCmp(Argv[1], L"-alarm") == 0) {  
        Status = SystemTable->RuntimeServices->GetWakeupTime(&Enabled,&Pending,&Waketime);
        if (Enabled){
            Print(L"Current Wakeup Alarm Clock is: %d-%d-%d %d:%d:%d\n",Waketime.Year,Waketime.Month,Waketime.Day,Waketime.Hour,Waketime.Minute,Waketime.Second);
        }else{
            Print(L"Current Wakeup Alarm Clock is Disabled!\nPlease enabled it in bios setup!\n");
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
