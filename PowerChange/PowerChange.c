#include <Uefi.h>
#include <Library/BaseLib.h>
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
#include <Setup.h>

#define SETUP_GUID { 0xEC87D643, 0xEBA4, 0x4BB5, 0xA1, 0xE5, 0x3F, 0x3E, 0x36, 0xB2, 0x0D, 0xA9 }

UINTN  Argc;
CHAR16 **Argv;

//
//Nano project bios id list
//
UINT32 NanoBIOSIdList [] = {
  SIGNATURE_32 ('M', '2', 'A', 0)
};
/**
  Print APP usage.
**/
VOID
PrintUsage (
  VOID
  )
{
  Print(L"PowerState:  usage\n");
  Print(L"  PowerState -pf/--poweroff\n");
  Print(L"  PowerState -po/--poweron\n");
  Print(L"  PowerState -ls/--laststate\n");
}

/**

  This function parse application ARG.
  @param  ImageHandle
  @return Status
**/
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
  EFI_STATUS            Status;
  EFI_GUID              gSetupGuid = SETUP_GUID;
  SETUP_DATA            SetupData;
  UINTN                 DataSize = 0;
  UINT32                Attributes = 0;
  
  //
   //Get input arg
   //
   Status = GetArg(ImageHandle);
   if (EFI_ERROR(Status)) {
     Print(L"Please use UEFI SHELL to run this application!\n", Status);
     return Status;
   }
   if (Argc != 2) {
     Print(L"Error Parameters input, please see usage below!\n", Status);
     PrintUsage();
     return EFI_INVALID_PARAMETER;
   }
   
   
   //
     //arg "pf" means power off
     //
     DataSize = sizeof(SETUP_DATA);
     if ((StrCmp(Argv[1], L"-pf") == 0) || (StrCmp(Argv[1], L"--poweroff") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
              if ((!EFI_ERROR(Status))){
                  SetupData.AC_Support = 0;
                            
                  Status = gRT->SetVariable (
                                              L"Setup",
                                              &gSetupGuid,
                                              Attributes,
                                              sizeof(SETUP_DATA),
                                              &SetupData
                            );
         
              }
              Status = gRT->GetVariable(
                                          L"Setup",
                                          &gSetupGuid,
                                          &Attributes,
                                          &DataSize,
                                          &SetupData);
              if((!EFI_ERROR(Status))){
                  if (SetupData.AC_Support == 0){
                      Print(L"Operation Successfully!\n");
                  }else{
                      Print(L"Operation Failed!\n");
                  }
              }
              return Status;
     }
     
     //
     //arg "po" means poweron
     //
     if ((StrCmp(Argv[1], L"-po") == 0) || (StrCmp(Argv[1], L"--poweron") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
              if ((!EFI_ERROR(Status))){
                  SetupData.AC_Support = 1;
                            
                  Status = gRT->SetVariable (
                                              L"Setup",
                                              &gSetupGuid,
                                              Attributes,
                                              sizeof(SETUP_DATA),
                                              &SetupData);
              }
              Status = gRT->GetVariable(
                                          L"Setup",
                                          &gSetupGuid,
                                          &Attributes,
                                          &DataSize,
                                          &SetupData
                                          );
              if((!EFI_ERROR(Status))){
                  if (SetupData.AC_Support == 1){
                      Print(L"Operation Successfully!\n");
                  }else{
                      Print(L"Operation Failed!\n");
                  }
              }
              return Status;
     }
     //
     //arg "l" means laststate
     //
     if ((StrCmp(Argv[1], L"-ls") == 0) || (StrCmp(Argv[1], L"--laststate") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.AC_Support = 2;
                            
             Status = gRT->SetVariable (
                                              L"Setup",
                                              &gSetupGuid,
                                              Attributes,
                                              sizeof(SETUP_DATA),
                                              &SetupData
                                         );
         }
         
         Status = gRT->GetVariable(
                                                     L"Setup",
                                                     &gSetupGuid,
                                                     &Attributes,
                                                     &DataSize,
                                                     &SetupData
                                                  );
         if((!EFI_ERROR(Status))){
             if (SetupData.AC_Support == 2){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return Status;
     }
     Print(L"Error Parameters input, please see usage below!\n", Status);
     PrintUsage();
     return Status;
     
}
