/* *****************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * *****************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <map>
#include <cstdlib>
#include <pthread.h>
#include <mutex>
#include <condition_variable>

#include "logger.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "OCPlatform.h"
#include "OCApi.h"
#include "OCProvisioningManager.h"
#include "oxmjustworks.h"
#include "oxmrandompin.h"

#define MAX_URI_LENGTH (64)
#define MAX_PERMISSION_LENGTH (5)
#define CREATE (1)
#define READ (2)
#define UPDATE (4)
#define DELETE (8)
#define NOTIFY (16)
#define DASH '-'
#define PREDEFINED_TIMEOUT (10)
#define MAX_OWNED_DEVICE (10)
#define TAG  "provisioningclient"

#define JSON_DB_PATH "./oic_svr_db_client.json"
#define DEV_STATUS_ON "DEV_STATUS_ON"
#define DEV_STATUS_OFF "DEV_STATUS_OFF"

#define DISCOVERY_TIMEOUT 5

using namespace OC;

DeviceList_t pUnownedDevList, pOwnedDevList;
static int transferDevIdx, ask = 1;

static FILE* client_open(const char *UNUSED_PARAM, const char *mode)
{
    (void)UNUSED_PARAM;
    return fopen(JSON_DB_PATH, mode);
}

void printMenu()
{
    std::cout << "\nChoose an option:"<<std::endl;
    std::cout << "   1. UnOwned Device discovery"<<std::endl;
    std::cout << "   2. Owned Device discovery"<<std::endl;
    std::cout << "   3. Ownership transfer"<<std::endl;
    std::cout << "   4. Provision ACL"<<std::endl;
    std::cout << "   5. Provision Credentials"<<std::endl;
    std::cout << "   6. Credential & ACL provisioning b/w two devices"<<std::endl;
    std::cout << "   7. Unlink Devices"<<std::endl;
    std::cout << "   8. Remove Device"<<std::endl;
    std::cout << "   9. Get Linked Devices"<<std::endl;
    std::cout << "   10. Get Device Status"<<std::endl;
    std::cout << "   11. Exit loop"<<std::endl;
}

void moveTransferredDevice()
{
    pOwnedDevList.push_back(pUnownedDevList[transferDevIdx]);
    pUnownedDevList.erase(pUnownedDevList.begin() + transferDevIdx);
}

void InputPinCB(char* pinBuf, size_t bufSize)
{
    if(pinBuf)
    {
        std::cout <<"INPUT PIN : ";
        std::string ptr;
        std::cin >> ptr;
        OICStrcpy(pinBuf, bufSize, ptr.c_str());
        return;
    }
}

void printUuid(OicUuid_t uuid)
{
    for (int i = 0; i < UUID_LENGTH; i++)
    {
        std::cout <<std::hex << uuid.id[i] << " ";
    }
    std::cout<<std::endl;
}

void ownershipTransferCB(PMResultList_t *result, int hasError)
{
    if (hasError)
    {
        std::cout << "Error!!! in OwnershipTransfer"<<std::endl;
    }
    else
    {
        std::cout<< "\nTransferred Ownership successfuly for device : ";
        printUuid(result->at(0).deviceId);
        delete result;

        moveTransferredDevice();
    }
    ask = 1;
}

void printStatus(int status)
{
    static std::map<int, std::string> devStatus = {{1<<0, DEV_STATUS_ON}, {1<<1, DEV_STATUS_OFF}};

    std::cout <<devStatus[status] <<std::endl;
}

void printDevices(DeviceList_t &list)
{
   for (unsigned int i = 0; i < list.size(); i++ )
   {
      std::cout << "Device "<< i+1 <<" ID: ";
      std::cout << list[i]->getDeviceID() << " From IP: ";
      std::cout << list[i]->getDevAddr() << std::endl;
   }
}

/**
 * Callback function for provisioning ACL, Credentials.
 *
 * @param[in]    result Result list
 * @param[in] hasError indicates if the result has error
 */
void provisionCB(PMResultList_t *result, int hasError)
{
   if (hasError)
   {
       std::cout << "Error in provisioning operation!"<<std::endl;
   }
   else
   {
       std::cout<< "\nReceived provisioning results: ";
       for (unsigned int i = 0; i < result->size(); i++)
       {
           std::cout << "Result is = " << result->at(i).res <<" for device ";
           printUuid(result->at(i).deviceId);
       }

       delete result;
   }
   printMenu();
   ask = 1;
}
/**
 *
 * Ask user with which devices it wants to make further actions.
 * All possible error checks included.
 * Default behavior in case if only one options leaves are included too.
 * Expect that user count devices from 1, f.e. 1st, 2nd, 3rd, etc
 * Use DeviceList_t instead of devicesCount because of print devices info
 *
 * @param[in] list owned devices list.
 * @param[out] out device number array.
 * @param[in] count how many device numbers need to read.
 * @return 0 in case of success and other value otherwise.
 */
int readDeviceNumber(DeviceList_t &list, int count, int *out)
{
   if (out == NULL || count <= 0)
   {
      std::cout << "Error! Please put valid input parameters" << std::endl;
      return -1;
   }

   int devicesCount = list.size();

   //Print current list of owned devices
   std::cout <<"Owned devices, count = " << devicesCount << std::endl;
   printDevices(list);

   if (devicesCount < count)
   {
      std::cout << "You can't proceed with selected action because Owned devices count ( ";
      std::cout << devicesCount << " ) are less then required devices ( " << count << " ).";
      std::cout << "You may need to discover devices again" << std::endl;
      return -2;
   }

   std::cout << "Select " << count << " device(s) for provisioning" << std::endl;

   for (int curr = 0; curr < count; curr++)
   {
      //select last device by default if only 1 option exist
      //from user's point of view device counting starts from 1,
      //so 1st, 2nd, 3rd, etc devices
      if ((curr == count - 1) && (devicesCount == count))
      {
         int sum = 0;
         for (int i = 0; i < curr; i++)
         {
             sum += out[i];
         }

         out[curr] = (count*(count+1))/2 - sum;

         std::cout << "Device " << curr + 1 << " : " << out[curr];
         std::cout << " - selected automatically (because no other options exist)" << std::endl;
         break;
      }

      int choice;
      std::cout << "Device " << curr + 1 << " : ";
      std::cin >> choice;

      if (choice < 1 || choice > devicesCount)
      {
         std::cout << "Error! You should enter valid device number!" << std::endl;
         return -3;
      }

      //check that user doesn't select the same device twice
      for (int i = 0; i < curr; i++)
      {
         if (out[i] == choice)
         {
            std::cout << "Error! You cannot select the same device twice!" << std::endl;
            return -4;
         }
      }

      out[curr] = choice;
   }

   //Users count devices from 1, so 1st, 2nd, 3rd, etc device
   //But deviceList array start index is 0, so need to decrease all numbers to 1
   for (int i = 0; i < count; i++) out[i] -= 1;

   return 0;
}

/**
 * Perform cleanup for ACL
 * @param[in]    ACL
 */
static void deleteACL(OicSecAcl_t *acl)
{
    if (acl)
    {
        /* Clean Resources */
        for (unsigned int i = 0; i < (acl)->resourcesLen; i++)
        {
            OICFree((acl)->resources[i]);
        }
        OICFree((acl)->resources);

        /* Clean Owners */
        OICFree((acl)->owners);

        /* Clean ACL node itself */
        /* Required only if acl was created in heap */
        OICFree((acl));
    }
}

/**
 * Calculate ACL permission from string to bit
 *
 * @param[in] temp_psm    Input data of ACL permission string
 * @param[in,out] pms    The pointer of ACL permission value
 * @return  0 on success otherwise -1.
 */
static int CalculateAclPermission(const char *temp_pms, uint16_t *pms)
{
    int i = 0;

    if (NULL == temp_pms || NULL == pms)
    {
        return -1;
    }
    *pms = 0;
    while (temp_pms[i] != '\0')
    {
        switch (temp_pms[i])
        {
            case 'C':
                {
                    *pms += CREATE;
                    i++;
                    break;
                }
            case 'R':
                {
                    *pms += READ;
                    i++;
                    break;
                }
            case 'U':
                {
                    *pms += UPDATE;
                    i++;
                    break;
                }
            case 'D':
                {
                    *pms += DELETE;
                    i++;
                    break;
                }
            case 'N':
                {
                    *pms += NOTIFY;
                    i++;
                    break;
                }
            case '_':
                {
                    i++;
                    break;
                }
            default:
                {
                    return -1;
                }
        }
    }
    return 0;
}

/**
 * Get the ACL property from user
 *
 * @param[in]    ACL Datastructure to save user inputs
 * @return  0 on success otherwise -1.
 */
static int InputACL(OicSecAcl_t *acl)
{
    int ret;
    char *temp_id, *temp_rsc, *temp_pms;

    printf("******************************************************************************\n");
    printf("-Set ACL policy for target device\n");
    printf("******************************************************************************\n");
    //Set Subject.
    printf("-URN identifying the subject\n");
    printf("ex) 1111-1111-1111-1111 (16 Numbers except to '-')\n");
    printf("Subject : ");
    ret = scanf("%19ms", &temp_id);

    if (1 == ret)
    {
        for (int i = 0, j = 0; temp_id[i] != '\0'; i++)
        {
            if (DASH != temp_id[i])
                acl->subject.id[j++] = temp_id[i];
        }
        OICFree(temp_id);
    }
    else
    {
        printf("Error while input\n");
        return -1;
    }

    //Set Resource.
    printf("Num. of Resource : ");
    ret = scanf("%zu", &acl->resourcesLen);
    printf("-URI of resource\n");
    printf("ex)/oic/sh/temp/0 (Max_URI_Length: 64 Byte )\n");
    acl->resources = (char **)OICCalloc(acl->resourcesLen, sizeof(char *));
    if (NULL == acl->resources)
    {
        OC_LOG(ERROR, TAG, "Error while memory allocation");
        return -1;
    }
    for (size_t i = 0; i < acl->resourcesLen; i++)
    {
        printf("[%zu]Resource : ", i + 1);
        ret = scanf("%64ms", &temp_rsc);
        if (1 != ret)
        {
            printf("Error while input\n");
            return -1;
        }

        acl->resources[i] = OICStrdup(temp_rsc);
        OICFree(temp_rsc);
        if (NULL == acl->resources[i])
        {
            OC_LOG(ERROR, TAG, "Error while memory allocation");
            return -1;
        }
    }
    // Set Permission
    do
    {
        printf("-Set the permission(C,R,U,D,N)\n");
        printf("ex) CRUDN, CRU_N,..(5 Charaters)\n");
        printf("Permission : ");
        ret = scanf("%5ms", &temp_pms);
        if (1 != ret)
        {
            printf("Error while input\n");
            return -1;
        }
        ret = CalculateAclPermission(temp_pms, &(acl->permission));
        OICFree(temp_pms);
    } while (0 != ret );

    // Set Rowner
    printf("Num. of Rowner : ");
    ret = scanf("%zu", &acl->ownersLen);
    printf("-URN identifying the rowner\n");
    printf("ex) 1111-1111-1111-1111 (16 Numbers except to '-')\n");
    acl->owners = (OicUuid_t *)OICCalloc(acl->ownersLen, sizeof(OicUuid_t));
    if (NULL == acl->owners)
    {
        OC_LOG(ERROR, TAG, "Error while memory allocation");
        return -1;
    }
    for (size_t i = 0; i < acl->ownersLen; i++)
    {
        printf("[%zu]Rowner : ", i + 1);
        ret = scanf("%19ms", &temp_id);
        if (1 != ret)
        {
            printf("Error while input\n");
            return -1;
        }

        for (int k = 0, j = 0; temp_id[k] != '\0'; k++)
        {
            if (DASH != temp_id[k])
            {
                acl->owners[i].id[j++] = temp_id[k];
            }
        }
        OICFree(temp_id);
    }
    return 0;
}

static int InputCredentials(Credential &cred)
{
   int choice;

   do
   {
       std::cout << "Select credential type from following values:" << std::endl;
       std::cout << "1:  symmetric pair-wise key" << std::endl;
       std::cout << "2:  symmetric group key" << std::endl;
       std::cout << "4:  asymmetric key" << std::endl;
       std::cout << "8:  signed asymmetric key (aka certificate)" << std::endl;
       std::cout << "16: PIN /password" << std::endl;
       std::cout << "Your choice: ";

       std::cin >> choice;

       switch (choice){
           case 1:
               cred.setCredentialType(static_cast<OicSecCredType_t>(choice));
               choice = 0; //validation of the accepted choice.
               break;
           case 2:
           case 4:
           case 8:
           case 16:
               std::cout << "selected type is not supported yet" << std::endl;
               break;
           default:
               std::cout << "Error! Please select valid credential type" << std::endl;
               break;
       }
   } while(0 != choice);

   std::cout << "Please enter key size (valid size is 128 or 256) :";
   std::cin >> choice;

   if(128 == choice)
   {
       cred.setCredentialKeySize(OWNER_PSK_LENGTH_128);
   }
   else if(256 == choice)
   {
        cred.setCredentialKeySize(OWNER_PSK_LENGTH_256);
   }
   else
   {
      std::cout << "Error! Please enter valid key size!" << std::endl;
      return -2;
   }

   return 0;
}

int main(void)
{
    OCPersistentStorage ps {client_open, fread, fwrite, fclose, unlink };

    // Create PlatformConfig object
    PlatformConfig cfg {
        OC::ServiceType::InProc,
            OC::ModeType::Both,
            "0.0.0.0",
            0,
            OC::QualityOfService::LowQos,
            &ps
    };

    OCPlatform::Configure(cfg);

    try
    {
        int choice;
        OicSecAcl_t *acl1 = nullptr, *acl2 = nullptr;
        if (OCSecure::provisionInit("") != OC_STACK_OK)
        {
            std::cout <<"PM Init failed"<< std::endl;
            return 1;
        }

        for (int out = 0; !out;)
        {
            while (!ask)
            {
                sleep(1);
            }

            if (acl1)
            {
                deleteACL(acl1);
                acl1 = nullptr;
            }

            if (acl2)
            {
                deleteACL(acl2);
                acl2 = nullptr;
            }

            printMenu();
            std::cin >> choice;
            switch(choice) {
                case 1:
                    {
                        //Secure resource discovery.

                        pUnownedDevList.clear();
                        std::cout << "Started discovery..." <<std::endl;
                        OCStackResult result = OCSecure::discoverUnownedDevices(DISCOVERY_TIMEOUT,
                                pUnownedDevList);
                        if (result != OC_STACK_OK)
                        {
                            std::cout<< "!!Error - UnOwned Discovery failed."<<std::endl;
                        }
                        else if (pUnownedDevList.size())
                        {
                            std::cout <<"Found secure devices, count = " <<
                                pUnownedDevList.size() << std::endl;
                            printDevices(pUnownedDevList);
                        }
                        else
                        {
                            std::cout <<"No Secure devices found"<<std::endl;
                        }
                        break;
                    }
                case 2:
                    {
                        pOwnedDevList.clear();
                        std::cout << "Started discovery..." <<std::endl;
                        OCStackResult result = OCSecure::discoverOwnedDevices(DISCOVERY_TIMEOUT,
                                pOwnedDevList);
                        if (result != OC_STACK_OK)
                        {
                            std::cout<< "!!Error - Owned Discovery failed."<<std::endl;
                        }
                        else if (pOwnedDevList.size())
                        {
                            std::cout <<"Found owned devices, count = " <<
                                pOwnedDevList.size() << std::endl;
                            printDevices(pOwnedDevList);
                        }
                        else
                        {
                            std::cout <<"No Secure devices found"<<std::endl;
                        }
                        break;
                    }
                case 3:
                    {
                        unsigned int devNum;

                        if (!pUnownedDevList.size())
                        {
                            std::cout <<"There are no more Unowned devices"<<std::endl;
                            break;
                        }

                        for (unsigned int i = 0; i < pUnownedDevList.size(); i++ )
                        {
                            std::cout << i+1 << ": "<< pUnownedDevList[i]->getDeviceID();
                            std::cout << " From IP:" << pUnownedDevList[i]->getDevAddr() <<std::endl;
                        }

                        std::cout <<"Select device number: "<<std::endl;
                        std::cin >> devNum;
                        if (devNum > pUnownedDevList.size())
                        {
                            std::cout <<"Invalid device number"<<std::endl;
                            break;
                        }
                        transferDevIdx = devNum - 1;

                        //register callbacks for JUST WORKS and PIN methods
                        std::cout <<"Registering OTM Methods: 1. JUST WORKS and 2. PIN"<<std::endl;

                        {
                            OTMCallbackData_t justWorksCBData;
                            justWorksCBData.loadSecretCB = LoadSecretJustWorksCallback;
                            justWorksCBData.createSecureSessionCB =
                                CreateSecureSessionJustWorksCallback;
                            justWorksCBData.createSelectOxmPayloadCB =
                                CreateJustWorksSelectOxmPayload;
                            justWorksCBData.createOwnerTransferPayloadCB =
                                CreateJustWorksOwnerTransferPayload;
                            OCSecure::setOwnerTransferCallbackData(OIC_JUST_WORKS,
                                    &justWorksCBData, NULL);
                        }

                        {
                            OTMCallbackData_t pinBasedCBData;
                            pinBasedCBData.loadSecretCB = InputPinCodeCallback;
                            pinBasedCBData.createSecureSessionCB =
                                CreateSecureSessionRandomPinCallbak;
                            pinBasedCBData.createSelectOxmPayloadCB =
                                CreatePinBasedSelectOxmPayload;
                            pinBasedCBData.createOwnerTransferPayloadCB =
                                CreatePinBasedOwnerTransferPayload;
                            OCSecure::setOwnerTransferCallbackData(OIC_RANDOM_DEVICE_PIN,
                                    &pinBasedCBData, InputPinCB);
                        }

                        ask = 0;
                        std::cout << "Transfering ownership for : "<<
                            pUnownedDevList[devNum-1]->getDeviceID()<<std::endl;
                        if (pUnownedDevList[devNum-1]->doOwnershipTransfer(ownershipTransferCB)
                                != OC_STACK_OK)
                        {
                            std::cout<<"OwnershipTransferCallback is failed"<<std::endl;
                            ask = 1;
                        }
                        break;
                    }
                case 4: //Provision ACL
                    {
                        int index;

                        if (0 != readDeviceNumber(pOwnedDevList, 1, &index)) break;

                        std::cout << "Provision ACL for : "<<
                            pOwnedDevList[index]->getDeviceID()<< std::endl;

                        acl1 = (OicSecAcl_t *)OICCalloc(1,sizeof(OicSecAcl_t));
                        if (NULL == acl1)
                        {
                            OC_LOG(ERROR, TAG, "Error while memory allocation");
                            break;
                        }

                        std::cout << "Please input ACL for selected device: " << std::endl;
                        if (0 != InputACL(acl1))
                        {
                            break;
                        }

                        ask = 0;

                        if (pOwnedDevList[index]->provisionACL(acl1, provisionCB) != OC_STACK_OK)
                        {
                            ask = 1;
                            std::cout <<"provisionACL is failed"<< std::endl;
                        }
                    }
                    break;
                case 5: //Provision Credentials
                    {
                        int devices[2];

                        if (0 != readDeviceNumber(pOwnedDevList, 2, devices)) break;

                        int first  = devices[0];
                        int second = devices[1];

                        std::cout << "Provision Credentials to devices: "<<
                            pOwnedDevList[first]->getDeviceID();
                        std::cout << " and "<< pOwnedDevList[second]->getDeviceID() << std::endl;

                        Credential cred( NO_SECURITY_MODE ,0);
                        std::cout << "Please input credentials for selected devices: " << std::endl;
                        if (0 != InputCredentials(cred))
                            break;

                        ask = 0;

                        if (pOwnedDevList[first]->provisionCredentials(cred,
                                    *pOwnedDevList[second].get(), provisionCB) != OC_STACK_OK)
                        {
                            ask = 1;
                            std::cout <<"provisionCredentials is failed"<< std::endl;
                        }
                    }
                    break;
                case 6: //Provision ACL & Creds b/w two devices.
                    {
                        int devices[2];

                        if (0 != readDeviceNumber(pOwnedDevList, 2, devices)) break;

                        int first  = devices[0];
                        int second = devices[1];

                        std::cout << "Provision pairwise devices: "<<
                            pOwnedDevList[first]->getDeviceID();
                        std::cout << " and "<< pOwnedDevList[second]->getDeviceID() << std::endl;

                        Credential cred( NO_SECURITY_MODE, 0);
                        std::cout << "Please input credentials for selected devices: " << std::endl;
                        if (0 != InputCredentials(cred))
                            break;

                        acl1 = (OicSecAcl_t *)OICCalloc(1,sizeof(OicSecAcl_t));
                        if (NULL == acl1)
                        {
                            OC_LOG(ERROR, TAG, "Error while memory allocation");
                            break;
                        }

                        std::cout << "Please input ACL for selected device: " << std::endl;
                        if (0 != InputACL(acl1))
                        {
                            break;
                        }

                        acl2 = (OicSecAcl_t *)OICCalloc(1,sizeof(OicSecAcl_t));
                        if (NULL == acl2)
                        {
                            OC_LOG(ERROR, TAG, "Error while memory allocation");
                            break;
                        }

                        std::cout << "Please input ACL for selected device: " << std::endl;
                        if (0 != InputACL(acl2))
                        {
                            break;
                        }

                        ask = 0;

                        if (pOwnedDevList[first]->provisionPairwiseDevices(cred, acl1,
                                    *pOwnedDevList[second].get(), acl2, provisionCB) != OC_STACK_OK)
                        {
                            ask = 1;
                            std::cout <<"provisionPairwiseDevices is failed"<< std::endl;
                        }
                    }
                    break;
                case 7: //Unlink Devices
                    {
                        int devices[2];

                        if (0 != readDeviceNumber(pOwnedDevList, 2, devices)) break;

                        int first  = devices[0];
                        int second = devices[1];

                        std::cout << "Unlink devices: "<< pOwnedDevList[first]->getDeviceID();
                        std::cout << " and "<< pOwnedDevList[second]->getDeviceID() << std::endl;

                        ask = 0;

                        if (pOwnedDevList[first]->unlinkDevices(*pOwnedDevList[second].get(),
                                    provisionCB) != OC_STACK_OK)
                        {
                            ask = 1;
                            std::cout <<"unlinkDevice is failed"<< std::endl;
                        }
                        break;
                    }
                case 8: //Remove Device
                    {
                        int index;

                        if (0 != readDeviceNumber(pOwnedDevList, 1, &index)) break;

                        std::cout << "Remove Device: "<< pOwnedDevList[index]->getDeviceID()<< std::endl;

                        ask = 0;

                        if (pOwnedDevList[index]->removeDevice(DISCOVERY_TIMEOUT, provisionCB)
                                != OC_STACK_OK)
                        {
                            ask = 1;
                            std::cout <<"removeDevice is failed"<< std::endl;
                        }
                        break;
                    }
                case 9: //Get Linked devices
                    {
                        UuidList_t linkedUuid;
                        unsigned int devNum;

                        if (!pOwnedDevList.size())
                        {
                            std::cout <<"There are no Owned devices yet,"
                                " may need to discover"<<std::endl;
                            break;
                        }

                        for (unsigned int i = 0; i < pOwnedDevList.size(); i++ )
                        {
                            std::cout << i+1 << ": "<< pOwnedDevList[i]->getDeviceID() <<" From IP:";
                            std::cout << pOwnedDevList[i]->getDevAddr() <<std::endl;
                        }

                        std::cout <<"Select device number: "<<std::endl;
                        std::cin >> devNum;
                        if (devNum > pOwnedDevList.size())
                        {
                            std::cout <<"Invalid device number"<<std::endl;
                            break;
                        }

                        if(pOwnedDevList[devNum  -1]->getLinkedDevices(linkedUuid) == OC_STACK_OK)
                        {
                            if (!linkedUuid.size())
                            {
                                std::cout <<"No devices are linked to "<<
                                    pOwnedDevList[devNum  -1]->getDeviceID() << std::endl;
                            }
                            //display the Linked devices (UUIDs)
                            for(unsigned int i = 0; i < linkedUuid.size(); i++)
                            {
                                printUuid(linkedUuid[i]);
                            }
                        }
                        else
                        {
                            std::cout <<"Error! in getLinkedDevices"<<std::endl;
                        }
                        break;
                    }
                case 10: //Get device' status
                    {
                        DeviceList_t unownedList, ownedList;

                        if (OCSecure::getDevInfoFromNetwork(DISCOVERY_TIMEOUT, ownedList,
                                    unownedList) == OC_STACK_OK)
                        {
                            std::cout <<"Owned Device' status for" <<std::endl;
                            for (unsigned int i = 0; i < ownedList.size(); i++ )
                            {
                                std::cout << "Device "<<i+1 <<" ID: '";
                                std::cout << ownedList[i]->getDeviceID() << "' From IP: ";
                                std::cout << ownedList[i]->getDevAddr() << " Status: ";
                                printStatus(ownedList[i]->getDeviceStatus());
                            }
                            std::cout <<"\nUnOwned Device' status for" <<std::endl;
                            for (unsigned int i = 0; i < unownedList.size(); i++ )
                            {
                                std::cout << "Device "<<i+1 <<" ID: '";
                                std::cout << unownedList[i]->getDeviceID() << "' From IP: ";
                                std::cout << unownedList[i]->getDevAddr() << " Status: ";
                                printStatus(unownedList[i]->getDeviceStatus());
                            }

                        }
                        break;
                    }

                case 11:
                default:
                    out = 1;
                    break;
            }
        }
    }
    catch(OCException& e)
    {
        oclog() << "Exception in main: "<<e.what();
    }

    return 0;
}
