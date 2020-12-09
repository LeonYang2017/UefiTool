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

#define SETUP_GUID             { 0xEC87D643, 0xEBA4, 0x4BB5, 0xA1, 0xE5, 0x3F, 0x3E, 0x36, 0xB2, 0x0D, 0xA9 }
#define SETUP_FORM_SET_GUID    { 0x7B59104A, 0xC00D, 0x4158, 0x87, 0xFF, 0xF0, 0x4D, 0x63, 0x96, 0xA9, 0x15 }

UINTN  Argc;
CHAR16 **Argv;

typedef struct{
    UINT8 SecureBootSupport;   ///< Setup control
    UINT8 SecureBootMode;      ///< Setup control
    UINT8 DefaultKeyProvision; ///< Setup control
    UINT8 Reserved;            ///< reserved
    UINT8 Load_from_OROM;      ///< Setup control
    UINT8 Load_from_REMOVABLE_MEDIA; ///< Setup control
    UINT8 Load_from_FIXED_MEDIA; ///< Setup control
} SECURE_BOOT_SETUP_VAR;
/**
  Print APP usage.
**/
VOID
PrintUsage (
  VOID
  )
{
  Print(L"  CSI:  usage\n");
  Print(L"  CSI -csb : Check secure boot status\n");
  Print(L"  CSI -dsb/-disablesecureboot : Disable Secure Boot\n");
  Print(L"  CSI -esb/-enablesecureboot : Enable Secure Boot\n");
  Print(L"  CSI -afb/-allowflasholdbios : Allow flash older bios\n");
  Print(L"  CSI -nafb/-notallowflasholdbios : Not allow flash older bios\n");
  Print(L"  CSI -enopt/-enosoptimized : Enabled OS Optimized\n");
  Print(L"  CSI -disopt/-disosoptimized : Disabled OS Optimized\n");
  Print(L"  CSI -po : Set last power status is power on\n");
  Print(L"  CSI -pf : Set last power status is power off\n");
  Print(L"  CSI -ls : Set last power status is last state\n");
  Print(L"  CSI -endevguard : Enabled deviceGuard\n");
  Print(L"  CSI -disdevguard : Disabled deviceGuard\n");
  Print(L"  CSI -ahci : Set SATA AHCI mode\n");
  Print(L"  CSI -rst : Set SATA RST mode\n");
  Print(L"  CSI -raid : Set SATA RAID mode\n");
  Print(L"  CSI -enhttp : Enabled Http boot\n");
  Print(L"  CSI -dishttp : Disabled Http boot\n");
  Print(L"  CSI -envtd : Enabled VT-d\n");
  Print(L"  CSI -disvtd : Disabled VT-d\n");
  Print(L"  CSI -enms : Enabled Windows Modern Standby\n");
  Print(L"  CSI -disms : Disabled Windows Modern Standby\n");
  Print(L"  CSI -enwu : Enabled Windows UEFI Firmware Update\n");
  Print(L"  CSI -diswu : Disabled Windows UEFI Firmware Update\n");
  Print(L"  CSI -dtpm : Set TCG Security Device Discrete TPM\n");
  Print(L"  CSI -ftpm : Set TCG Security Device Firmware TPM\n");
  Print(L"  CSI -enserchip/entpm : Enabled security chip\n");
  Print(L"  CSI -disserchip/distpm : Disabled security chip\n");
  Print(L"  CSI -encpt : Enabled computrace\n");
  Print(L"  CSI -discpt : Disabled computrace\n");
  Print(L"  CSI -perdiscpt : Permanently disabled computrace\n");
  Print(L"  CSI -encsm : Enabled CSM\n");
  Print(L"  CSI -discsm : Disabled CSM\n");
  Print(L"  CSI -blink : Enabled blink\n");
  Print(L"  CSI -noblink : Disabled blink\n");
  Print(L"  CSI -r0 : Return code 0\n");
  Print(L"  CSI -r1 : Return code 1\n");
  Print(L"  If you wanna support more item,please contact:leonryan@qq.com\n");
}

EFI_STATUS
EFIAPI
DisableLCD (
  VOID
  ) 
{
  UINT8  Data = 0;
  GetValueWithIO (0x02, 0xBA, &Data);
  Data &= ~BIT0;    //clear bit0
  Data &= ~BIT1;    //clear bit1
  SetValueWithIO (0x02, 0xBA, Data);
  gBS->Stall (1000);
  GetValueWithIO (0x02, 0xBA, &Data);
  if (Data & BIT0) {
    Print (L"Disable LCD Blinking Failed\n");
    return EFI_DEVICE_ERROR;
  }
  Print (L"Disable LCD Blinking Successfully\n");
  return EFI_SUCCESS;
}

/**
  This function Enable LCD.
  @return EFI_SUCCESS disable ME successfuly, others failed.
**/
EFI_STATUS
EFIAPI 
EnableLCD (
  VOID
  )
{
  UINT8  Data = 0;
  GetValueWithIO (0x2, 0xBA, &Data);
  Data = 0xF2;   //F2 to 02,speed upper and upper
  SetValueWithIO (0x02, 0xBA, Data);
  gBS->Stall (1000);
  GetValueWithIO (0x2, 0xBA, &Data);
  if (Data & BIT1) {
      if(Data | BIT0){
          Print (L"Enable LCD Blinking Successfully\n");
          return EFI_SUCCESS;  
      }else{
          Print (L"Enable LCD Blinking Failed\n");
      }
  }
  Print (L"Enable LCD Blinking Failed\n");
  return EFI_DEVICE_ERROR;  
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
  EFI_GUID              guidSecurity = SETUP_FORM_SET_GUID;
  SETUP_DATA            SetupData;
  SECURE_BOOT_SETUP_VAR SecData;
  UINTN                 DataSize = 0;
  UINTN                 SecDataSize = 0;
  UINT32                Attributes = 0;
  UINT32                time = 5;
  UINT8                 i=0;
  UINT8                 j=0;
  UINT32                count=0;
  
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
   
     DataSize = sizeof(SETUP_DATA);
     SecDataSize = sizeof(SECURE_BOOT_SETUP_VAR);
     if (StrCmp(Argv[1], L"-sleep") == 0) {
         time = 22;
         for(i=0;i<100;i++){
             for(j=0;j<time;j++){
                 count++;                
             }
         }
         return 886;
     }
     if (StrCmp(Argv[1], L"-r0") == 0) {
                   return 0;
          }
     if (StrCmp(Argv[1], L"-r1") == 0) {
                   return 1;
               }
     if ((StrCmp(Argv[1], L"-afb") == 0) || (StrCmp(Argv[1], L"-allowflasholdbios") == 0)) {
             Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
              if ((!EFI_ERROR(Status))){
                  SetupData.RollBackSupport = 1;
                            
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
                  if (SetupData.RollBackSupport == 1){
                      Print(L"Operation Successfully!\n");
                  }else{
                      Print(L"Operation Failed!\n");
                  }
              }
              return 0;
     }
     
     //
     //arg "po" means poweron
     //
     if ((StrCmp(Argv[1], L"-nafb") == 0) || (StrCmp(Argv[1], L"-notallowflasholdbios") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
              if ((!EFI_ERROR(Status))){
                  SetupData.RollBackSupport = 0;
                            
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
                  if (SetupData.RollBackSupport == 0){
                      Print(L"Operation Successfully!\n");
                  }else{
                      Print(L"Operation Failed!\n");
                  }
              }
              return 0;
     }

     if ((StrCmp(Argv[1], L"-dsb") == 0) || (StrCmp(Argv[1], L"-disablesecureboot") == 0)) {
         Status = gRT->GetVariable(
                                            L"SecureBootSetup",
                                            &guidSecurity,
                                            &Attributes,
                                            &SecDataSize,
                                            &SecData
                                         );
                        
              if ((!EFI_ERROR(Status))){
                  SecData.SecureBootSupport = 0;
                            
                  Status = gRT->SetVariable (
                                              L"SecureBootSetup",
                                              &guidSecurity,
                                              Attributes,
                                              sizeof(SECURE_BOOT_SETUP_VAR),
                                              &SecData);
              }
              Status = gRT->GetVariable(
                                          L"SecureBootSetup",
                                          &guidSecurity,
                                          &Attributes,
                                          &SecDataSize,
                                          &SecData
                                          );
              if((!EFI_ERROR(Status))){
                  if (SecData.SecureBootSupport == 0){
                      Print(L"Operation Successfully!\n");
                  }else{
                      Print(L"Operation Failed!\n");
                  }
              }
              return 0;
     }

     if ((StrCmp(Argv[1], L"-ensb") == 0) || (StrCmp(Argv[1], L"-enablesecureboot") == 0)) {
         Status = gRT->GetVariable(
                                            L"SecureBootSetup",
                                            &guidSecurity,
                                            &Attributes,
                                            &SecDataSize,
                                            &SecData
                                         );
                        
              if ((!EFI_ERROR(Status))){
                  SecData.SecureBootSupport = 1;
                            
                  Status = gRT->SetVariable (
                                              L"SecureBootSetup",
                                              &guidSecurity,
                                              Attributes,
                                              sizeof(SECURE_BOOT_SETUP_VAR),
                                              &SecData);
              }
              Status = gRT->GetVariable(
                                          L"SecureBootSetup",
                                          &guidSecurity,
                                          &Attributes,
                                          &SecDataSize,
                                          &SecData
                                          );
              if((!EFI_ERROR(Status))){
                  if (SecData.SecureBootSupport == 1){
                      Print(L"Operation Successfully!\n");
                  }else{
                      Print(L"Operation Failed!\n");
                  }
              }
              return 0;
     }

     if ((StrCmp(Argv[1], L"-enopt") == 0) || (StrCmp(Argv[1], L"-enosoptimized") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
              if ((!EFI_ERROR(Status))){
                  SetupData.OsOptimize = 1;
                            
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
                  if (SetupData.OsOptimize == 1){
                      Print(L"Operation Successfully!\n");
                  }else{
                      Print(L"Operation Failed!\n");
                  }
              }
              return 0;
     }

     if ((StrCmp(Argv[1], L"-disopt") == 0) || (StrCmp(Argv[1], L"-disosoptimized") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
              if ((!EFI_ERROR(Status))){
                  SetupData.OsOptimize = 0;
                            
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
                  if (SetupData.OsOptimize == 0){
                      Print(L"Operation Successfully!\n");
                  }else{
                      Print(L"Operation Failed!\n");
                  }
              }
              return 0;
     }

     if ((StrCmp(Argv[1], L"-pf") == 0) || (StrCmp(Argv[1], L"-poweroff") == 0)) {
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
              return 0;
     }
     
     //
     //arg "po" means poweron
     //
     if ((StrCmp(Argv[1], L"-po") == 0) || (StrCmp(Argv[1], L"-poweron") == 0)) {
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
              return 0;
     }
     //
     //arg "l" means laststate
     //
     if ((StrCmp(Argv[1], L"-ls") == 0) || (StrCmp(Argv[1], L"-laststate") == 0)) {
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
         return 0;
     }

     if ((StrCmp(Argv[1], L"-endg") == 0) || (StrCmp(Argv[1], L"-endevguard") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.LEMDeviceGuard = 1;
                            
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
             if (SetupData.LEMDeviceGuard == 1){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return 0;
     }

     if ((StrCmp(Argv[1], L"-disdg") == 0) || (StrCmp(Argv[1], L"-disdevguard") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.LEMDeviceGuard = 0;
                            
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
             if (SetupData.LEMDeviceGuard == 0){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return 0;
     }
    
    if ((StrCmp(Argv[1], L"-ahci") == 0) || (StrCmp(Argv[1], L"-AHCI") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.SataInterfaceMode = 0;
                            
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
             if (SetupData.SataInterfaceMode == 0){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return 0;
     }

     if ((StrCmp(Argv[1], L"-raid") == 0) || (StrCmp(Argv[1], L"-RAID") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.SataInterfaceMode = 2;
                            
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
             if (SetupData.SataInterfaceMode == 2){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return 0;
     }

      if ((StrCmp(Argv[1], L"-rst") == 0) || (StrCmp(Argv[1], L"-RST") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.SataInterfaceMode = 1;
                            
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
             if (SetupData.SataInterfaceMode == 1){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return 0;
     }

     if ((StrCmp(Argv[1], L"-enhttp") == 0) || (StrCmp(Argv[1], L"-EnHttpBoot") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.HttpsBoot = 1;
                            
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
             if (SetupData.HttpsBoot == 1){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return 0;
     }

     if ((StrCmp(Argv[1], L"-dishttp") == 0) || (StrCmp(Argv[1], L"-DisHttpBoot") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.HttpsBoot = 0;
                            
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
             if (SetupData.HttpsBoot == 0){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return 0;
     }

     if ((StrCmp(Argv[1], L"-disvtd") == 0) || (StrCmp(Argv[1], L"-DISVTD") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.EnableVtd = 0;
                            
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
             if (SetupData.EnableVtd == 0){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return 0;
     }

     if ((StrCmp(Argv[1], L"-envtd") == 0) || (StrCmp(Argv[1], L"-ENVTD") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.EnableVtd = 1;
                            
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
             if (SetupData.EnableVtd == 1){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return 0;
     }

     if ((StrCmp(Argv[1], L"-disms") == 0) || (StrCmp(Argv[1], L"-DISMS") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.LowPowerS0Idle = 0;
                            
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
             if (SetupData.LowPowerS0Idle == 0){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return 0;
     }

 if ((StrCmp(Argv[1], L"-enms") == 0) || (StrCmp(Argv[1], L"-ENMS") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.LowPowerS0Idle = 1;
                            
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
             if (SetupData.LowPowerS0Idle == 1){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return 0;
     }

     if ((StrCmp(Argv[1], L"-diswu") == 0) || (StrCmp(Argv[1], L"-DISWU") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.WindowsUEFIFirmwareUpdate = 0;
                            
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
             if (SetupData.WindowsUEFIFirmwareUpdate == 0){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return 0;
     }

 if ((StrCmp(Argv[1], L"-enwu") == 0) || (StrCmp(Argv[1], L"-ENWU") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.WindowsUEFIFirmwareUpdate = 1;
                            
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
             if (SetupData.WindowsUEFIFirmwareUpdate == 1){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return 0;
     }

 if ((StrCmp(Argv[1], L"-ftpm") == 0) || (StrCmp(Argv[1], L"-fTPM") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.TpmDeviceSelection = 1;
                            
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
             if (SetupData.TpmDeviceSelection == 1){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return 0;
     }

     if ((StrCmp(Argv[1], L"-dtpm") == 0) || (StrCmp(Argv[1], L"-dTPM") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.TpmDeviceSelection = 0;
                            
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
             if (SetupData.TpmDeviceSelection == 0){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return 0;
     }

     if ((StrCmp(Argv[1], L"-enserchip") == 0) || (StrCmp(Argv[1], L"-entpm") == 0)) {
         Status = gRT->GetVariable(
                                            L"Setup",
                                            &gSetupGuid,
                                            &Attributes,
                                            &DataSize,
                                            &SetupData
                                         );
                        
         if ((!EFI_ERROR(Status))){
             SetupData.TpmSupport = 1;
                            
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
             if (SetupData.TpmSupport == 1){
                 Print(L"Operation Successfully!\n");
             }else{
                 Print(L"Operation Failed!\n");
             }
         }
         return 0;
     }

    if ((StrCmp(Argv[1], L"-disserchip") == 0) || (StrCmp(Argv[1], L"-distpm") == 0)) {
             Status = gRT->GetVariable(
                                                L"Setup",
                                                &gSetupGuid,
                                                &Attributes,
                                                &DataSize,
                                                &SetupData
                                             );
                            
             if ((!EFI_ERROR(Status))){
                 SetupData.TpmSupport = 0;
                                
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
                 if (SetupData.TpmSupport == 0){
                     Print(L"Operation Successfully!\n");
                 }else{
                     Print(L"Operation Failed!\n");
                 }
             }
             return 0;
         }

    if ((StrCmp(Argv[1], L"-encpt") == 0) || (StrCmp(Argv[1], L"-ENCPT") == 0)) {
             Status = gRT->GetVariable(
                                                L"Setup",
                                                &gSetupGuid,
                                                &Attributes,
                                                &DataSize,
                                                &SetupData
                                             );
                            
             if ((!EFI_ERROR(Status))){
                 SetupData.ComputraceAction = 1;
                                
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
                 if (SetupData.ComputraceAction == 1){
                     Print(L"Operation Successfully!\n");
                 }else{
                     Print(L"Operation Failed!\n");
                 }
             }
             return 0;
         }

    if ((StrCmp(Argv[1], L"-discpt") == 0) || (StrCmp(Argv[1], L"-DISCPT") == 0)) {
             Status = gRT->GetVariable(
                                                L"Setup",
                                                &gSetupGuid,
                                                &Attributes,
                                                &DataSize,
                                                &SetupData
                                             );
                            
             if ((!EFI_ERROR(Status))){
                 SetupData.ComputraceAction = 0;
                                
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
                 if (SetupData.ComputraceAction == 0){
                     Print(L"Operation Successfully!\n");
                 }else{
                     Print(L"Operation Failed!\n");
                 }
             }
             return 0;
         }

    if ((StrCmp(Argv[1], L"-perdiscpt") == 0) || (StrCmp(Argv[1], L"-PERDISCPT") == 0)) {
             Status = gRT->GetVariable(
                                                L"Setup",
                                                &gSetupGuid,
                                                &Attributes,
                                                &DataSize,
                                                &SetupData
                                             );
                            
             if ((!EFI_ERROR(Status))){
                 SetupData.ComputraceAction = 2;
                                
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
                 if (SetupData.ComputraceAction == 2){
                     Print(L"Operation Successfully!\n");
                 }else{
                     Print(L"Operation Failed!\n");
                 }
             }
             return 0;
         }
    
    if ((StrCmp(Argv[1], L"-encsm") == 0) || (StrCmp(Argv[1], L"-ENCSM") == 0)) {
             Status = gRT->GetVariable(
                                                L"Setup",
                                                &gSetupGuid,
                                                &Attributes,
                                                &DataSize,
                                                &SetupData
                                             );
                            
             if ((!EFI_ERROR(Status))){
                 SetupData.CsmSupport = 1;
                                
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
                 if (SetupData.CsmSupport == 1){
                     Print(L"Operation Successfully!\n");
                 }else{
                     Print(L"Operation Failed!\n");
                 }
             }
             return 0;
         }

    if ((StrCmp(Argv[1], L"-discsm") == 0) || (StrCmp(Argv[1], L"-DISCSM") == 0)) {
             Status = gRT->GetVariable(
                                                L"Setup",
                                                &gSetupGuid,
                                                &Attributes,
                                                &DataSize,
                                                &SetupData
                                             );
                            
             if ((!EFI_ERROR(Status))){
                 SetupData.CsmSupport = 0;
                                
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
                 if (SetupData.CsmSupport == 0){
                     Print(L"Operation Successfully!\n");
                 }else{
                     Print(L"Operation Failed!\n");
                 }
             }
             return 0;
         }
    if ((StrCmp(Argv[1], L"-blink") == 0)) {
            EnableLCD();
            return 0;
         }

    if ((StrCmp(Argv[1], L"-noblink") == 0)) {
            DisableLCD();
            return 0;
         }

    if ((StrCmp(Argv[1], L"-csb") == 0)) {
                  Status = gRT->GetVariable(
                                              L"SecureBootSetup",
                                              &guidSecurity,
                                              &Attributes,
                                              &SecDataSize,
                                              &SecData
                                              );
                  if((!EFI_ERROR(Status))){
                      if (SecData.SecureBootSupport == 1){
                          Print(L"Secure Boot is Enabled!\n");
                      }else{
                          Print(L"Secure Boot is Disabled!\n");
                      }
                  }
                  return 0;
        
     }
     Print(L"Error Parameters input, please see usage below!\n", Status);
     PrintUsage();
     return Status;
     
}
