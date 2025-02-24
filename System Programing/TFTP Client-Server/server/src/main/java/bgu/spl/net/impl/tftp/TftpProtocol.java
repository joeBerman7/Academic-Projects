package bgu.spl.net.impl.tftp;

import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;

import java.util.concurrent.ConcurrentHashMap;
import java.util.LinkedList;

import bgu.spl.net.api.BidiMessagingProtocol;

import bgu.spl.net.srv.Connections;


import java.io.File;

import java.io.FileOutputStream;
import java.io.IOException;


class handler {
    static public ConcurrentHashMap<String, Integer> logged_User = new ConcurrentHashMap<>();
    static public LinkedList<String> ListAllFiles= new LinkedList<>();
    static {
        String path = "./Flies";
        File dirFiles = new File(path);
        File[] AllFiles = dirFiles.listFiles();
        
        for (File file : AllFiles) {
            if (file.isFile()) {
                ListAllFiles.add(file.getName());
            }  
        }
    }

}

public class TftpProtocol implements BidiMessagingProtocol<byte[]> {

    private Connections<byte[]> connections;
    private boolean shouldTerminate = false;
    private int connectionId;
    public String user;
    private boolean EnteredFirstPacket = false;
    private boolean login_error = false;
    public byte[] filename;
    public byte[] fileInBytes=new byte[1024];;
    public FileOutputStream destStream;
    public boolean lastData=false;    

    enum ErrorMsg {
        NOT_DEFINED(0, "Not defined, see error message (if any)."),
        FILE_NOT_FOUND(1, "File not found – RRQ DELRQ of non-existing file."),
        ACCESS_VIOLATION(2, "Access violation – File cannot be written, read, or deleted."),
        DISK_FULL(3, "Disk full or allocation exceeded – No room in disk."),
        ILLEGAL_OPERATION(4, "Illegal TFTP operation – Unknown Opcode."),
        FILE_ALREADY_EXISTS(5, "File already exists – File name exists on WRQ."),
        USER_NOT_LOGGED_IN(6, "User not logged in – Any opcode received before Login completes."),
        USER_ALREADY_LOGGED_IN(7, "User already logged in – Login username already connected.");

        private final int ErrorCode;
        private final String message;

        ErrorMsg(int ErrorCode, String message) {
            this.ErrorCode = ErrorCode;
            this.message = message;
        }

        public int getErrorCode() {
            return ErrorCode;
        }

        public String getMessage() {
            return message;
        }
    }

    @Override
    public void start(int connectionId, Connections<byte[]> connections) {
        this.shouldTerminate = false;
        this.connectionId = connectionId;
        this.connections = connections;
    }

    @Override
    public void process(byte[] message) {

        byte[] op_code = new byte[2];
        short op_code_short = 0;
        byte[] usernameBytes;
        short block = 1;

        // get op_code
        op_code[0] = message[0];
        op_code[1] = message[1];
        op_code_short = (short) (((short) message[0]) << 8 | (short) (message[1]) & 0x00ff);

        switch (op_code_short) {
            case 1:
                // RRQ
                if (EnteredFirstPacket) {
                    // save the filename
                    int k = 0;
                    filename = new byte[message.length-2];
                    for (int i = 2; i < message.length; i++) {
                        filename[k] = message[i];
                        k = k + 1;
                    }

                    String filenameStr = convertToString(filename);
                    // File FilePath = new File("./Flies/" + filenameStr);
                    Path filePath = Path.of("./Flies/"+filenameStr);
                    System.out.println("file path "+filePath);
                    if (Files.exists(filePath)) {
                        try{
                            System.out.println("exists");
                            fileInBytes=Files.readAllBytes(filePath);
                            // create DATA packet and send it with file itself
                            connections.send(connectionId, createDataPacket(fileInBytes, 0, block));
                            break;
                            }
                            catch (IOException e) {
                                e.printStackTrace();
                            }
                    } else {
                        fileNotFound();
                    }
                } else {
                    notLoggedInError();
                    
                }
                break;

            case 2:
                // WRQ 
                if (EnteredFirstPacket) {
                    // save filename
                    int m = 0;
                    filename = new byte[message.length-2];
                    for (int i = 2; i < message.length; i++) {
                        filename[m] = message[i];
                        m = m + 1;
                    }

                    String filenameUploadStr = convertToString(filename);
                    File FileToUpload = new File("./Flies/" + filenameUploadStr);
                    if (FileToUpload.exists()) {
                        file_exist();
                        // error - you cant upload if it's exists
                    } else {
                        try {
                            destStream = new FileOutputStream(FileToUpload);
                            connections.send(connectionId, createAckPacket());
                        } catch (IOException e) {
                        }
                    }
                } else {
                    notLoggedInError();
                }
                break;

            case 3:
                // DATA
                //get size packet
                byte[] sizePacketFromClient = new byte[2];
                sizePacketFromClient[0] = message[2];
                sizePacketFromClient[1] = message[3];
                short data_size = (short) (((short) sizePacketFromClient[0]) << 8
                        | (short) (sizePacketFromClient[1]) & 0x00ff);
                byte[] blockNumberFromClient = new byte[2];
                blockNumberFromClient[0] = message[4];
                blockNumberFromClient[1] = message[5];
                byte brodcast_flag = 0;

                // save the Data from packet

                for (int i = 6; i < data_size+6; i++) {
                    try {
                        destStream.write(message[i]);
                    } catch (IOException e) {
                    }
                }

                // if LAST PACKET - send ack, broadcast and close stream
                if (data_size < 512) {
                    // send ack
                    connections.send(connectionId, createSpecific_ack_packet(blockNumberFromClient));

                    // BROADCAST AND UPDATE LIST OF FILES
                    handler.ListAllFiles.add(convertToString(filename));
                    Bcast(brodcast_flag, convertToString(filename));

                    // close
                    try {
                        destStream.close();
                        break;
                    } catch (IOException e) {
                    }

                } else {
                    connections.send(connectionId, createSpecific_ack_packet(blockNumberFromClient));
                }
                break;

            case 4:
                // ACK 
                System.out.println("im in ack");
                if (EnteredFirstPacket) {
                    byte[] ackNumber = new byte[2];
                    ackNumber[0] = message[2];
                    ackNumber[1] = message[3];
                    short ack_short = (short) (((short) ackNumber[0]) << 8 | (short) (ackNumber[1]) & 0x00ff);

                    // ack+1 is the next blocking number
                    System.out.println("finished data is "+lastData);
                    if(!lastData){
                        short next_block_short = (short) (ack_short + 1);
                        connections.send(connectionId, createDataPacket(fileInBytes, 512*ack_short, next_block_short));
                    }
                    else{
                        lastData=false;
                    }

                   
                } else {
                    notLoggedInError();
                }
                break;

            case 6:
                // DIRQ
                if (EnteredFirstPacket) {
                    LinkedList<Byte> filenames = new LinkedList<>();

                    short blockDir = 1;
                    
                    // get all filesnames
                    for (String file : handler.ListAllFiles) {
                            byte[] byteFileName = encodeString(file);
                            for(byte b : byteFileName ){
                                filenames.add(b);
                            }
                            filenames.add((byte)0);
                        
                    }
                    if(filenames.size()!=0)
                        filenames.remove(filenames.size()-1);

                    //convert linked list to bytes
                    byte[] listOfFileNames = new byte[filenames.size()];

                    for(int i=0;i<filenames.size();i++)
                    {
                        listOfFileNames[i] = filenames.get(i);
                    }
                    
                    connections.send(connectionId,createDataPacket(listOfFileNames, 0, blockDir));
                
                } else {
                    notLoggedInError();
                }
                break;

            case 7:
            //LOGRQ
                if (!EnteredFirstPacket) {
                    usernameBytes = new byte[message.length];
                    int j = 0;
                    for (int i = 2; i < message.length; i++) {
                        usernameBytes[j] = message[i];
                        j = j + 1;
                    }
                    String new_username = convertToString(usernameBytes);
            
                    for (String user : handler.logged_User.keySet()) {
                        if (new_username.equals(user)) {
                            already_loggedIn();
                            break;
                        }

                    }
                    if (!login_error) {
                        this.user = new_username;
                        // Changed HashMap
                        handler.logged_User.put(user, connectionId);
                        // Changed HashMap
                        EnteredFirstPacket = true;

                        connections.send(connectionId, createAckPacket());
                    }

                    login_error = false;

                } else {
                    already_loggedIn();
                    login_error = false;
                }

                break;

            case 8:
                // DELRQ
                // save the filename
                if (EnteredFirstPacket) {
                    int l = 0;
                    byte brd_flag = 0;
                    filename = new byte[message.length-2];
                    for (int i = 2; i < message.length; i++) {
                        filename[l] = message[i];
                        l = l + 1;
                    }

                    String fileToDelete = convertToString(filename);
                    File FilePathToDelete = new File("./Flies/" + fileToDelete);
                    if (FilePathToDelete.exists()) {
                        // delete the file
                        FilePathToDelete.delete();
                        // BROADCAST AND UPDATE LIST FILES
                        handler.ListAllFiles.remove(fileToDelete);
                        Bcast(brd_flag, fileToDelete);
                        // send ack - the file has been deleted
                        connections.send(connectionId, createAckPacket());

                    } else {
                        fileNotFound();
                    }
                } else {
                    notLoggedInError();
                }
                break;

            case 10:
                // DISC
                if (EnteredFirstPacket) { // checking if the user is logged in which is the first packet
                    connections.send(connectionId, createAckPacket());
                    this.shouldTerminate = true;
                    handler.logged_User.remove(user);
                    shouldTerminate();
                    break;
                } else {
                    connections.send(connectionId, createAckPacket());
                    this.shouldTerminate = true;
                    shouldTerminate();
                }
                break;
        }
    }

    public byte[] createAckPacket() {
        // ack packet
        byte[] ackPacket = new byte[4];
        ackPacket[0] = 0;
        ackPacket[1] = 4;
        ackPacket[2] = 0;
        ackPacket[3] = 0;
        return ackPacket;

    }

    public byte[] createSpecific_ack_packet(byte[] ack) {
        // ack packet
        byte[] ackPacket = new byte[4];
        ackPacket[0] = 0;
        ackPacket[1] = 4;
        ackPacket[2] = ack[0];
        ackPacket[3] = ack[1];
        return ackPacket;

    }

    public byte[] createDataPacket(byte[] message, int position, short block) {
        int packetSize = message.length;
        System.out.println("packet size is  "+packetSize);
        int k = position;

        if(packetSize-k<512){
            this.lastData=true;
            System.out.println("ps "+packetSize);
            System.out.println("k here "+k);
            System.out.println("FINISHED");

            packetSize=packetSize-k;
            System.out.println("ps AFTER "+packetSize);
            
        }

        else if (message.length > 512) {
            packetSize = 512; // limit of packet size
        }
        else{
            lastData=true;
        }

        // finished sending all data - return empty data packet
        // this is the sign for the client that server sent all packets
        if (k == message.length-1) {
            packetSize = 0;
            fileInBytes = new byte[] {};
        }


        byte[] sizePacket_bytes = new byte[] { (byte) (packetSize >> 8), (byte) (packetSize & 0xff) };
        byte[] blockingNumber_bytes = new byte[] { (byte) (block >> 8), (byte) (block & 0xff) };

        byte[] dataPacket = new byte[packetSize+6];
        dataPacket[0] = (byte)0;
        dataPacket[1] = (byte)3;
        dataPacket[2] = sizePacket_bytes[0];
        dataPacket[3] = sizePacket_bytes[1];
        dataPacket[4] = blockingNumber_bytes[0];
        dataPacket[5] = blockingNumber_bytes[1];

        
        for (int i = 6; i < packetSize+6; i++) {
            System.out.println("i "+i);
            System.out.println("k "+k);
            dataPacket[i] = message[k];
            k = k + 1;
        }

        return dataPacket;

    }

    @Override
    public boolean shouldTerminate() {
        if (shouldTerminate) {
            this.connections.disconnect(this.connectionId);
            return true;
        }
        return false;
    }

    private String convertToString(byte[] msg) {
        String result = new String(msg, 0, msg.length, StandardCharsets.UTF_8);
        return result;
    }

    public void Bcast(byte DeletedOrAdded, String filename) {
        byte[] fileName = encodeString(filename);
        byte[] BcastPacket = new byte[4 + fileName.length];
        BcastPacket[0] = (byte)0;
        BcastPacket[1] = (byte)9;
        BcastPacket[2] = DeletedOrAdded;
        for (int i = 0; i < fileName.length; i++) {
            BcastPacket[i + 3] = fileName[i];
        }
        BcastPacket[3 + fileName.length] = 0;
        for (String user : handler.logged_User.keySet()) {
            if (!user.equals(user)) {
                connections.send(handler.logged_User.get(user), BcastPacket);
            }

        }
    }

    private void already_loggedIn() {
        String error = ErrorMsg.USER_ALREADY_LOGGED_IN.getMessage();
        byte[] Errormsg = encodeString(error);
        byte[] ErrorPacket = new byte[5 + Errormsg.length];
        ErrorPacket[0] = (byte)0;
        ErrorPacket[1] = (byte)5;
        ErrorPacket[2] = (byte)0;
        ErrorPacket[3] = (byte)7;
        for (int i = 0; i < Errormsg.length; i++) {
            ErrorPacket[i + 4] = Errormsg[i];
        }
        ErrorPacket[4 + Errormsg.length] = 0;
        login_error = true;
        connections.send(connectionId, ErrorPacket);
    }

    public void notLoggedInError() {
        String error = ErrorMsg.USER_NOT_LOGGED_IN.getMessage();
        byte[] Errormsg = encodeString(error);
        byte[] ErrorPacket = new byte[5 + Errormsg.length];
        ErrorPacket[0] = (byte)0;
        ErrorPacket[1] = (byte)5;
        ErrorPacket[2] = (byte)0;
        ErrorPacket[3] = (byte)6;
        for (int i = 0; i < Errormsg.length; i++) {
            ErrorPacket[i + 4] = Errormsg[i];
        }
        ErrorPacket[4 + Errormsg.length] = 0;
        connections.send(connectionId, ErrorPacket);

    }

    public void fileNotFound() {
        String error = ErrorMsg.FILE_NOT_FOUND.getMessage();
        byte[] Errormsg = encodeString(error);
        byte[] ErrorPacket = new byte[5 + Errormsg.length];
        ErrorPacket[0] = (byte)0;
        ErrorPacket[1] = (byte)5;
        ErrorPacket[2] = (byte)0;
        ErrorPacket[3] = (byte)1;
        for (int i = 0; i < Errormsg.length; i++) {
            ErrorPacket[i + 4] = Errormsg[i];
        }
        ErrorPacket[4 + Errormsg.length] = 0;
        connections.send(connectionId, ErrorPacket);

    }

    private byte[] encodeString(String message) {
        return message.getBytes(StandardCharsets.UTF_8);

    }

    public void file_exist() {
        String error = ErrorMsg.FILE_ALREADY_EXISTS.getMessage();
        byte[] Errormsg = encodeString(error);
        byte[] ErrorPacket = new byte[5 + Errormsg.length];
        ErrorPacket[0] = (byte)0;
        ErrorPacket[1] = (byte)5;
        ErrorPacket[2] = (byte)0;
        ErrorPacket[3] = (byte)5;
        for (int i = 0; i < Errormsg.length; i++) {
            ErrorPacket[i + 4] = Errormsg[i];
        }
        ErrorPacket[4 + Errormsg.length] = 0;
        connections.send(connectionId, ErrorPacket);

    }
}