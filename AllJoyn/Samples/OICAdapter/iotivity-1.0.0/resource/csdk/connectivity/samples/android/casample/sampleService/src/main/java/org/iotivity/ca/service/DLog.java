
package org.iotivity.ca.service;

import android.os.Handler;
import android.widget.TextView;

public final class DLog {

    private final static String MAIN_TAG = "Sample_Service : DLog";

    private static TextView mLogView = null;

    private static Handler mHandler = null;

    public static void setTextView(Handler handler, TextView logView) {
        mHandler = handler;
        mLogView = logView;
    }

    private static void addLogText(final String msg) {

        mHandler.post(new Runnable() {

            @Override
            public void run() {

                if (mLogView == null)
                    return;

                StringBuilder builder = new StringBuilder(mLogView.getText());
                // add front
                builder.append(msg + "\n");

                mLogView.setText(builder.toString());
            }

        });

    }

    public static final void v(String className, String msg) {
        android.util.Log.v(MAIN_TAG, className + "." + msg);

        if (mLogView != null) {
            addLogText(msg);
        }
    }

    public static final void d(String className, String msg) {
        android.util.Log.d(MAIN_TAG, className + "." + msg);

        if (mLogView != null) {
            addLogText(msg);
        }
    }

    public static final void i(String className, String msg) {
        android.util.Log.i(MAIN_TAG, className + "." + msg);

        if (mLogView != null) {
            addLogText(msg);
        }
    }

    public static final void w(String className, String msg) {
        android.util.Log.w(MAIN_TAG, className + "." + msg);

        if (mLogView != null) {
            addLogText(msg);
        }
    }

    public static final void w(String className, String msg, Exception ex) {
        android.util.Log.w(MAIN_TAG, className + "." + msg + ":" + ex.getMessage());

        if (mLogView != null) {
            addLogText(msg);
        }
    }

    public static final void w(String className, String msg, Error e) {
        android.util.Log.w(MAIN_TAG, className + "." + msg + ":" + e.getMessage());

        if (mLogView != null) {
            addLogText(msg);
        }
    }

}

