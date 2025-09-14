using System;
using System.Collections.Concurrent;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace ConsoleApp10
{
    internal class Program
    {
        /// <summary>
        /// 服务器隧道端口
        /// </summary>
        const int serverPort = 12346;
        /// <summary>
        /// 开启穿透标识
        /// </summary>
        const string forwardOpenFlag = "snltty_open";
        /// <summary>
        /// 回复穿透标识
        /// </summary>
        const string forwardReplyFlag = "snltty_reply";
        /// <summary>
        /// 数据分隔符，esp8266那边读取的时候按分隔符读取
        /// </summary>
        const char delimiterChar = '|';
        /// <summary>
        /// 数据结束符，esp8266那边读取的时候按结束符读取
        /// </summary>
        const char endChar = ',';

        static void Main(string[] args)
        {
            //服务端，用于隧道连接
            var socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            socket.Bind(new IPEndPoint(IPAddress.Any, serverPort));
            socket.Listen(int.MaxValue);
            while (true)
            {
                var client = socket.Accept();
                byte[] buffer = new byte[1024];
                Task.Run(() =>
                {
                    while (true)
                    {
                        try
                        {
                            int length = client.Receive(buffer);
                            string[] messageArr = System.Text.Encoding.ASCII.GetString(buffer, 0, length).Split(delimiterChar);
                            switch (messageArr[0])
                            {
                                case forwardOpenFlag:
                                    //开启一个穿透服务
                                    Tunnel(client, int.Parse(messageArr[1]), messageArr[2], int.Parse(messageArr[3]));
                                    break;
                                default:
                                    break;
                            }
                        }
                        catch (Exception)
                        {
                            break;
                        }
                    }
                });
            }
        }
        static void Tunnel(Socket tunnel, int listenPort, string targetip, int targetPort)
        {
            try
            {
                //监听一个穿透服务

                var socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                socket.Bind(new IPEndPoint(IPAddress.Any, listenPort));
                socket.Listen(int.MaxValue);

                ConcurrentDictionary<int, TaskCompletionSource<Socket>> dic = new ConcurrentDictionary<int, TaskCompletionSource<Socket>>();
                byte[] flag = Encoding.ASCII.GetBytes($"{forwardReplyFlag}{delimiterChar}");
                int idCounter = 0;

                Task.Run(() =>
                {
                    while (true)
                    {
                        var client = socket.Accept();

                        Task.Run(async () =>
                        {
                            byte[] buffer = new byte[4 * 1024];
                            int length = await client.ReceiveAsync(buffer);
                            Memory<byte> memory = buffer.AsMemory();

                            //收到回复
                            if (memory.Length > flag.Length && memory.Slice(0, flag.Length).Span.SequenceEqual(flag))
                            {
                                int id = int.Parse(Encoding.ASCII.GetString(memory.Span).Split(delimiterChar)[1]);
                                if (dic.TryGetValue(id, out TaskCompletionSource<Socket> task))
                                {
                                    task.SetResult(client);
                                }
                            }
                            //新请求
                            else
                            {
                                //缓存一下，等待回复
                                int id = idCounter++;
                                TaskCompletionSource<Socket> task = new TaskCompletionSource<Socket>();
                                dic.TryAdd(id, task);

                                //通知客户端
                                await tunnel.SendAsync(Encoding.ASCII.GetBytes($"{listenPort}{delimiterChar}{id}{delimiterChar}{targetip}{delimiterChar}{targetPort}{endChar}"));

                                //等待回复
                                var target = await task.Task.WaitAsync(TimeSpan.FromSeconds(5));
                                //发送第一次收到的数据
                                await target.SendAsync(memory);
                                //交换数据
                                await Task.WhenAll(CopyAsync(client, target), CopyAsync(target, client));
                            }
                        });

                    }
                });
            }
            catch (Exception)
            {
            }
        }

        static async Task CopyAsync(Socket from, Socket to)
        {
            Memory<byte> memory = new byte[8192];
            try
            {
                int bytesRead;
                while ((bytesRead = await from.ReceiveAsync(memory, SocketFlags.None).ConfigureAwait(false)) != 0)
                {
                    await to.SendAsync(memory.Slice(0, bytesRead), SocketFlags.None).ConfigureAwait(false);
                }
            }
            catch (Exception)
            {
            }
            finally
            {
                from.Shutdown(SocketShutdown.Both);
                from.Dispose();

                to.Shutdown(SocketShutdown.Both);
                to.Dispose();
            }
        }
    }
}
