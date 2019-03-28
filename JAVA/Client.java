/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package client;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author nicolas
 */
public class Client {
    private char[] pseudo = new char[50];
    private char[] address = new char[15];
    private Integer port;
    private Integer banned;

    /**
     * @param pseudo_p
     * @param address_p
     * @param port_p
     * @param banned_p 
     */
    
    public Client() {
        this.pseudo = new char[50];
        this.address = new char[15];
        this.port = 0;
        this.banned = 0;
    }
    
    public Client(String pseudo_p, String address_p, int port_p, int banned_p) {
        this.pseudo = pseudo_p.toCharArray();
        this.address = address_p.toCharArray();
        this.port = port_p;
        this.banned = banned_p;
    }
    
    public byte[] getBytes() {
        byte[] pseudo_b = new byte[50];
        byte[] address_b = new byte[15];
        for(int i = 0; i < this.pseudo.length; i++) {
            pseudo_b[i] = (byte)pseudo[i];
        }
        for(int i = 0; i < this.address.length; i++) {
            address_b[i] = (byte)address[i];
        }
       
        byte[] port_b = ByteBuffer.allocate(4).putInt(Integer.reverseBytes(this.port)).array();
        byte[] banned_b = ByteBuffer.allocate(4).putInt(Integer.reverseBytes(this.banned)).array();
        ByteArrayOutputStream os = new ByteArrayOutputStream();
        try {
            os.write(pseudo_b);
            os.write(address_b);
            os.write(port_b);
            os.write(banned_b);
        } catch (IOException ex) {
            Logger.getLogger(Client.class.getName()).log(Level.SEVERE, null, ex);
        }
        
        return os.toByteArray();
    }
    
    public static Client byte_to_client(byte[] data) {
        int MASK = 0xFF;
        char[] pseudo_buf = new char[50];
        char[] address_buf = new char[15];
        int port_buf = 0;
        int banned_buf = 0;
        for(int i = 0; i < 50; i++) {
            pseudo_buf[i] = (char)data[i];
        }
        for(int i = 50; i < 65; i++) {
            address_buf[i - 50] = (char)data[i];
        }
        port_buf = data[65] & MASK;
        port_buf = port_buf + ((data[66] & MASK) << 8);
        port_buf = port_buf + ((data[67] & MASK) << 16);
        port_buf = port_buf + ((data[68] & MASK) << 24);
        
        banned_buf = data[69] & MASK;
        banned_buf = banned_buf + ((data[70] & MASK) << 8);
        banned_buf = banned_buf + ((data[71] & MASK) << 16);
        banned_buf = banned_buf + ((data[72] & MASK) << 24);
        
        return new Client(String.valueOf(pseudo_buf), String.valueOf(address_buf), port_buf, banned_buf);
    }
    
    public char[] get_pseudo() {
        return this.pseudo;
    }
    
    public char[] get_address() {
        return this.address;
    }
    
    public Integer get_port() {
        return this.port;
    }
    
    public Integer get_banned() {
        return this.banned;
    }
    
    public String toString() {
        return ("pseudo : " + String.valueOf(this.pseudo) + " | address : " + String.valueOf(this.address) + " | port : " + this.port);
    }
}
