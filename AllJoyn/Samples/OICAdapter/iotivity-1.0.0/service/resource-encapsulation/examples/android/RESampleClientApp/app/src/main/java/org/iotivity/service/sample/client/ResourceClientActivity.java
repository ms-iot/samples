/******************************************************************
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 * <p>
 * <p>
 * <p>
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * <p>
 * http://www.apache.org/licenses/LICENSE-2.0
 * <p>
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************/
package org.iotivity.service.sample.client;

import static org.iotivity.service.client.RcsRemoteResourceObject.OnCacheUpdatedListener;
import static org.iotivity.service.client.RcsRemoteResourceObject.OnStateChangedListener;
import static org.iotivity.service.client.RcsRemoteResourceObject.ResourceState;

import java.lang.ref.WeakReference;

import org.iotivity.service.RcsException;
import org.iotivity.service.RcsResourceAttributes;
import org.iotivity.service.RcsValue;
import org.iotivity.service.client.RcsAddress;
import org.iotivity.service.client.RcsDiscoveryManager;
import org.iotivity.service.client.RcsDiscoveryManager.OnResourceDiscoveredListener;
import org.iotivity.service.client.RcsRemoteResourceObject;
import org.iotivity.service.client.RcsRemoteResourceObject.OnRemoteAttributesReceivedListener;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

/*
 * Activity for handling user's selection on UI for Resource Client APIs.
 * & for updating UI.
 */
public class ResourceClientActivity extends Activity
        implements OnItemClickListener {

    private static final String LOG_TAG = ResourceClientActivity.class
            .getSimpleName();

    private static final int MSG_ID_RESOURCE_DISCOVERED = 0;
    private static final int MSG_ID_ATTRIBUTE_RECEIVED  = 1;
    private static final int MSG_ID_PRINT_LOG           = 2;

    private static final String ATTR_KEY_TEMPERATURE = "Temperature";

    private TextView mLogView;
    private ListView mListView;
    private Button   mDiscoveryBtn;

    private Handler            mHandler;
    private ArrayAdapter<Item> mItemAdapter;

    private RcsDiscoveryManager.DiscoveryTask mDiscoveryTask;
    private RcsRemoteResourceObject           mResourceObj;

    private OnResourceDiscoveredListener mOnResourceDiscoveredListener = new OnResourceDiscoveredListener() {

        @Override
        public void onResourceDiscovered(
                RcsRemoteResourceObject foundResource) {
            Log.i(LOG_TAG, "onResourceDiscovered");

            mHandler.obtainMessage(MSG_ID_RESOURCE_DISCOVERED, foundResource)
                    .sendToTarget();
        }
    };

    private OnStateChangedListener mOnStateChangedListener = new OnStateChangedListener() {

        @Override
        public void onStateChanged(ResourceState resourceState) {
            Log.i(LOG_TAG, "onStateChanged");

            mHandler.obtainMessage(MSG_ID_PRINT_LOG,
                    "Current Resource State : " + resourceState);
        }
    };

    private OnRemoteAttributesReceivedListener mOnRemoteAttributesReceivedListener = new OnRemoteAttributesReceivedListener() {
        @Override
        public void onAttributesReceived(RcsResourceAttributes attrs,
                int eCode) {
            Log.i(LOG_TAG, "onAttributesReceived");

            mHandler.obtainMessage(MSG_ID_ATTRIBUTE_RECEIVED, attrs)
                    .sendToTarget();
        }
    };

    private OnCacheUpdatedListener mOnCacheUpdatedListener = new OnCacheUpdatedListener() {
        @Override
        public void onCacheUpdated(RcsResourceAttributes attrs) {
            Log.i(LOG_TAG, "onCacheUpdated");

            mHandler.obtainMessage(MSG_ID_ATTRIBUTE_RECEIVED, attrs)
                    .sendToTarget();
        }
    };

    private Item mStartMonitoring = new Item("1. Start Monitoring") {
        @Override
        public void execute() throws RcsException {
            if (mResourceObj.isMonitoring()) {
                printLog("Monitoring already started");
                return;
            }

            mResourceObj.startMonitoring(mOnStateChangedListener);
        }
    };

    private Item mStopMonitoring = new Item("2. Stop Monitoring") {
        @Override
        public void execute() throws RcsException {
            if (mResourceObj.isMonitoring()) {
                mResourceObj.stopMonitoring();
                printLog("Stopped Resource Monitoring");
            } else {
                printLog("Monitoring not started");
            }
        }
    };

    private Item mGetRemoteAttributes = new Item("3. Get Remote Attributes") {
        @Override
        public void execute() throws RcsException {
            mResourceObj
                    .getRemoteAttributes(mOnRemoteAttributesReceivedListener);
        }
    };

    private Item mSetRemoteAttributes = new Item("4. Set Remote Attributes") {

        @Override
        public void execute() throws RcsException {
            showInputValueDialog();
        }
    };

    private Item mStartCaching = new Item("5. Start Caching") {
        @Override
        public void execute() throws RcsException {
            if (mResourceObj.isCaching()) {
                printLog("Caching already started");
                return;
            }

            mResourceObj.startCaching(mOnCacheUpdatedListener);
        }
    };

    private Item mGetCacheState = new Item("6. Get Cache State") {
        @Override
        public void execute() throws RcsException {
            printLog("Cache State : " + mResourceObj.getCacheState());
        }
    };

    private Item mGetCachedAttributes = new Item(
            "7. Get All Cached Attributes") {
        @Override
        public void execute() throws RcsException {
            printAttributes(mResourceObj.getCachedAttributes());
        }
    };

    private Item mGetCachedAttribute = new Item("8. Get Cached Attribute") {
        @Override
        public void execute() throws RcsException {
            printLog(ATTR_KEY_TEMPERATURE + " : " + mResourceObj
                    .getCachedAttribute(ATTR_KEY_TEMPERATURE).asInt());
        }
    };

    private Item mStopCaching = new Item("9. Stop Caching") {
        @Override
        public void execute() throws RcsException {
            mResourceObj.stopCaching();
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_resource_client);

        mListView = (ListView) findViewById(R.id.list_menu);
        mLogView = (TextView) findViewById(R.id.text_log);
        mDiscoveryBtn = (Button) findViewById(R.id.btn_discovery);

        mHandler = new ClientHandler(this);

        initMenuList();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (mDiscoveryTask != null)
            mDiscoveryTask.cancel();
        if (mResourceObj != null)
            mResourceObj.destroy();
    }

    private void initMenuList() {
        Item[] items = new Item[] { mStartMonitoring, mStopMonitoring,
                mGetRemoteAttributes, mSetRemoteAttributes, mStartCaching,
                mGetCacheState, mGetCachedAttributes, mGetCachedAttribute,
                mStopCaching };

        mItemAdapter = new ArrayAdapter<>(this,
                android.R.layout.simple_list_item_1, items);

        mListView.setAdapter(mItemAdapter);

        mListView.setOnItemClickListener(this);
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        if (mResourceObj == null) {
            showError("no discovered RemoteResourceObject");
            return;
        }

        try {
            mItemAdapter.getItem(position).execute();
        } catch (RcsException e) {
            showError(e);
        }
    }

    public void onDiscoverResourceClick(View v) {
        toggleDiscovery();
    }

    private void toggleDiscovery() {
        if (mDiscoveryTask == null) {
            try {
                mDiscoveryTask = RcsDiscoveryManager.getInstance()
                        .discoverResource(RcsAddress.multicast(),
                                mOnResourceDiscoveredListener);
                mDiscoveryBtn.setText(R.string.cancel_discovery);

                mListView.setVisibility(View.INVISIBLE);

                if (mResourceObj != null) {
                    mResourceObj.destroy();
                    mResourceObj = null;
                }
            } catch (RcsException e) {
                showError(e);
            }
        } else {
            mDiscoveryTask.cancel();
            mDiscoveryTask = null;

            mDiscoveryBtn.setText(R.string.discover_resource);
        }
    }

    private void printAttributes(RcsResourceAttributes attributes) {
        try {
            StringBuilder sb = new StringBuilder();
            for (String key : attributes.keySet()) {
                sb.append(key + " : " + attributes.get(key));
            }
            printLog(sb.toString());
        } catch (Exception e) {
            printLog(e);
        }
    }

    private void setRemoteResourceObject(
            RcsRemoteResourceObject foundResource) {
        if (mResourceObj != null) {
            Log.w(LOG_TAG, "Another remote resource found...");
            return;
        }

        mResourceObj = foundResource;

        mListView.setVisibility(View.VISIBLE);
        toggleDiscovery();

        try {
            printLog(resourceInfo(mResourceObj));
        } catch (RcsException e) {
            showError(e);
        }
    }

    private void showInputValueDialog() {
        final AlertDialog dialog = new AlertDialog.Builder(this)
                .setTitle("Enter the Temperature Value")
                .setView(R.layout.dialog_content_edit_text)
                .setNegativeButton("Cancel", null).create();

        dialog.setButton(DialogInterface.BUTTON_POSITIVE, "OK",
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface,
                            int which) {

                        EditText temperatureValue = (EditText) dialog
                                .findViewById(R.id.attributeValue);

                        try {
                            RcsValue value = new RcsValue(Integer.parseInt(
                                    temperatureValue.getText().toString()));

                            RcsResourceAttributes attrs = new RcsResourceAttributes();
                            attrs.put(ATTR_KEY_TEMPERATURE, value);

                            mResourceObj.setRemoteAttributes(attrs,
                                    mOnRemoteAttributesReceivedListener);
                        } catch (NumberFormatException e) {
                            showError("Please enter the Integer Value");
                        } catch (RcsException e) {
                            showError(e);
                        }
                    }
                });
        dialog.show();
    }

    private void showError(String msg) {
        Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
        Log.e(LOG_TAG, msg);
    }

    private void showError(Exception e) {
        Toast.makeText(this, e.getMessage(), Toast.LENGTH_SHORT).show();
        Log.e(LOG_TAG, e.getMessage(), e);
    }

    private void printLog(String message) {
        Log.i(LOG_TAG, message);
        mLogView.setText(message);
    }

    private void printLog(Exception e) {
        Log.i(LOG_TAG, e.getMessage(), e);
        mLogView.setText(e.getMessage());
    }

    private String resourceInfo(RcsRemoteResourceObject resourceObject)
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

    private static abstract class Item {
        private final String mTitle;

        protected Item(String title) {
            mTitle = title;
        }

        @Override
        public String toString() {
            return mTitle;
        }

        public abstract void execute() throws RcsException;
    }

    private static class ClientHandler extends Handler {
        private WeakReference<ResourceClientActivity> mActivityRef;

        private ClientHandler(ResourceClientActivity activity) {
            mActivityRef = new WeakReference<>(activity);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            ResourceClientActivity activity = mActivityRef.get();
            if (activity == null)
                return;

            switch (msg.what) {
                case MSG_ID_RESOURCE_DISCOVERED:
                    activity.setRemoteResourceObject(
                            (RcsRemoteResourceObject) msg.obj);
                    break;

                case MSG_ID_ATTRIBUTE_RECEIVED:
                    activity.printAttributes((RcsResourceAttributes) msg.obj);
                    break;

                case MSG_ID_PRINT_LOG:
                    activity.printLog(msg.obj.toString());
                    break;
            }
        }
    }
}
