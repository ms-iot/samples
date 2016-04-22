using System;
using System.Collections;
using System.Runtime.InteropServices;
using Windows.System.Threading;
using Windows.Foundation;
using ChakraHost.Hosting;
using System.Diagnostics;
using System.Threading;

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
        private Queue taskQueue = new Queue();
        private IAsyncAction workItem;
        public bool running { get; private set; }
        public string result { get; private set; }

        private readonly ManualResetEventSlim waitEvent = new ManualResetEventSlim(false);

        public ChakraHostAsync()
        {
        }

        public string init()
        {
            running = false;
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
                taskQueue.Enqueue(task);
            };

            if (Native.JsSetPromiseContinuationCallback(promiseContinuationCallback, IntPtr.Zero) != JavaScriptErrorCode.NoError)
                return "failed to setup callback for ES6 Promise";

            if (Native.JsProjectWinRTNamespace("Windows") != JavaScriptErrorCode.NoError)
                return "failed to project windows namespace.";

            if (Native.JsProjectWinRTNamespace("IoTBlocklyHelper") != JavaScriptErrorCode.NoError)
                return "failed to project windows namespace.";

            return "NoError";
        }

        public string dispose()
        {
            try
            {
                var res = Native.JsDisableRuntimeExecution(runtime);
                if (res != JavaScriptErrorCode.NoError)
                {
                    Debug.WriteLine(res);
                    return GetErrorMessage();
                }
                running = false;
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

        public void runScript(string script)
        {
            IntPtr returnValue;
            string resultMsg = "";

            try
            {
                if (init() != "NoError")
                {
                    resultMsg = GetErrorMessage();
                }
                else
                {
                    JavaScriptValue result;
                    running = true;

                    var res = Native.JsRunScript(script, currentSourceContext++, "", out result);
                    if (res == JavaScriptErrorCode.ScriptTerminated)
                    {
                        resultMsg = "script terminated";
                    }
                    else if (res != JavaScriptErrorCode.NoError)
                    {
                        resultMsg = GetErrorMessage();
                    }
                    else
                    {
                        // Execute promise tasks stored in taskQueue 
                        while (taskQueue.Count != 0)
                        {
                            JavaScriptValue task = (JavaScriptValue)taskQueue.Dequeue();
                            JavaScriptValue promiseResult;
                            JavaScriptValue global;
                            Native.JsGetGlobalObject(out global);
                            JavaScriptValue[] args = new JavaScriptValue[1] { global };
                            Native.JsCallFunction(task, args, 1, out promiseResult);
                        }

                        // Convert the return value.
                        JavaScriptValue stringResult;
                        UIntPtr stringLength;
                        if (Native.JsConvertValueToString(result, out stringResult) != JavaScriptErrorCode.NoError)
                        {
                            resultMsg = "failed to convert value to string.";
                        }
                        else if (Native.JsStringToPointer(stringResult, out returnValue, out stringLength) != JavaScriptErrorCode.NoError)
                        {
                            resultMsg = "failed to convert return value.";
                        }
                        else
                        {
                            resultMsg = Marshal.PtrToStringUni(returnValue);
                        }
                    }
                }
            }
            catch (Exception e)
            {
                resultMsg = "chakrahost: fatal error: internal error: " + e.Message;
            }
            running = false;
            result = resultMsg;
            Debug.WriteLine(resultMsg);
        }

        public void runScriptAsync(string script, bool terminatePreviousScript = true)
        {
            if (terminatePreviousScript)
            {
                if (running)
                {
                    haltScript();
                }
            }

            workItem = ThreadPool.RunAsync(
                (workItem) =>
                {
                    runScript(script);
                });
        }

        public string haltScript()
        {
            while (running)
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
                }
                catch (Exception e)
                {
                    string msg = "chakrahost: fatal error: internal error: " + e.Message;
                    Debug.WriteLine(msg);
                    return msg;
                }
                Pause(50);
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

        private void Pause(double milliseconds)
        {
            waitEvent.Wait(TimeSpan.FromMilliseconds(milliseconds));
        }

    }
}