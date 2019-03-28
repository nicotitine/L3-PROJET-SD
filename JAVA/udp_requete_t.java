/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package client;

import java.nio.ByteBuffer;

/**
 *
 * @author nicolas
 */
public enum udp_requete_t{
    ADD_CLIENT((Integer)1),
    REFRESH_TABLE((Integer)2),
    REMOVE_CLIENT((Integer)3),
    BAN_CLIENT((Integer)4),
    UNBAN_CLIENT((Integer)5),
    TOO_MUCH_CLIENT((Integer)6),
    STILL_ALIVE((Integer)7),
    SERVER_CLOSED((Integer)8),
    PSEUDO_ALREADY_TAKEN((Integer)9);
    

    private final Integer type;

    private udp_requete_t(Integer type_p) {
        this.type = type_p;
    }
    
    public Integer toInt() {
        return this.type;
    }

    public byte[] getBytes() {
        return ByteBuffer.allocate(4).putInt(Integer.reverseBytes(this.type)).array();
    }
}
