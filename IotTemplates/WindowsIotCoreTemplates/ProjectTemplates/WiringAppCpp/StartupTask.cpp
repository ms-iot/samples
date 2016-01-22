using namespace Windows::ApplicationModel::Background;

// These functions should be defined in the sketch file
void setup();
void loop();

namespace $safeprojectname$
{
    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class StartupTask sealed : public IBackgroundTask
    {
    public:
        virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance^ taskInstance) 
        {
            auto deferral = taskInstance->GetDeferral();

            setup();
            while (true)
            {
                loop();
            }

            deferral->Complete();
        }
    };
}
