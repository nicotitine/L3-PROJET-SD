package client;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.Enumeration;
import java.util.logging.Level;
import java.util.logging.Logger;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author nicolas
 */
public class Main {
    public static void main(String[] args) throws IOException {
         
        
        
      

        Client c = new Client("Tst", "192.168.1.29", 4000, 0);
        
        udp_handler ud = new udp_handler(c);
        ud.start();
        
        ud.interrupt();
        
       
        
    }
}
