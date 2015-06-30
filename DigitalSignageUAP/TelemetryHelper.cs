using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;
using Microsoft.Diagnostics.Tracing;

namespace DigitalSignageUAP
{
    public class TelemetryHelper
    {
        // Trait that declares the provider's membership in the Microsoft Telemetry provider group
        private static string[] traits = { "ETW_GROUP", "{4f50731a-89cf-4782-b3e0-dce8c90476ba}" };

        private const EventKeywords MicrosoftTelemetry = (EventKeywords)0x0200000000000;
        private static readonly string EventProvider = "Microsoft.Windows.IoT.Client.Athens.DigitalSignageUAP";
        public static EventSource eventLogger = new EventSource(EventProvider, EventSourceSettings.EtwSelfDescribingEventFormat, traits);

        public static EventSourceOptions DebugVerboseOption = new EventSourceOptions() { Level = EventLevel.Verbose };
        public static EventSourceOptions DebugErrorOption = new EventSourceOptions() { Level = EventLevel.Error };
        public static EventSourceOptions TelemetryInfoOption = new EventSourceOptions() { Keywords = MicrosoftTelemetry };
        public static EventSourceOptions TelemetryErrorOption = new EventSourceOptions() { Level = EventLevel.Error, Keywords = MicrosoftTelemetry };
        public static EventSourceOptions TelemetryStartOption = new EventSourceOptions() { Opcode = EventOpcode.Start, Keywords = MicrosoftTelemetry };
        public static EventSourceOptions TelemetryStopOption = new EventSourceOptions() { Opcode = EventOpcode.Stop, Keywords = MicrosoftTelemetry };

        // For app Launch and Exit events, we rely on built in events from Threshold
        // KernelProcess.ProcessStarted
        // KernelProcess.ProcessStopped

        public static readonly string HeartbeatEvent = "HeartbeatEvent"; // every 30 minutes
        public static readonly string SlideShowLaunchExitEvent = "SlideshowStartExitEvent";
        public static readonly string BrowserLaunchExitEvent = "BrowserStartExitEvent";
        public static readonly string UserCommentEvent = "UserCommentEvent";
        public static readonly string VisionVideoEvent = "IoTVisionStartExitEvent";
        public static readonly string UnhandledExceptionEvent = "UnhandledExceptionEvent";
        public static readonly string SettingsChangedEvent = "SettingsChangedEvent";
    }
}
