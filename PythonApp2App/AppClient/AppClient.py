from WinRT.ApplicationModel import AppService
import time

greetingsReceived = False
serviceClosed = False

def OnServiceClosed(connection, eventArgs):
    print("OnServiceClosed() -", eventArgs.Status)
    global serviceClosed
    serviceClosed = True

def OnRequestReceived(conection, eventArgs):
    deferral = eventArgs.GetDeferal()
    print("OnRequestReceived()")
    print("Message:", eventArgs.Request.Message);
    print("Sending response...")
    status = eventArgs.Request.SendResponse({"Greetings": "Thank you"})
    print("Response status:", status)
    global greetingsReceived
    greetingsReceived = True
    deferral.Complete()

def PrintMessage(dict):
    for k, v in sorted(dict.items()):
        print('\t', k,'\t', v)

def main():
    with AppService.AppServiceConnection() as conn:
        conn.ServiceClosed.Add(OnServiceClosed)
        conn.RequestReceived.Add(OnRequestReceived)

        conn.AppServiceName = "HelloService"
        conn.PackageFamilyName = "Service-uwp_gpek5j0d8wyr0"

        print("Opening...")
        status = conn.Open()
        print("Open status:", status)
        while (not greetingsReceived):
            time.sleep(1)

        print()
        print("Send test message...")
        respond = conn.SendMessage({"Action":"TestValues"})
        print("Status:", respond.Status)
        print("Message:")
        PrintMessage(respond.Message)
        testValues = respond.Message

        print()
        print("Send echo message...")
        respond = conn.SendMessage({"Action":"Echo", "String": "this should be echo'ed back"})
        print("Status:", respond.Status)
        print("Message:")
        PrintMessage(respond.Message)

        print()
        print("Send echo test value message...")
        testValues["Action"] = "Echo"
        respond = conn.SendMessage(testValues)
        print("Status:", respond.Status)
        print("Message:")
        PrintMessage(respond.Message)

        print()
        print("Send disconnect message...")
        respond = conn.SendMessage({"Action":"Disconnect"})
        print("Status:", respond.Status)
        print("Message:", respond.Message)

        while (not serviceClosed):
            time.sleep(1)

main()
