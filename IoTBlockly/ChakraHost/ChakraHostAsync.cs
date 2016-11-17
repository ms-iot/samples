using System;
using System.Collections;
using System.Runtime.InteropServices;
using Windows.System.Threading;
using Windows.Foundation;
using ChakraHost.Hosting;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace ChakraHost
{
    // ChackraHostAsync is derived from the original 'JavaScript Runtime Sample' you can find at
    // https://github.com/Microsoft/Chakra-Samples (full path at http://bit.ly/1QgvdWa), wrote by
    // our buddies working on Chakra :-)
    // We trimmed down the sample and added a touch of async to start the engine on a separate thread.
    //
    // We really like the power and flexibility of Chakra, a modern and fast JavaScript engine available
    // by default in every Windows 10 Edition (not just IoTCore). We leverage the same engine to power
    // Node.JS UWP apps running on IoTCore (check it out at http://aka.ms/ntvsiotlatest if you're interested).

    public class ChakraHostAsync
    {
        private JavaScriptSourceContext currentSourceContext = JavaScriptSourceContext.FromIntPtr(IntPtr.Zero);
        private JavaScriptRuntime runtime;

        public string result { get; private set; }

        private AutoResetEvent newScriptEvent = new AutoResetEvent(false);

        bool initialised = false;
        string script;

        public ChakraHostAsync()
        {

        }

        public string Init()
        {
            result = "";
            JavaScriptContext context;

            if (Native.JsCreateRuntime(JavaScriptRuntimeAttributes.AllowScriptInterrupt, null, out runtime) != JavaScriptErrorCode.NoError)
                return "failed to create runtime.";

            if (Native.JsCreateContext(runtime, out context) != JavaScriptErrorCode.NoError)
                return "failed to create execution context.";

            if (Native.JsSetCurrentContext(context) != JavaScriptErrorCode.NoError)
                return "failed to set current context.";

            // ES6 Promise callback
            JavaScriptPromiseContinuationCallback promiseContinuationCallback = delegate (JavaScriptValue task, IntPtr callbackState)
            {
            };

            if (Native.JsSetPromiseContinuationCallback(promiseContinuationCallback, IntPtr.Zero) != JavaScriptErrorCode.NoError)
                return "failed to setup callback for ES6 Promise";

            if (Native.JsProjectWinRTNamespace("Windows") != JavaScriptErrorCode.NoError)
                return "failed to project windows namespace.";

            if (Native.JsProjectWinRTNamespace("IoTBlocklyHelper") != JavaScriptErrorCode.NoError)
                return "failed to project windows namespace.";

            runtime.MemoryLimit = (UIntPtr)5000000; // limit JS memory

            return "NoError";
        }

        public string Dispose()
        {
            try
            {
                var res = Native.JsDisableRuntimeExecution(runtime);
                if (res != JavaScriptErrorCode.NoError)
                {
                    Debug.WriteLine(res);
                    return GetErrorMessage();
                }
                result = "";
                if (Native.JsDisposeRuntime(runtime) != JavaScriptErrorCode.NoError)
                {
                    return GetErrorMessage();
                }
            }
            catch (Exception e)
            {
                return "chakrahost: fatal error: internal error: " + e.Message;
            }
            return "";
        }

        public void ScriptProcessor()
        {
            JavaScriptValue result;

            while (true)
            {
                try
                {
                    newScriptEvent.WaitOne();

                    Init();

                    var res = Native.JsRunScript(script, currentSourceContext++, "", out result);

                }
                catch (Exception e)
                {
                    Debug.WriteLine("chakrahost: fatal error: internal error: " + e.Message);
                }
                finally
                {
                    if (runtime.IsValid)
                    {
                        Native.JsDisposeRuntime(runtime);
                        Pause(250); // I think this makes more stable
                    }
                }
            }
        }

        public void RunScriptAsync(string script, bool terminatePreviousScript = true)
        {
            if (!initialised)
            {
                initialised = true;
                Task.Run(new Action(ScriptProcessor));
            }

            this.script = script;

            HaltScript();

            newScriptEvent.Set();
        }

        public string HaltScript()
        {
            try
            {
                if (runtime.IsValid && !runtime.Disabled)
                {
                    var r = Native.JsDisableRuntimeExecution(runtime);
                }

            }
            catch (Exception e)
            {
                string msg = "chakrahost: fatal error: internal error: " + e.Message;
                Debug.WriteLine(msg);
                return msg;
            }
            return "";
        }

        private static string GetErrorMessage()
        {
            // Get error message and clear exception
            JavaScriptValue exception;
            if (Native.JsGetAndClearException(out exception) != JavaScriptErrorCode.NoError)
                return "failed to get and clear exception";

            JavaScriptPropertyId messageName;
            if (Native.JsGetPropertyIdFromName("message",
                out messageName) != JavaScriptErrorCode.NoError)
                return "failed to get error message id";

            JavaScriptValue messageValue;
            if (Native.JsGetProperty(exception, messageName, out messageValue)
                != JavaScriptErrorCode.NoError)
                return "failed to get error message";

            IntPtr message;
            UIntPtr length;
            if (Native.JsStringToPointer(messageValue, out message, out length) != JavaScriptErrorCode.NoError)
                return "failed to convert error message";

            return Marshal.PtrToStringUni(message);
        }

        private void Pause(int milliseconds)
        {
            Task.Delay(milliseconds).Wait();
        }
    }
}