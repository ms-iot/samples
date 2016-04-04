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

class AppLogger : public ILogger
{
    public:
        void write(std::string time, ILogger::Level level, std::string message)
        {
            std::cout << "[APPLogger] " << time << " " << ILogger::getString(level) << " "
                      << message;
        }
};
std::shared_ptr<AppLogger> gAppLogger(new AppLogger());

class SimLightResource
{
    public:
        void startTest(std::string &configPath)
        {
            printMenu();
            bool cont = true;
            while (cont)
            {
                int choice = -1;
                std::cout << "Enter your choice: ";
                std::cin >> choice;
                if (choice < 0 || choice > 10)
                {
                    std::cout << "Invaild choice !" << std::endl; continue;
                }

                switch (choice)
                {
                    case 1 : simulateResource(configPath); break;
                    case 2 : displayResource(); break;
                    case 3 :
                        try
                        {
                            deleteResource();
                        }
                        catch (InvalidArgsException &e)
                        {
                            std::cout << "InvalidArgsException occured [code : " << e.code() <<
                                      " Detail: " << e.what() << "]" << std::endl;
                        }
                        break;
                    case 4 : updateAttributePower(); break;
                    case 5 : updateAttributeIntensity(); break;
                    case 6 : automateResourceUpdate(); break;
                    case 7 : automateAttributeUpdate(); break;
                    case 8 : stopAutomation(); break;
                    case 9 : getObservers(); break;
                    case 10: printMenu(); break;
                    case 0: cont = false;
                }
            }
        }

    private:
        void printMenu()
        {
            std::cout << "########### LIGHT RESOURCE TESTING ###########" << std::endl;
            std::cout << "1. Simulate resource" << std::endl;
            std::cout << "2. Display resource information" << std::endl;
            std::cout << "3. Delete resource" << std::endl;
            std::cout << "4. Update attribute \"power\"" << std::endl;
            std::cout << "5. Update attribute \"intensity\"" << std::endl;
            std::cout << "6. Automate resource update" << std::endl;
            std::cout << "7. Automate attributes update" << std::endl;
            std::cout << "8. Stop Automation" << std::endl;
            std::cout << "9. Get Observers of a resource" << std::endl;
            std::cout << "10: Help" << std::endl;
            std::cout << "0. Exit" << std::endl;
            std::cout << "#######################################" << std::endl;
        }

        int selectResource()
        {
            if (0 == m_resources.size())
            {
                std::cout << "No resouces!" << std::endl;
                return -1;
            }

            int index = 1;
            for (auto & resource : m_resources)
            {
                std::cout << index++ << ": " << resource->getURI().c_str() << std::endl;
            }

            int choice = -1;
            std::cout << "Choose the resource: ";
            std::cin >> choice;

            if (choice < 1 || choice > index - 1)
            {
                std::cout << "Invalid choice !" << std::endl;
                choice = -1;
            }

            return choice;
        }

        void onResourceModelChanged(const std::string &uri,
                                    const SimulatorResourceModel &resModel)
        {
            std::cout << "[callback] Resource model is changed URI: " << uri.c_str()
                      << " Count : " << resModel.size() << std::endl;
            std::cout << "#### Modified attributes are ####" << std::endl;
            for (auto & attribute : resModel.getAttributes())
            {
                std::cout << attribute.second.getName() << " :  "
                          << attribute.second.valueToString().c_str() << std::endl;
            }
            std::cout << "########################" << std::endl;
        }

        void simulateResource(std::string &configPath)
        {
            SimulatorResourceServer::ResourceModelChangedCB callback = std::bind(
                        &SimLightResource::onResourceModelChanged, this, std::placeholders::_1,
                        std::placeholders::_2);

            try
            {
                SimulatorResourceServerSP resource =
                    SimulatorManager::getInstance()->createResource(configPath, callback);
                m_resources.push_back(resource);
                std::cout << "Resource created successfully! URI= " << resource->getURI().c_str()
                          << std::endl;
            }
            catch (InvalidArgsException &e)
            {
                std::cout << "InvalidArgsException occured [code : " << e.code() << " Detail: "
                          << e.what() << "]" << std::endl;
            }
            catch (SimulatorException &e)
            {
                std::cout << "SimulatorException occured [code : " << e.code() << " Detail: "
                          << e.what() << "]" << std::endl;
            }
        }

        void deleteResource()
        {
            int choice = -1;
            std::cout << "1. Delete single resource" << std::endl;
            std::cout << "2. Delete resources on resource types" << std::endl;
            std::cout << "3. Delete all resources" << std::endl;

            std::cout << "Enter your choice: ";
            std::cin >> choice;
            if (choice < 1 || choice > 3)
            {
                std::cout << "Invalid choice !" << std::endl;
                return;
            }

            switch (choice)
            {
                case 1:
                    {
                        int index = selectResource();
                        if (-1 == index)
                            return;

                        SimulatorManager::getInstance()->deleteResource(m_resources[index - 1]);
                        std::cout << "Resource deleted successfully! " << std::endl;
                        m_resources.erase(m_resources.begin() + (index - 1));

                    } break;

                case 2:
                    {
                        std::string resourceType;
                        std::cout  << "Enter resource type:  ";
                        std::cin >> resourceType;
                        if (resourceType.empty())
                        {
                            std::cout << "Invalid resource type!" << std::endl;
                            break;
                        }

                        try
                        {
                            SimulatorManager::getInstance()->deleteResource(resourceType);
                            std::cout << "Resources of type \"" << resourceType << "\"" <<
                                      " deleted successfully! " << std::endl;
                            std::vector<SimulatorResourceServerSP>::iterator ite = m_resources.begin();
                            while (ite != m_resources.end())
                            {
                                if (!resourceType.compare((*ite)->getResourceType()))
                                {
                                    ite = m_resources.erase(ite);
                                    continue;
                                }
                                ite++;
                            }
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
                    } break;

                case 3:
                    {
                        SimulatorManager::getInstance()->deleteResource();
                        std::cout << "All resources deleted successfully! " << std::endl;
                        m_resources.clear();
                    } break;
            }

        }

        void updateAttributePower()
        {
            int index = selectResource();
            if (-1 == index)
                return;

            SimulatorResourceServerSP resource = m_resources[index - 1];
            SimulatorResourceModel resModel = resource->getModel();
            SimulatorResourceModel::Attribute powerAttribute;
            resModel.getAttribute("power", powerAttribute);

            int allowedValuesSize = powerAttribute.getAllowedValuesSize();
            if (0 == allowedValuesSize)
            {
                std::cout << "This attribute does not have allowed values!" << std::endl;
                return;
            }

            std::cout << "Setting the new values from allowed values list to power attribute" <<
                      std::endl;
            // Update all possible values from allowed values
            for (int index = 0; index < allowedValuesSize; index++)
            {
                // Update the new value and display the resource model after modifying
                resource->updateFromAllowedValues("power", index);
                std::cout << "Attribute value is modified ####" << std::endl;

                // Display the resource to user to verify the changed attribute value
                displayResource(resource);
                std::cout << std::endl << std::endl;

                // Get user input for continuing this operation
                if ((index + 1) < allowedValuesSize)
                {
                    int choice;
                    std::cout << "Would you like to change attribute value again ? (1/0): ";
                    std::cin >> choice;
                    if (0 == choice)
                        break;
                }
            }

            std::cout << "All the allowed values are tried!" << std::endl;
        }

        void updateAttributeIntensity()
        {
            int index = selectResource();
            if (-1 == index)
                return;

            SimulatorResourceServerSP resource = m_resources[index - 1];
            SimulatorResourceModel resModel = resource->getModel();
            SimulatorResourceModel::Attribute intensityAttribute;
            resModel.getAttribute("intensity", intensityAttribute);

            int min, max;
            intensityAttribute.getRange(min, max);
            if (!min && !max)
            {
                std::cout << "This attribute does not have range!" << std::endl;
                return;
            }

            std::cout << "Setting the new values from allowed values list to intensity attribute"
                      << std::endl;
            // Update all possible values from allowed values
            for (int index = min; index <= max; index++)
            {
                // Update the new value and display the resource model after modifying
                resource->updateAttributeValue("intensity", index);
                std::cout << "Attribute value is modified ####" << std::endl;

                // Display the resource to user to verify the changed attribute value
                displayResource(resource);
                std::cout << std::endl << std::endl;

                // Get user input for continuing this operation
                if ((index + 1) <= max)
                {
                    int choice;
                    std::cout << "Would you like to change attribute value again ? (1/0): ";
                    std::cin >> choice;
                    if (0 == choice)
                        break;
                }
            }

            std::cout << "All the allowed values are tried!" << std::endl;
        }

        void displayResource()
        {
            int index = selectResource();
            if (-1 == index)
                return;

            SimulatorResourceServerSP resource = m_resources[index - 1];
            displayResource(resource);
        }

        void displayResource(SimulatorResourceServerSP resource)
        {
            std::cout << "#############################" << std::endl;
            std::cout << "Name: " << resource->getName().c_str() << std::endl;
            std::cout << "URI: " << resource->getURI().c_str() << std::endl;
            std::cout << "R. Type: " << resource->getResourceType().c_str() << std::endl;
            std::cout << "I. Type: " << resource->getInterfaceType().c_str() << std::endl;

            // Attributes
            SimulatorResourceModel resModel = resource->getModel();
            std::map<std::string, SimulatorResourceModel::Attribute> attributes =
                resModel.getAttributes();
            std::cout << "##### Attributes [" << attributes.size() << "]" << std::endl;
            for (auto & attribute : attributes)
            {
                std::cout << (attribute.second).getName() << " :  {" << std::endl;
                std::cout << "value: " << (attribute.second).valueToString().c_str() << std::endl;
                int min, max;
                (attribute.second).getRange(min, max);
                std::cout << "min: " << min << std::endl;
                std::cout << "max: " << max << std::endl;
                std::cout << "allowed values : ";
                std::cout << "[ ";
                for (auto & value : (attribute.second).allowedValuesToString())
                    std::cout << value << " ";
                std::cout << "]" << std::endl;
                std::cout << "}" << std::endl << std::endl;
            }
            std::cout << "#############################" << std::endl;
        }

        void onUpdateAutomationCompleted(const std::string &uri,
                                         const int id)
        {
            std::cout << "Update automation is completed [URI: " << uri.c_str()
                      << "  AutomationID: " << id << "] ###" << std::endl;
        }

        void automateResourceUpdate()
        {
            int index = selectResource();
            if (-1 == index)
                return;

            AutomationType type = AutomationType::NORMAL;
            int choice = 0;
            std::cout << "Press 1 if you want recurrent automation: ";
            std::cin >> choice;
            if (1 == choice)
                type = AutomationType::RECURRENT;

            try
            {
                int id = m_resources[index - 1]->startUpdateAutomation(type,
                         std::bind(&SimLightResource::onUpdateAutomationCompleted, this,
                                   std::placeholders::_1, std::placeholders::_2));

                std::cout << "startUpdateAutomation() returned succces : " << id << std::endl;
            }
            catch (SimulatorException &e)
            {
                std::cout << "SimulatorException occured [code : " << e.code() << " Detail: " <<
                          e.what() << "]" << std::endl;
            }
        }

        void automateAttributeUpdate()
        {
            int index = selectResource();
            if (-1 == index)
                return;

            SimulatorResourceServerSP resource = m_resources[index - 1];
            SimulatorResourceModel resModel = resource->getModel();
            std::map<std::string, SimulatorResourceModel::Attribute> attributes =
                resModel.getAttributes();
            int size = 0;
            for (auto & attribute : attributes)
            {
                std::cout << ++size << ": " << attribute.first.c_str() << std::endl;
            }

            if (0 == size)
            {
                std::cout << "This resource doest not contain any attributes!" << std::endl;
                return;
            }

            int choice = -1;
            std::cout << "Select the attribute which you want to automate for updation: " <<
                      std::endl;
            std::cin >> choice;
            if (choice < 0 || choice > size)
            {
                std::cout << "Invalid selection!" << std::endl;
                return;
            }

            int count = 0;
            std::string attributeName;
            for (auto & attribute : attributes)
            {
                if (count == choice - 1)
                {
                    attributeName = attribute.first;
                    break;
                }

                count++;
            }

            AutomationType type = AutomationType::NORMAL;
            std::cout << "Press 1 if you want recurrent automation: ";
            std::cin >> choice;
            if (1 == choice)
                type = AutomationType::RECURRENT;

            std::cout << "Requesting attribute automation for " << attributeName.c_str() <<
                      std::endl;

            try
            {

                int id = resource->startUpdateAutomation(attributeName, type,
                         std::bind(&SimLightResource::onUpdateAutomationCompleted, this,
                                   std::placeholders::_1, std::placeholders::_2));
                std::cout << "startUpdateAutomation() returned succces : " << id << std::endl;
            }
            catch (SimulatorException &e)
            {
                std::cout << "SimulatorException occured [Error: " << e.code() << " Details: " <<
                          e.what() << "]" << std::endl;
            }
        }

        void stopAutomation()
        {
            int index = selectResource();
            if (-1 == index)
                return;

            SimulatorResourceServerSP resource = m_resources[index - 1];

            // Select the automation to stop
            std::vector<int> ids;
            {
                std::vector<int> rids = resource->getResourceAutomationIds();
                std::vector<int> aids = resource->getAttributeAutomationIds();
                ids.insert(ids.end(), rids.begin(), rids.end());
                ids.insert(ids.end(), aids.begin(), aids.end());
            }

            if (!ids.size())
            {
                std::cout << "No automation operation is going on this resource right now!" <<
                          std::endl;
                return;
            }

            for (auto & id : ids)
                std::cout <<  id  << " ";

            int automationid;
            std::cout << "\nEnter automation id: " << std::endl;
            std::cin >> automationid;
            resource->stopUpdateAutomation(automationid);
        }

        void onObserverChanged(const std::string &uri, ObservationStatus state,
                               const ObserverInfo &observerInfo)
        {
            std::cout << "[callback] Observer notification received..." << uri.c_str() << std::endl;
            std::ostringstream out;
            out << "ID:  " << (int) observerInfo.id << std::endl;
            out << " [address: " << observerInfo.address << " port: " << observerInfo.port
                << "]" << std::endl;
            std::cout << out.str();
        }

        void getObservers()
        {
            int index = selectResource();
            if (-1 == index)
                return;

            SimulatorResourceServerSP resource = m_resources[index - 1];

            SimulatorResourceServer::ObserverCB callback = std::bind(
                        &SimLightResource::onObserverChanged, this, std::placeholders::_1,
                        std::placeholders::_2, std::placeholders::_3);
            resource->setObserverCallback(callback);

            std::vector<ObserverInfo> observersList = resource->getObserversList();

            std::cout << "##### Number of Observers [" << observersList.size() << "]" << std::endl;
            for (auto & observerInfo : observersList)
            {
                std::cout << " ID :  " << (int) observerInfo.id << " [address: " <<
                          observerInfo.address << " port: " << observerInfo.port << "]" << std::endl;
            }
            std::cout << "########################" << std::endl;
        }

    private:
        std::vector<SimulatorResourceServerSP> m_resources;
};

void printMainMenu()
{
    std::cout << "############### MAIN MENU###############" << std::endl;
    std::cout << "1. Test simulation of resource" << std::endl;
    std::cout << "2. Set Logger" << std::endl;
    std::cout << "3. Help" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout <<
              "To set the Resource from RAML file, run the service provider with argument of Path of Raml File."
              << std::endl;
    std::cout <<
              "Example: ./simulator-server PATH-TO-RAML-FILE"
              << std::endl;
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
            } break;
        case 2:
            {
                std::string filePath;
                std::cout << "Enter the file path (without file name) : ";
                std::cin >> filePath;
                if (false == SimulatorManager::getInstance()->setFileLogger(filePath))
                    std::cout << "Failed to set default file logger" << std::endl;
            } break;
        case 3: SimulatorManager::getInstance()->setLogger(gAppLogger);
    }
}

int main(int argc, char *argv[])
{
    std::string configPath = "";

    if (argc == 2)
    {
        char *value = argv[1];
        configPath.append(value);
    }
    else
    {
        configPath = "";
    }
    SimLightResource lightResource;

    printMainMenu();
    bool cont = true;
    while (cont == true)
    {
        int choice = -1;
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        if (choice < 0 || choice > 3)
        {
            std::cout << "Invaild choice !" << std::endl; continue;
        }

        switch (choice)
        {
            case 1: lightResource.startTest(configPath);
                std::cout << "Welcome back to main menu !" << std::endl;
                break;
            case 2: setLogger(); break;
            case 3: printMainMenu(); break;
            case 0: cont = false;
        }
    }

    std::cout << "Terminating test !!!" << std::endl;
}
