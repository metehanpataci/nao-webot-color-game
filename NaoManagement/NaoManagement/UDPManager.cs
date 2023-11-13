using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace NaoManagement
{
    class UDPManager
    {
        private string ipadress = "127.0.0.1";//dinlenecek ve gönderilecek ip adresi
        private int portno = 45100;//dinlenecek port
        private int clientPortNo = 45101;
        private UdpClient udp;
        IAsyncResult ar_ = null;
        private UdpClient client;
        private IPEndPoint ip;

        public UDPManager() 
        { 
            udp = new UdpClient(clientPortNo);
            createClient();

        }

        private void createClient() 
        {
            client = new UdpClient();
            ip = new IPEndPoint(IPAddress.Parse(ipadress), portno);
        }

        private void StartListening()
        {
            ar_ = udp.BeginReceive(Receive, new object());
        }

        private void Receive(IAsyncResult ar)
        {
            // izlenecek ip adresi tanımlanıyor
            IPEndPoint ip = new IPEndPoint(IPAddress.Parse(ipadress), portno);

            //udp okuma başlatılıyor
            byte[] bytes = udp.EndReceive(ar, ref ip);
            string icerik = Encoding.ASCII.GetString(bytes);
            /* alınan icerik burada işleniyor */

            // okuma tamamlandı ise tekrar okuyalım.
            StartListening();
        }

        public void Send(string message)
        {
            
            byte[] bytes = Encoding.ASCII.GetBytes(message);
            client.Send(bytes, bytes.Length, ip);
            
            Console.WriteLine("Gönderilen mesaj : {0} ", message);
        }

        private void closeClient() 
        {
            client.Close();
        }
    }
}
