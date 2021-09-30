using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net.Sockets;
using System.Text;
public class ServerHandler : MonoBehaviour
{
    private TcpClient _Client;

    private NetworkStream _stream;
    // Start is called before the first frame update
    void Start()
    {
        _Client = new TcpClient("localhost", 8081);
        print("Got connection");
        _stream = _Client.GetStream();
        
        //receive message
        Byte[] data = new Byte[256];
        int byteNum = _stream.Read(data, 0, 256);
        String message = Encoding.ASCII.GetString(data,0,byteNum);
        print(message);
        //send message
        //Send("yo yo yo");
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    public void Send(string message) {
        Byte[] msg = Encoding.ASCII.GetBytes(message);
        _stream.Write(msg,0,msg.Length);
    }
}
