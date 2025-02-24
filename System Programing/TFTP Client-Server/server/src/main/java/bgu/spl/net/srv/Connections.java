package bgu.spl.net.srv;

public interface Connections<T> {

    void connect(int connectionId, ConnectionHandler<byte[]> handler);

    boolean send(int connectionId, byte[] msg);

    void disconnect(int connectionId);

}
