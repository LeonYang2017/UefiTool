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

/**
    Check if it's IPV4 device path
    @param  *Size                   - The size of mac address
    @retval UINT8 *                 - The returned ptr of mac address
**/

UINT8
IsEfiNetWorkType (
  IN  EFI_DEVICE_PATH_PROTOCOL   *Node
  )
{
  for (; !IsDevicePathEndType (Node); Node = NextDevicePathNode (Node)) {
    if (DevicePathType (Node) == MESSAGING_DEVICE_PATH) {
      if (DevicePathSubType (Node) == MSG_IPv4_DP) { // Only clac IPV4 address
        return TRUE;
      }
    }
  }
  return FALSE;
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
      EFI_STATUS                            Status;
      BOOLEAN                               IsEfiNetWork;
      EFI_DEVICE_PATH_PROTOCOL              *DevicePath;
      EFI_HANDLE                            *Handles;
      UINTN                                 HandleCount;
      UINTN                                 Index;
      UINT8                                 *MacData;
      MAC_ADDR_DEVICE_PATH                  *Mac;
      UINTN                                 TempSize;
      UINTN                                 SumSize;
      UINT8                                 count = 0;
      EFI_GUID                              gEfiLoadFileProtocolGuid = { 0x56EC3091, 0x954C, 0x11D2, { 0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }};
      UINT8                                 tpmmac1[6];
      UINT8                                 tpmmac2[6];
      UINT8                                 tpmmac3[6];
      UINT8                                 repetcount = 0;
      UINT8                                 repetcount1 = 0;
      UINT8                                 repetcount2 = 0;
      UINT8                                 repetcount3 = 0;
      UINT8                                 i;
      UINT8                                 j;
      
      //
      // Init
      //
      MacData  = NULL;
      Mac      = NULL;
      SumSize  = 0;
      TempSize = 0;

      //
      // Parse load file, assuming UEFI Network drivers
      //
      Print(L"MacCheck Version: 3.0.0\n");
      SystemTable->BootServices->LocateHandleBuffer (
           ByProtocol,
           &gEfiLoadFileProtocolGuid,
           NULL,
           &HandleCount,
           &Handles
           );
      for (Index = 0; Index < HandleCount; Index++) {
        Status = SystemTable->BootServices->HandleProtocol(
                        Handles[Index],
                        &gEfiDevicePathProtocolGuid,
                        &DevicePath
                        );
        if (!EFI_ERROR (Status)) {
          IsEfiNetWork = IsEfiNetWorkType (DevicePath);

          while (!IsDevicePathEnd (DevicePath) &&
               ((DevicePathType (DevicePath) != MESSAGING_DEVICE_PATH) ||
                (DevicePathSubType (DevicePath) != MSG_MAC_ADDR_DP))
               ) {
            DevicePath = NextDevicePathNode (DevicePath);
          }

          if (IsDevicePathEnd (DevicePath)) {
            Print(L"DevicePathEnd is FALSE!\n");  
            Print(L"\nERROR Code:0x2");
            Print(L"\n         ");
            return 2;
          }

          Mac = (MAC_ADDR_DEVICE_PATH *) DevicePath;
          if (!IsEfiNetWork) {
            continue;
          }
          TempSize = SumSize;
          if (MacData == NULL) {
            MacData = AllocateZeroPool (6); //Get 6 bytes of mac address
            SumSize += 6;
          } else {
            MacData = ReallocatePool (TempSize, SumSize, MacData);
            SumSize += 6;
          }
          CopyMem (MacData + TempSize, Mac->MacAddress.Addr, 6);
          //*Size = SumSize;
          Print (L"Mac DATA %02x-%02x-%02x-%02x-%02x-%02x\n", 
               Mac->MacAddress.Addr[0], Mac->MacAddress.Addr[1], Mac->MacAddress.Addr[2],
                Mac->MacAddress.Addr[3], Mac->MacAddress.Addr[4], Mac->MacAddress.Addr[5]);
          count++;
        }
      }
      if (SumSize == 0) {
        Print(L"No Network Device Detect!\n");  
        Print(L"\nERROR Code:0x3");
        Print(L"\n         ");
        return 3;
      }
      if (count == 1){
          Print(L"Only One Network Device Detect!\n");  
          Print(L"\nERROR Code:0x3");
          Print(L"\n         ");
          return 3;
      }
      
      if (count == 2){
          Print(L"Two Network Devices Detect!\n");
          repetcount = 0;
          for(i=0;i<6;i++){
              tpmmac1[i] = MacData[i];
          }
          
          j=0;
          for(i=6;i<12;i++){
                  tpmmac2[j] = MacData[i];
                  j++;
          }
          for(i=0;i<6;i++){
              if (tpmmac1[i]==tpmmac2[i]){
                  repetcount++;
              }
          }
          if (repetcount == 6){
              Print(L"Two MAC Address is the same!\n");
              Print(L"\nERROR Code:0x1");
              Print(L"\n         ");
              return 1;
          }
          Print(L"Two MAC Address is not the same!\n");
          Print(L"\nPASS!Return Code:0x0");
          Print(L"\n         ");
          return 0;
      }
      
      if (count == 3){
                Print(L"Three Network Devices Detect!\n");
                repetcount1 = 0;
                repetcount2 = 0;
                repetcount3 = 0;
                for(i=0;i<6;i++){
                    tpmmac1[i] = MacData[i];
                }

                j=0;
                for(i=6;i<12;i++){
                 
                        tpmmac2[j] = MacData[i];
                        j++;
                }

                j=0;
                for(i=12;i<18;i++){
                    
                        tpmmac3[j] = MacData[i];
                        j++;
                }
                
                for(i=0;i<6;i++){
                    if (tpmmac1[i]==tpmmac2[i]){
                        repetcount1++;
                    }
                }
                
                for(i=0;i<6;i++){
                    if (tpmmac1[i]==tpmmac3[i]){
                        repetcount2++;
                    }
                }
                
                for(i=0;i<6;i++){
                    if (tpmmac2[i]==tpmmac3[i]){
                        repetcount3++;
                        }
                    }
                
                if (repetcount1 == 6 || repetcount2 == 6 || repetcount3 == 6){
                    Print(L"Two MAC Address is the same!\n");
                    Print(L"\nERROR Code:0x1");
                    Print(L"\n         ");
                    return 1;
                }
                
                Print(L"Three MAC Address is not the same!\n");
                Print(L"PASS!Return Code:0x0");
                Print(L"\n         ");
                return 0;
            }
      
      Print(L"\nTOOL ERROR!Return Code:0x2\n");
      Print(L"\n         ");
      return 2;
    
    /*
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
    */
}

