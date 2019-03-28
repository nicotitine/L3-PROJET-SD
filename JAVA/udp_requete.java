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
public class udp_requete {
    private udp_requete_t requete_type;
    private Client sender;
    private Client list[];
    
    public udp_requete() {
        this.requete_type = null;
        this.sender = null;
        this.list = new Client[10];
    }

    public udp_requete(udp_requete_t requete_type_p, Client sender_p, Client[] list_p) {
        this.requete_type = requete_type_p;
        this.sender = sender_p;
        this.list = list_p;
    }

    public byte[] getBytes() {
        int i;
        byte[] sender_b, requete_type_b;
        ByteArrayOutputStream os = null, os_return = null;
        
        try {
            os = new ByteArrayOutputStream();
            os_return = new ByteArrayOutputStream();
            requete_type_b = this.requete_type.getBytes();
            sender_b = this.sender.getBytes();
            for(i = 0; i < 10; i++) {
                if(this.list != null)
                    os.write(this.list[i].getBytes());
            }   
            
            os_return.write(requete_type_b);
            os_return.write(sender_b);
            os_return.write(os.toByteArray());
            
        } catch (IOException ex) {
            Logger.getLogger(udp_requete.class.getName()).log(Level.SEVERE, null, ex);
        }
        return os_return.toByteArray();
    }
    
    public static udp_requete byte_to_requete(byte[] data) {
        udp_requete buf = new udp_requete();
        int MASK = 0xFF;
        int requete_t_buf = 0;        
       
        
        requete_t_buf = data[0] & MASK;
        requete_t_buf = requete_t_buf + ((data[1] & MASK) << 8);
        requete_t_buf = requete_t_buf + ((data[2] & MASK) << 16);
        requete_t_buf = requete_t_buf + ((data[3] & MASK) << 24);
        
        switch(requete_t_buf) {
            case 1:
                buf.requete_type = udp_requete_t.ADD_CLIENT;
            break;
            case 2:
                buf.requete_type = udp_requete_t.REFRESH_TABLE;
            break;
            case 3:
                buf.requete_type = udp_requete_t.REMOVE_CLIENT;
            break;
            case 4:
                buf.requete_type = udp_requete_t.BAN_CLIENT;
            break;
            case 5:
                buf.requete_type = udp_requete_t.UNBAN_CLIENT;
            break;
            case 6:
                buf.requete_type = udp_requete_t.TOO_MUCH_CLIENT;
            break;
            case 7:
                buf.requete_type = udp_requete_t.STILL_ALIVE;
            break;
            case 8:
                buf.requete_type = udp_requete_t.SERVER_CLOSED;
            break;
            case 9:
                buf.requete_type = udp_requete_t.PSEUDO_ALREADY_TAKEN;
            break;
            
        }
        
        byte[] client_byte = new byte[73];
        for(int i = 4; i < 77; i++) {
            client_byte[i-4] = data[i];
        }
        buf.sender = Client.byte_to_client(client_byte);
       
        int index = 77;
        
        for(int i = 0; i < 10; i++) {
            for(int j = index; j < index + 73; j++) {
                client_byte[j - index] = data[j];
            }
            buf.list[i] = Client.byte_to_client(client_byte);
            
            index += 73;
        }
        
       
        
        
        
        return buf;
    }
    
    public udp_requete_t get_udp_requete_t() {
        return this.requete_type;
    }
    
    public Client get_sender() {
        return this.sender;
    }
    
    public Client[] get_list() {
        return this.list;
    }
    
   
}
