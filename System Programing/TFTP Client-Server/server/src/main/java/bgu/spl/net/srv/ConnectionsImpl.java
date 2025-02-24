package bgu.spl.net.srv;
import java.util.concurrent.ConcurrentHashMap;

public class ConnectionsImpl implements Connections<byte[]> {

    public ConcurrentHashMap<Integer,ConnectionHandler<byte[]>> connection_id = new ConcurrentHashMap<>();

    @Override
    public boolean send(int connectionId,byte[] msg) {
        ConnectionHandler<byte[]> handler = connection_id.get(connectionId);
        if(handler!=null){
            handler.send(msg);
            return true;
        }
        return false;
       
    }

    @Override
    public void connect(int connectionId, ConnectionHandler<byte[]> handler){
        connection_id.put(connectionId, handler);

    }
    @Override
    public void disconnect(int connectionId){
        connection_id.remove(connectionId);
        ConnectionHandler<byte[]> handler = connection_id.get(connectionId);
        try {
            handler.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}