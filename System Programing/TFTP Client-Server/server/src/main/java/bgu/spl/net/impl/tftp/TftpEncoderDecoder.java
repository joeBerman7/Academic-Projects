package bgu.spl.net.impl.tftp;
import java.util.ArrayList;
import java.util.List;
import bgu.spl.net.api.MessageEncoderDecoder;

public class TftpEncoderDecoder implements MessageEncoderDecoder<byte[]> {

    private enum DecodeState {
        WAITING_TO_DATA,
        WAITING_TO_DATASIZE,
        WAITING_TO_ZERO,
        WAITING_TO_OPCODE,
        WAITING_TO_BLOCKNUM,
    }

    private DecodeState decodeState = DecodeState.WAITING_TO_OPCODE; //the state right now of the encdec
    private int msgSize = 0 ; //represent only the DATA case
    List<Byte> buffer = new ArrayList<Byte>();

    public byte[] decodeNextByte(byte nextByte) {
        buffer.add(nextByte);
        switch (decodeState) {
            case WAITING_TO_OPCODE:
                if (buffer.size() == 2) {
                    short opcode = bytesToShort(convert(), 0, 1);
                    if (opcode == 6 | opcode == 10) {
                        return msgIsComplete();
                    }
                    else if (opcode == 4) {
                        decodeState = DecodeState.WAITING_TO_BLOCKNUM;
                    }
                    else if (opcode == 3) {
                        decodeState = DecodeState.WAITING_TO_DATASIZE;
                    }
                    else {
                        decodeState = DecodeState.WAITING_TO_ZERO;
                    }
                }
                break;
            case WAITING_TO_BLOCKNUM:
                if (buffer.size() == 4) {
                    return msgIsComplete();
                }
                break;
            case WAITING_TO_DATA:
                if (buffer.size() == msgSize) {
                    msgSize = 0;
                    return msgIsComplete();
                }
                break;
            case WAITING_TO_DATASIZE:
                if (buffer.size() == 4) {
                    decodeState = DecodeState.WAITING_TO_DATA;
                    msgSize = bytesToShort(convert(), 2, 3);
                }
                break;
            case WAITING_TO_ZERO:
                if (nextByte == 0){
                    return msgIsComplete();
                }
                break;
        }
        return null; // if message is not yet complete
    }

    @Override
    public byte[] encode(byte[] message) {
        return message;
    }

    // Convert short to byte 
    public byte[] shortToByte (short a){
        return new byte[]{(byte)(a >> 8), (byte)(a & 0xFF)};
    }
    
    // Convert byte to short
    public short bytesToShort (byte[] bytes, int i, int j){
        return (short)(((short)bytes[i] << 8) | (short)(bytes[j] & 0xFF));
    }

    private byte[] msgIsComplete() {
        byte[] msgComplete = convert();
        decodeState = DecodeState.WAITING_TO_OPCODE;
        buffer.clear();
        return msgComplete;
    }

    private byte[] convert(){
        if (buffer != null) {
            byte[] converted = new byte[buffer.size()];
            int i = 0;
            for (byte b : buffer) {
                converted[i] = b;
                i++;
            }
            return converted;
        }
        return null;
    }
}