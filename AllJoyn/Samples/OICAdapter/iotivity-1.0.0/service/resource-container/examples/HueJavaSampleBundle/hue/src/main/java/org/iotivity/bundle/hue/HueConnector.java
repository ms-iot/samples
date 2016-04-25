package org.iotivity.bundle.hue;

import java.io.IOException;

import org.apache.http.HttpEntity;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.ContentType;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;
import org.iotivity.resourcecontainer.bundle.api.ProtocolBridgeConnector;

public class HueConnector implements ProtocolBridgeConnector {
    public void connect() {

    }

    public void disconnect() {

    }

    public void transmit(String target, String payload) {
        try {
            CloseableHttpClient httpclient = HttpClients.createDefault();
            HttpPost httpPost = new HttpPost(target);
            // httpPost.setHeader("content-type","application/json");
            StringEntity stringEntity = new StringEntity(payload,
                    ContentType.create("application/json", "UTF-8"));
            httpPost.setEntity(stringEntity);

            CloseableHttpResponse response1;
            response1 = httpclient.execute(httpPost);
            // The underlying HTTP connection is still held by the response
            // object
            // to allow the response content to be streamed directly from the
            // network socket.
            // In order to ensure correct deallocation of system resources
            // the user MUST call CloseableHttpResponse#close() from a finally
            // clause.
            // Please note that if response content is not fully consumed the
            // underlying
            // connection cannot be safely re-used and will be shut down and
            // discarded
            // by the connection manager.
            try {
                HttpEntity entity1 = response1.getEntity();
                // do something useful with the response body
                // and ensure it is fully consumed
                EntityUtils.consume(entity1);
            } finally {
                response1.close();
            }

        } catch (ClientProtocolException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    public String read(String target) {
        return "";
    }

}
