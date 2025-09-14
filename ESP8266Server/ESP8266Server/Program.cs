using System.Net;
using System.Net.Sockets;
using System.Text;

namespace ESP8266Server
{
    internal class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("输入你要监听的端口:");
            int port = int.Parse(Console.ReadLine());

            Socket socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            socket.Bind(new IPEndPoint(IPAddress.Any, port));
            socket.Listen(int.MaxValue);

            Console.WriteLine($"已监听<{Dns.GetHostAddresses(Dns.GetHostName()).FirstOrDefault(c=>c.AddressFamily == AddressFamily.InterNetwork)}:{port}>正在等待esp8266连接");
            Console.WriteLine("===========================================");
            Console.WriteLine();

            Socket client = null;
            Task.Run(() =>
            {
                while (true)
                {
                    client = socket.Accept();
                    Console.WriteLine($"收到<{client.RemoteEndPoint}>的连接");
                    Console.WriteLine("===========================================");
                    Console.WriteLine();
                    Task.Run(() =>
                    {
                        try
                        {
                            while (true)
                            {
                                client.Receive(new byte[1024]);
                                client.Send(Encoding.ASCII.GetBytes($"10:pong:pong\n"));
                            }
                        }
                        catch (Exception)
                        {
                        }
                    });
                }
            });

            while (true)
            {
                if (client == null)
                {
                    Thread.Sleep(1000);
                    continue;
                }
                Console.WriteLine("输入命令并回车(down按下，up抬起):");
                string cmd = Console.ReadLine();
                if (string.IsNullOrWhiteSpace(cmd)) break;

                Console.WriteLine("输入参数并回车(没有则直接回车):");
                string content = Console.ReadLine();
                if (string.IsNullOrWhiteSpace(content)) content = "null";

                client.Send(Encoding.ASCII.GetBytes($"{cmd.Length + 2 + content.Length}:{cmd}:{content}\n"));
                Console.WriteLine($"已发送<{cmd.Length + 2 + content.Length}:{cmd}:{content}>");
                Console.WriteLine("===========================================");
                Console.WriteLine();
            }
        }
    }
}
