/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package client;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author nico
 */
public class udp_handler extends Thread{
    
    private String address;
    private int port;
    private MulticastSocket socket;
    private InetAddress ip;
    private Client this_client;
    private Client[] other_clients;
    private int requete_size;
    private int my_socket_index;
    
    public udp_handler(Client this_client_p) {
        this.address = "226.1.2.3";
        this.port = 8888;
        this.this_client = this_client_p;
        this.other_clients = new Client[10];
        for(int i = 0; i < this.other_clients.length; i++) {
            this.other_clients[i] = new Client();
        }
    }
    
    public udp_handler(String address_p, int port_p, Client this_client_p) {
        this.address = address_p;
        this.port = port_p;
        this.this_client = this_client_p;
        this.other_clients = new Client[10];
        for(int i = 0; i < this.other_clients.length; i++) {
            this.other_clients[i] = new Client();
        }
    }
    
    @Override
    public void run() {
        try {
            this.socket = new MulticastSocket(port);
            this.ip = InetAddress.getByName(address);
            this.socket.joinGroup(ip);
        } catch (IOException ex) {
            Logger.getLogger(udp_handler.class.getName()).log(Level.SEVERE, null, ex);
        }
        udp_requete requete = new udp_requete(udp_requete_t.ADD_CLIENT, this.this_client, this.other_clients);
        this.requete_size = requete.getBytes().length;
        byte[] buffer = new byte[this.requete_size];
        DatagramPacket dp = new DatagramPacket(requete.getBytes(), requete_size, this.ip, this.port);
        DatagramPacket dp_rcv = new DatagramPacket(buffer, requete_size);
        try {
            this.socket.send(dp);
        } catch (IOException ex) {
            Logger.getLogger(udp_handler.class.getName()).log(Level.SEVERE, null, ex);
        }
        
        while(true) {
            try {
                this.socket.receive(dp_rcv);
                udp_requete requete_rcv = udp_requete.byte_to_requete(buffer);
                switch(requete_rcv.get_udp_requete_t()) {
                    case ADD_CLIENT:
                        //System.out.println(1);
                    break;
                    case REFRESH_TABLE:
                        for(int i = 0; i < 10; i++) {
                            if(requete_rcv.get_list()[i].get_pseudo()[0] != '\0' && this.other_clients[i].get_pseudo()[0] == '\0') {
                                if(requete_rcv.get_list()[i].get_pseudo() == this_client.get_pseudo()) {
                                    this.my_socket_index = i;
                                    System.out.println(i);
                                }
                            }
                        }
                    break;
                    case REMOVE_CLIENT:
                        System.out.println(3);
                    break;
                    case BAN_CLIENT:
                        System.out.println(4);
                    break;
                    case UNBAN_CLIENT:
                        System.out.println(5);
                    break;
                    case TOO_MUCH_CLIENT:
                        System.out.println(6);
                    break;
                    case STILL_ALIVE:
                        System.out.println(7);
                    break;
                    case SERVER_CLOSED:
                        System.out.println(8);
                    break;
                    case PSEUDO_ALREADY_TAKEN:
                        System.out.println(9);
                    break;
                }
            } catch (IOException ex) {
                Logger.getLogger(udp_handler.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }
    
}
