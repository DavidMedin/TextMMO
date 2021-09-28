using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net.Sockets;
public class ServerHandler : MonoBehaviour
{
    private TcpClient _Client;
    // Start is called before the first frame update
    void Start()
    {
        _Client = new TcpClient("localhost", 8081);
        print("Got connection");
        NetworkStream stream = _Client.GetStream();
        
        //send message
        string msgStr = "yo yo yo";
        Byte[] msg = System.Text.Encoding.ASCII.GetBytes(msgStr);
        stream.Write(msg,0,msg.Length);
        
        //receive message
        Byte[] data = new Byte[256];
        int byteNum = stream.Read(data, 0, 256);
        String message = System.Text.Encoding.ASCII.GetString(data,0,byteNum);
        print(message);
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
