// © Copyright(C) Microsoft. All rights reserved.

using System;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;
using Windows.Networking;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;

namespace bertha
{
    class TelnetClient
    {
        async public Task Connect(string server, int port = 23)
        {
            HostName hostname = new HostName(server);
            socket = new StreamSocket();
            await socket.ConnectAsync(hostname, port.ToString());
            reader = new DataReader(socket.InputStream);
            reader.InputStreamOptions = InputStreamOptions.Partial;
            writer = new DataWriter(socket.OutputStream);
        }

        public void Close()
        {
            reader.Dispose();
            reader = null;
            writer.Dispose();
            writer = null;
            socket.Dispose();
            socket = null;
        }

        async public void Listen(Action<string> readAction)
        {
            var sb = new StringBuilder();
            while (true)
            {
                try
                {
                    await reader.LoadAsync(BUFFER_SIZE);

                }
                catch(Exception e)
                {
                    Debug.WriteLine("Listen - LoadAsync - Exception");
                    break;
                }
                sb.Clear();
                while (reader.UnconsumedBufferLength > 0)
                {
                    await FilterTelnet(sb);
                }
                if (sb.Length > 0)
                {
                    readAction(sb.ToString());
                }
            }
        }

        async public Task Write(string msg)
        {
            byte[] buf = System.Text.Encoding.UTF8.GetBytes(msg.Replace("\0xFF", "\0xFF\0xFF"));
            writer.WriteBytes(buf);
            await writer.StoreAsync();
            await writer.FlushAsync();
        }

        enum Verbs
        {
            WILL = 251,
            WONT = 252,
            DO = 253,
            DONT = 254,
            IAC = 255
        }

        async Task FilterTelnet(StringBuilder sb)
        {
            int input = reader.ReadByte();
            switch (input)
            {
                case (int)Verbs.IAC:
                    // interpret as command
                    if (reader.UnconsumedBufferLength == 0)
                    {
                        await reader.LoadAsync(BUFFER_SIZE);
                    }
                    int inputverb = reader.ReadByte();
                    switch (inputverb)
                    {
                        case (int)Verbs.IAC:
                            //literal IAC = 255 escaped, so append char 255 to string
                            sb.Append((char)inputverb);
                            break;
                        case (int)Verbs.DONT:
                        case (int)Verbs.WONT:
                        case (int)Verbs.DO:
                        case (int)Verbs.WILL:
                            if (reader.UnconsumedBufferLength == 0)
                            {
                                await reader.LoadAsync(BUFFER_SIZE);
                            }
                            int inputoption = reader.ReadByte();
                            if (inputverb == (int)Verbs.DO || inputverb == (int)Verbs.WILL)
                            {
                                writer.WriteByte((byte)Verbs.IAC);
                                writer.WriteByte(inputverb == (int)Verbs.DO ? (byte)Verbs.WONT : (byte)Verbs.DONT);
                                writer.WriteByte((byte)inputoption);
                                await writer.StoreAsync();
                                await writer.FlushAsync();
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    sb.Append((char)input);
                    break;
            }
        }

        const uint BUFFER_SIZE = 1024 * 8;
        StreamSocket socket;
        DataReader reader;
        DataWriter writer;
    }
}
