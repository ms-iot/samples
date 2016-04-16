package org.iotivity.service.sample.client;

import android.content.Context;
import android.util.Log;
import android.widget.Toast;

import org.iotivity.service.RcsException;
import org.iotivity.service.client.RcsRemoteResourceObject;

public class Utils {
    public static String resourceInfo(RcsRemoteResourceObject resourceObject)
            throws RcsException {
        StringBuilder sb = new StringBuilder();

        sb.append("URI : " + resourceObject.getUri() + "\n");
        sb.append("Host : " + resourceObject.getAddress() + "\n");
        for (String type : resourceObject.getTypes()) {
            sb.append("resourceType : " + type + "\n");
        }

        for (String itf : resourceObject.getInterfaces()) {
            sb.append("resourceInterfaces : " + itf + "\n");
        }

        sb.append("isObservable : " + resourceObject.isObservable() + "\n");

        return sb.toString();
    }

    public static void showError(Context ctx, String tag, String msg) {
        Toast.makeText(ctx, msg, Toast.LENGTH_SHORT).show();
        Log.e(tag, msg);
    }

    public static void showError(Context ctx, String tag, Exception e) {
        Toast.makeText(ctx, e.getMessage(), Toast.LENGTH_SHORT).show();
        Log.e(tag, e.getMessage(), e);
    }
}
