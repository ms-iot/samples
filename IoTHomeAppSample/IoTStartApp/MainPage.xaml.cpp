// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#include "pch.h"
#include "MainPage.xaml.h"

#include <ppltasks.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace IotStartApp;
using namespace std;

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Storage::Streams;
using namespace Concurrency;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Management::Deployment;

namespace IotStartApp
{
    vector<wstring> filterPackages{
        L"Microsoft.AAD.BrokerPlugin_cw5n1h2txyewy",
        L"Microsoft.AccountsControl_cw5n1h2txyewy",
        L"Microsoft.Windows.CloudExperienceHost_cw5n1h2txyewy",
        L"Microsoft.Windows.IoTShell.IoTDevicesFlow_cw5n1h2txyewy",
        L"Microsoft.Windows.IoTShell.IoTShellExperienceHost_cw5n1h2txyewy",
        L"Microsoft.Windows.IoTShell.OnScreenKeyboard_cw5n1h2txyewy",
        L"WebAuthBridgeInternetSso_cw5n1h2txyewy",
        L"WebAuthBridgeInternet_cw5n1h2txyewy",
        L"WebAuthBridgeIntranetSso_cw5n1h2txyewy",
        L"ZWaveAdapterHeadlessAdapterApp_1w720vyc4ccym",
        L"IoTOnboardingTask-uwp_1w720vyc4ccym",
        L"IotStartApp_1w720vyc4ccym"            //filter self
    };

    bool isPackageFiltered(wstring pkgFamilyName)
    {
        return(find(filterPackages.begin(), filterPackages.end(), pkgFamilyName) != filterPackages.end());
    }
}

MainPage::MainPage()
{
    InitializeComponent();
    EnumApplications();
}

void MainPage::EnumApplications()
{
    m_AppItemList = ref new Vector<AppListItem^>();
    auto mgr = ref new PackageManager();

    auto packages = mgr->FindPackagesForUserWithPackageTypes(nullptr, PackageTypes::Main);

    for (auto& pkg : packages)
    {
        if (isPackageFiltered(pkg->Id->FamilyName->Data())) continue;

        auto task = create_task(pkg->GetAppListEntriesAsync());
        task.then([this, pkg](IVectorView<AppListEntry^>^ entryList)
        {
            for (auto entry : entryList)
            {
                try
                {
                    auto displayInfo = entry->DisplayInfo;
                    auto logo = displayInfo->GetLogo(Size(150.0, 150.0));
                    
                    auto appItem = ref new AppListItem;
                    appItem->Name = displayInfo->DisplayName;
                    appItem->PackageFullName = pkg->Id->FullName;
                    appItem->AppEntry = entry;
                    appItem->ImgSrc = ref new BitmapImage();

                    create_task(logo->OpenReadAsync()).then([this, appItem](IRandomAccessStreamWithContentType^ stream)
                    {
                        appItem->ImgSrc->SetSourceAsync(stream);

                    });

                    m_AppItemList->Append(appItem);
                }
                catch (Exception^ e)
                {
                    OutputDebugString(e->Message->Data());
                }
                catch (...)
                {
                    OutputDebugString(L"Unknown Exception");
                }
            }
        });
    }


}

void IotStartApp::MainPage::StackPanel_Tapped(Object^ sender, TappedRoutedEventArgs^ e)
{
    auto appItem = dynamic_cast<AppListItem^>(appList->SelectedItem);

    if (appItem)
    {
        appItem->AppEntry->LaunchAsync();
    }
}
