/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include "simulator_manager.h"
#include <map>
#include <mutex>

std::string getOperationStateString(OperationState state)
{
    switch (state)
    {
        case OP_START: return "OP_START";
        case OP_COMPLETE: return "OP_COMPLETE";
        case OP_ABORT: return "OP_ABORT";
    }

    return "OP_UNKNOWN";
}

class AppLogger : public ILogger
{
    public:
        void write(std::string time, ILogger::Level level, std::string message)
        {
            std::cout << "[APPLogger] " << time << " " << ILogger::getString(level) << " " << message;
        }
};
std::shared_ptr<AppLogger> gAppLogger(new AppLogger());

class ClientController
{
    public:
        void startTest()
        {
            printMenu();
            bool cont = true;
            while (cont)
            {
                int choice = -1;
                std::cout << "Enter your choice: ";
                std::cin >> choice;
                if (choice < 0 || choice > 12)
                {
                    std::cout << "Invaild choice !" << std::endl; continue;
                }

                switch (choice)
                {
                    case 1: findResource(); break;
                    case 2: displayResource(); break;
                    case 3: observeResource(); break;
                    case 4: cancelObserving(); break;
                    case 5: sendGet(); break;
                    case 6: sendPut(); break;
                    case 7: sendPost(); break;
                    case 8: sendAllGETRequests(); break;
                    case 9: sendAllPUTRequests(); break;
                    case 10: sendAllPOSTRequests(); break;
                    case 11: configure(); break;
                    case 12: printMenu(); break;
                    case 0: cont = false;
                }
            }
        }

    private:
        void printMenu()
        {
            std::cout << "########### SIMULATOR CLIENT CONTROLLER ###########" << std::endl;
            std::cout << "1. Find resource" << std::endl;
            std::cout << "2. Display resource information" << std::endl;
            std::cout << "3. Observe for resource change" << std::endl;
            std::cout << "4. Cancel observation" << std::endl;
            std::cout << "5. Send GET message" << std::endl;
            std::cout << "6. Send PUT message" << std::endl;
            std::cout << "7. Send POST message" << std::endl;
            std::cout << "8. Send All GET requests" << std::endl;
            std::cout << "9. Send All PUT requests" << std::endl;
            std::cout << "10. Send All POST requests" << std::endl;
            std::cout << "11. Configure (using RAML file)" << std::endl;
            std::cout << "12: Help" << std::endl;
            std::cout << "0. Exit" << std::endl;
            std::cout << "###################################################" << std::endl;
        }

        SimulatorRemoteResourceSP selectResource()
        {
            std::lock_guard<std::recursive_mutex> lock(m_mutex);
            if (0 == m_resList.size())
            {
                std::cout << "No resouces!" << std::endl;
                return nullptr;
            }

            int index = 1;
            std::vector<std::string> ids;
            for (auto & resourceEntry : m_resList)
            {
                std::cout << index++ << ": " << (resourceEntry.second)->getURI() << "[" <<
                          (resourceEntry.second)->getHost()  << "]" << std::endl;
                ids.push_back((resourceEntry.second)->getID());
            }

            int choice = -1;
            std::cout << "Choose the resource: ";
            std::cin >> choice;

            if (choice < 1 || choice > index - 1)
            {
                std::cout << "Invalid choice !" << std::endl;
                return nullptr;
            }

            return m_resList[ids[choice - 1]];
        }

        void findResource()
        {
            std::string resourceType;
            std::cout << "Enter resource type : ";
            std::cin >> resourceType;

            ResourceFindCallback callback = [this](std::shared_ptr<SimulatorRemoteResource> resource)
            {
                std::cout << "Resource found ######" << std::endl;
                displayResource(resource);

                // Add to local list
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
                if (m_resList.end() == m_resList.find(resource->getID()))
                    m_resList[resource->getID()] = resource;
                else
                    std::cout << "Resource with UID: " << resource->getID() << "already exist in the list!" <<
                              std::endl;
            };

            try
            {
                SimulatorManager::getInstance()->findResource(resourceType, callback);
                std::cout << "SimulatorManager::findResource is successfull" << std::endl;
            }
            catch (InvalidArgsException &e)
            {
                std::cout << "InvalidArgsException occured [code : " << e.code() << " Detail: " << e.what() << "]"
                          << std::endl;
            }
            catch (SimulatorException &e)
            {
                std::cout << "SimulatorException occured [code : " << e.code() << " Detail: " << e.what() << "]" <<
                          std::endl;
            }
        }

        void displayResource()
        {
            displayResource(selectResource());
        }

        void displayResource(SimulatorRemoteResourceSP resource)
        {
            if (!resource) return;

            std::cout << "#############################" << std::endl;
            std::cout << "URI: " << resource->getURI().c_str() << std::endl;
            std::cout << "Host: " << resource->getHost().c_str() << std::endl;
            std::cout << "ID: " << resource->getID().c_str() << std::endl;
            std::cout << "Resource Types: ";
            for (auto & type : resource->getResourceTypes())
                std::cout << type << " ";
            std::cout << "\nInterface Types: ";
            for (auto & type : resource->getResourceInterfaces())
                std::cout << type << " ";
            std::cout << std::boolalpha << "\nisObservable : " << resource->isObservable()
                    << std::noboolalpha << std::endl;
            std::cout << "#############################" << std::endl;
        }

        void observeResource()
        {
            SimulatorRemoteResourceSP resource = selectResource();
            if (!resource) return;

            // callback implementaion
            SimulatorRemoteResource::ObserveNotificationCallback callback =
                [](std::string uid, SimulatorResult errorCode, SimulatorResourceModelSP rep, int seq)
            {
                std::cout << "\nObserve notification received ###[errorcode:  " << errorCode <<
                          " seq:  " << seq << "UID: " << uid << "]" << std::endl;
                std::map<std::string, SimulatorResourceModel::Attribute> attributes = rep->getAttributes();
                for (auto & attribute : attributes)
                {
                    std::cout << (attribute.second).getName() << " :  {" << std::endl;
                    std::cout << "value: " << (attribute.second).valueToString().c_str() << std::endl;
                    std::cout << "}" << std::endl;
                }
                std::cout << std::endl;
            };

            try
            {
                resource->observe(ObserveType::OBSERVE, callback);
                std::cout << "Observe is successfull!" << std::endl;
            }
            catch (InvalidArgsException &e)
            {
                std::cout << "InvalidArgsException occured [code : " << e.code() << " Detail: "
                        << e.what() << "]" << std::endl;
            }
            catch (SimulatorException &e)
            {
                std::cout << "SimulatorException occured [code : " << e.code() << " Detail: " <<
                        e.what() << "]" << std::endl;
            }
        }

        void cancelObserving()
        {
            SimulatorRemoteResourceSP resource = selectResource();
            if (!resource) return;

            try
            {
                resource->cancelObserve();
                std::cout << "Cancelling observe is successfull!" << std::endl;
            }
            catch (SimulatorException &e)
            {
                std::cout << "SimulatorException occured [code : " << e.code() << " Detail: " <<
                        e.what() << "]" << std::endl;
            }
        }

        void sendGet()
        {
            SimulatorRemoteResourceSP resource = selectResource();
            if (!resource) return;

            // callback implementaion
            SimulatorRemoteResource::ResponseCallback callback =
                [](std::string uId, SimulatorResult errorCode, SimulatorResourceModelSP rep)
            {
                std::cout << "\nGET Response received ### [errorcode:  " << errorCode << "]"
                        << std::endl;
                std::cout << "UID is: " << uId << std::endl;
                std::cout << "Representation is: " << std::endl;
                std::map<std::string, SimulatorResourceModel::Attribute> attributes =
                        rep->getAttributes();
                for (auto & attribute : attributes)
                {
                    std::cout << (attribute.second).getName() << " :  {" << std::endl;
                    std::cout << "value: " << (attribute.second).valueToString().c_str()
                            << std::endl;
                    std::cout << "}" << std::endl;
                }
                std::cout << std::endl;
            };

            try
            {
                resource->get(std::map <std::string, std::string>(), callback);
                std::cout << "GET is successfull!" << std::endl;
            }
            catch (InvalidArgsException &e)
            {
                std::cout << "InvalidArgsException occured [code : " << e.code() << " Detail: "
                        << e.what() << "]" << std::endl;
            }
            catch (NoSupportException &e)
            {
                std::cout << "NoSupportException occured [code : " << e.code() << " Detail: " <<
                        e.what() << "]" << std::endl;
            }
            catch (SimulatorException &e)
            {
                std::cout << "SimulatorException occured [code : " << e.code() << " Detail: " <<
                        e.what() << "]" << std::endl;
            }
        }

        void sendPut()
        {
            SimulatorRemoteResourceSP resource = selectResource();
            if (!resource) return;

            // callback implementaion
            SimulatorRemoteResource::ResponseCallback callback =
                [](std::string uId, SimulatorResult errorCode, SimulatorResourceModelSP rep)
            {
                std::cout << "\nPUT Response received ![errorcode:  " << errorCode << "]"
                        << std::endl;
                std::cout << "UID is: " << uId << std::endl;
                std::cout << "Representation is: " << std::endl;
                std::map<std::string, SimulatorResourceModel::Attribute> attributes =
                        rep->getAttributes();
                for (auto & attribute : attributes)
                {
                    std::cout << (attribute.second).getName() << " :  {" << std::endl;
                    std::cout << "value: " << (attribute.second).valueToString().c_str()
                            << std::endl;
                    std::cout << "}" << std::endl;
                }
                std::cout << std::endl;
            };

            try
            {
                SimulatorResourceModelSP rep = std::make_shared<SimulatorResourceModel>();
                std::string value = "off";
                rep->addAttribute("power", value);
                rep->addAttribute("intensity", 5);

                resource->put(std::map <std::string, std::string>(), rep, callback);
                std::cout << "PUT is successfull!" << std::endl;
            }
            catch (InvalidArgsException &e)
            {
                std::cout << "InvalidArgsException occured [code : " << e.code() << " Detail: "
                        << e.what() << "]" << std::endl;
            }
            catch (NoSupportException &e)
            {
                std::cout << "NoSupportException occured [code : " << e.code() << " Detail: " <<
                        e.what() << "]" << std::endl;
            }
            catch (SimulatorException &e)
            {
                std::cout << "SimulatorException occured [code : " << e.code() << " Detail: " <<
                        e.what() << "]" << std::endl;
            }
        }

        void sendPost()
        {
            SimulatorRemoteResourceSP resource = selectResource();
            if (!resource) return;

            // callback implementaion
            SimulatorRemoteResource::ResponseCallback callback =
                [](std::string uId, SimulatorResult errorCode, SimulatorResourceModelSP rep)
            {
                std::cout << "\nPOST Response received ![errorcode:  " << errorCode << "]"
                        << std::endl;
                std::cout << "UID is: " << uId << std::endl;
                std::cout << "Representation is: " << std::endl;
                std::map<std::string, SimulatorResourceModel::Attribute> attributes =
                        rep->getAttributes();
                for (auto & attribute : attributes)
                {
                    std::cout << (attribute.second).getName() << " :  {" << std::endl;
                    std::cout << "value: " << (attribute.second).valueToString().c_str()
                            << std::endl;
                    std::cout << "}" << std::endl;
                }
                std::cout << std::endl;
            };

            try
            {
                SimulatorResourceModelSP rep = std::make_shared<SimulatorResourceModel>();
                std::string value = "on";
                rep->addAttribute("power", value);
                rep->addAttribute("intensity", 7);

                resource->post(std::map <std::string, std::string>(), rep, callback);
                std::cout << "POST is successfull!" << std::endl;
            }
            catch (InvalidArgsException &e)
            {
                std::cout << "InvalidArgsException occured [code : " << e.code() << " Detail: "
                        << e.what() << "]" << std::endl;
            }
            catch (NoSupportException &e)
            {
                std::cout << "NoSupportException occured [code : " << e.code() << " Detail: " <<
                        e.what() << "]" << std::endl;
            }
            catch (SimulatorException &e)
            {
                std::cout << "SimulatorException occured [code : " << e.code() << " Detail: " <<
                        e.what() << "]" << std::endl;
            }
        }

        void sendAllGETRequests()
        {
            SimulatorRemoteResourceSP resource = selectResource();
            if (!resource) return;

            SimulatorRemoteResource::StateCallback callback = [] (std::string uid, int sessionId,
                    OperationState state)
            {
                std::cout << "\nResource verification status received ![id:  " << sessionId <<
                        "  State: " << getOperationStateString(state) << " UID: " << uid << "]" <<
                        std::endl;
            };

            try
            {
                int id = resource->startVerification(RequestType::RQ_TYPE_GET, callback);
                std::cout << "startVerification for GET is successfull!id: " << id << std::endl;
            }
            catch (InvalidArgsException &e)
            {
                std::cout << "InvalidArgsException occured [code : " << e.code() << " Detail: "
                        << e.what() << "]" << std::endl;
            }
            catch (NoSupportException &e)
            {
                std::cout << "NoSupportException occured [code : " << e.code() << " Detail: " <<
                        e.what() << "]" << std::endl;
            }
            catch (SimulatorException &e)
            {
                std::cout << "SimulatorException occured [code : " << e.code() << " Detail: " <<
                        e.what() << "]" << std::endl;
            }
        }

        void sendAllPUTRequests()
        {
            SimulatorRemoteResourceSP resource = selectResource();
            if (!resource) return;

            SimulatorRemoteResource::StateCallback callback = [] (std::string uid, int sessionId,
                    OperationState state)
            {
                std::cout << "\nResource verification status received ![id:  " << sessionId <<
                        "  State: " << getOperationStateString(state) << " UID: " << uid << "]" <<
                        std::endl;
            };

            try
            {
                int id = resource->startVerification(RequestType::RQ_TYPE_PUT, callback);
                std::cout << "startVerification for PUT is successfull!id: " << id << std::endl;
            }
            catch (InvalidArgsException &e)
            {
                std::cout << "InvalidArgsException occured [code : " << e.code() << " Detail: "
                        << e.what() << "]" << std::endl;
            }
            catch (NoSupportException &e)
            {
                std::cout << "NoSupportException occured [code : " << e.code() << " Detail: " <<
                        e.what() << "]" << std::endl;
            }
            catch (SimulatorException &e)
            {
                std::cout << "SimulatorException occured [code : " << e.code() << " Detail: " <<
                        e.what() << "]" << std::endl;
            }
        }

        void sendAllPOSTRequests()
        {
            SimulatorRemoteResourceSP resource = selectResource();
            if (!resource) return;

            SimulatorRemoteResource::StateCallback callback = [] (std::string uid, int sessionId,
                    OperationState state)
            {
                std::cout << "\nResource verification status received ![id:  " << sessionId <<
                        "  State: " << getOperationStateString(state) << " UID: " << uid << "]"
                        << std::endl;
            };

            try
            {
                int id = resource->startVerification(RequestType::RQ_TYPE_POST, callback);
                std::cout << "startVerification for POST is successfull!id: " << id << std::endl;
            }
            catch (InvalidArgsException &e)
            {
                std::cout << "InvalidArgsException occured [code : " << e.code() << " Detail: "
                        << e.what() << "]" << std::endl;
            }
            catch (NoSupportException &e)
            {
                std::cout << "NoSupportException occured [code : " << e.code() << " Detail: " <<
                        e.what() << "]" << std::endl;
            }
            catch (SimulatorException &e)
            {
                std::cout << "SimulatorException occured [code : " << e.code() << " Detail: " <<
                        e.what() << "]" << std::endl;
            }
        }

        void configure()
        {
            SimulatorRemoteResourceSP resource = selectResource();
            if (!resource)
                return;

            try
            {
                std::string configPath;
                std::cout << "Enter the config path: ";
                std::cin >> configPath;

                resource->configure(configPath);
                std::cout << "configuration is successfull!" << std::endl;
            }
            catch (InvalidArgsException &e)
            {
                std::cout << "InvalidArgsException occured [code : " << e.code() << " Detail: "
                        << e.what() << "]" << std::endl;
            }
            catch (SimulatorException &e)
            {
                std::cout << "SimulatorException occured [code : " << e.code() << " Detail: " <<
                        e.what() << "]" << std::endl;
            }
        }

    private:
        std::recursive_mutex m_mutex;
        std::map<std::string, SimulatorRemoteResourceSP> m_resList;
};

void printMainMenu()
{
    std::cout << "############### MAIN MENU###############" << std::endl;
    std::cout << "1. Client Controller Test" << std::endl;
    std::cout << "2. Get device information" << std::endl;
    std::cout << "3. Get platform information" << std::endl;
    std::cout << "4. Set Logger" << std::endl;
    std::cout << "5. Help" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "######################################" << std::endl;
}

void setLogger()
{
    std::cout << "1. Default console logger" << std::endl;
    std::cout << "2. Default file logger" << std::endl;
    std::cout << "3. custom logger" << std::endl;

    int choice = -1;
    std::cin >> choice;
    if (choice <= 0 || choice > 3)
    {
        std::cout << "Invalid selection !" << std::endl;
        return;
    }

    switch (choice)
    {
        case 1:
            {
                if (false == SimulatorManager::getInstance()->setConsoleLogger())
                    std::cout << "Failed to set the default console logger" << std::endl;
            }
            break;

        case 2:
            {
                std::string filePath;
                std::cout << "Enter the file path (without file name) : ";
                std::cin >> filePath;
                if (false == SimulatorManager::getInstance()->setFileLogger(filePath))
                    std::cout << "Failed to set default file logger" << std::endl;
            }
            break;

        case 3:
            SimulatorManager::getInstance()->setLogger(gAppLogger);
    }
}

int main(void)
{
    ClientController clientController;
    printMainMenu();
    bool cont = true;
    while (cont == true)
    {
        int choice = -1;
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        if (choice < 0 || choice > 5)
        {
            std::cout << "Invaild choice !" << std::endl; continue;
        }

        switch (choice)
        {
            case 1: clientController.startTest();
                std::cout << "Welcome back to main menu !" << std::endl;
                break;

            case 2:
                {
                    try
                    {
                        SimulatorManager::getInstance()->getDeviceInfo(std::bind([](DeviceInfo & deviceInfo)
                        {
                            std::cout << "###Device Information received...." << std::endl;
                            std::ostringstream out;
                            out << "Device name: " << deviceInfo.getName() << std::endl;
                            out << "Device ID: " << deviceInfo.getID() << std::endl;
                            out << "Device Spec version: " << deviceInfo.getSpecVersion() << std::endl;
                            out << "Device dat model version: " << deviceInfo.getDataModelVersion() << std::endl;

                            std::cout << out.str() << std::endl;
                        }, std::placeholders::_1));
                    }
                    catch (InvalidArgsException &e)
                    {
                        std::cout << "InvalidArgsException occured [code : " << e.code() << " Detail: " << e.what() << "]"
                                  << std::endl;
                    }
                    catch (SimulatorException &e)
                    {
                        std::cout << "SimulatorException occured [code : " << e.code() << " Detail: " << e.what() << "]" <<
                                  std::endl;
                    }
                }
                break;

            case 3:
                {
                    try
                    {
                        SimulatorManager::getInstance()->getPlatformInfo(std::bind([](PlatformInfo & platformInfo)
                        {
                            std::cout << "###Platform Information received...." << std::endl;
                            std::ostringstream out;
                            out << "Platform ID: " << platformInfo.getPlatformID() << std::endl;
                            out << "Platform version: " << platformInfo.getPlatformVersion() << std::endl;
                            out << "Manufacturer name: " << platformInfo.getManufacturerName() << std::endl;
                            out << "Manufacturer url: " << platformInfo.getManufacturerUrl() << std::endl;
                            out << "Modle number: " << platformInfo.getModelNumber() << std::endl;
                            out << "Date of manufacture: " << platformInfo.getDateOfManfacture() << std::endl;
                            out << "Operatio system version: " << platformInfo.getOSVersion() << std::endl;
                            out << "Hardware version: " << platformInfo.getHardwareVersion() << std::endl;
                            out << "Firmware version: " << platformInfo.getFirmwareVersion() << std::endl;
                            out << "Support url: " << platformInfo.getSupportUrl() << std::endl;
                            out << "System time: " << platformInfo.getSystemTime() << std::endl;

                            std::cout << out.str() << std::endl;
                        }, std::placeholders::_1));
                    }
                    catch (InvalidArgsException &e)
                    {
                        std::cout << "InvalidArgsException occured [code : " << e.code()
                                << " Detail: " << e.what() << "]" << std::endl;
                    }
                    catch (SimulatorException &e)
                    {
                        std::cout << "SimulatorException occured [code : " << e.code()
                                << " Detail: " << e.what() << "]" << std::endl;
                    }
                }
                break;

            case 4: setLogger(); break;

            case 5: printMainMenu(); break;

            case 0: cont = false;
        }
    }

    std::cout << "Terminating test !!!" << std::endl;
}
