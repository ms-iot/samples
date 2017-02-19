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

package org.iotivity.service.sample.server;

import java.lang.ref.WeakReference;

import org.iotivity.service.RcsException;
import org.iotivity.service.RcsResourceAttributes;
import org.iotivity.service.RcsValue;
import org.iotivity.service.server.RcsGetResponse;
import org.iotivity.service.server.RcsRequest;
import org.iotivity.service.server.RcsResourceObject;
import org.iotivity.service.server.RcsResourceObject.GetRequestHandler;
import org.iotivity.service.server.RcsResourceObject.OnAttributeUpdatedListener;
import org.iotivity.service.server.RcsResourceObject.SetRequestHandler;
import org.iotivity.service.server.RcsSetResponse;

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

public class CustomServerActivity extends Activity
        implements OnItemClickListener {
    private static final String LOG_TAG = CustomServerActivity.class
            .getSimpleName();

    private static final int MSG_ID_PRINT_LOG = 0;

    private Handler mHandler;

    private ListView           mListView;
    private TextView           mLogView;
    private ArrayAdapter<Item> mItemAdapter;

    private RcsResourceObject mResourceObject;

    private GetRequestHandler mGetRequestHandler = new GetRequestHandler() {
        @Override
        public RcsGetResponse onGetRequested(RcsRequest request,
                RcsResourceAttributes attrs) {
            mHandler.obtainMessage(MSG_ID_PRINT_LOG,
                    "Got a Get request from client, send default response \n"
                            + "URI : " + request.getResourceUri() + "\n")
                    .sendToTarget();

            return RcsGetResponse.defaultAction();
        }
    };

    private Item mGetRequestHandlerItem = new ToggleItem(
            R.string.register_get_request_handler,
            R.string.unregister_get_request_handler) {

        @Override
        public void execute() throws RcsException {
            if (isChecked()) {
                mResourceObject.setGetRequestHandler(null);
            } else {
                mResourceObject.setGetRequestHandler(mGetRequestHandler);
            }
            toggle();
        }
    };

    private SetRequestHandler mSetRequestHandler = new SetRequestHandler() {
        @Override
        public RcsSetResponse onSetRequested(RcsRequest request,
                RcsResourceAttributes attrs) {
            mHandler.obtainMessage(MSG_ID_PRINT_LOG,
                    "Got a Set request from client, send default response\n"
                            + "URI : " + request.getResourceUri() + "\n")
                    .sendToTarget();

            return RcsSetResponse.defaultAction();
        }
    };

    private Item mSetRequestHandlerItem = new ToggleItem(
            R.string.register_set_request_handler,
            R.string.unregister_set_request_handler) {

        @Override
        public void execute() throws RcsException {
            if (isChecked()) {
                mResourceObject.setSetRequestHandler(null);
            } else {
                mResourceObject.setSetRequestHandler(mSetRequestHandler);
            }
            toggle();
        }
    };

    private OnAttributeUpdatedListener mOnAttributeUpdatedListener = new OnAttributeUpdatedListener() {
        @Override
        public void onAttributeUpdated(RcsValue oldValue, RcsValue newValue) {
            mHandler.obtainMessage(MSG_ID_PRINT_LOG,
                    "attributes updated\n" + "oldValue : " + oldValue
                            + ",  newValue : " + newValue + "\n")
                    .sendToTarget();
        }
    };

    private Item mAttributeUpdatedListenerItem = new ToggleItem(
            R.string.register_attribute_updated_listener,
            R.string.unregister_attribute_updated_listener) {

        @Override
        public void execute() throws RcsException {
            if (isChecked()) {
                mResourceObject.removeAttributeUpdatedListener(
                        ResourceProperties.ATTR_KEY_TEMPERATURE);
            } else {
                mResourceObject.addAttributeUpdatedListener(
                        ResourceProperties.ATTR_KEY_TEMPERATURE,
                        mOnAttributeUpdatedListener);
            }
            toggle();
        }
    };
    private Item mSetTempItem = new Item() {

        @Override
        public String toString() {
            return getString(R.string.set_temp);
        }

        @Override
        public void execute() throws RcsException {
            showInputValueDialog();
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_custom_server);

        mHandler = new LocalHandler(this);

        mLogView = (TextView) findViewById(R.id.text_log);
        mListView = (ListView) findViewById(R.id.list_menu);

        initMenuList();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (mResourceObject != null)
            mResourceObject.destroy();
    }

    private void initMenuList() {
        // the items that will be displayed on the UI.
        Item[] items = new Item[] { mSetTempItem, mGetRequestHandlerItem,
                mSetRequestHandlerItem, mAttributeUpdatedListenerItem };

        mItemAdapter = new ArrayAdapter<>(this,
                android.R.layout.simple_list_item_1, items);
        mListView.setAdapter(mItemAdapter);

        mListView.setOnItemClickListener(this);
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        try {
            mItemAdapter.getItem(position).execute();
            mItemAdapter.notifyDataSetChanged();
        } catch (RcsException e) {
            showError(e.getMessage());
        }
    }

    public void onStartServerClick(View v) {
        Button btn = (Button) v;
        if (mResourceObject == null) {
            RcsResourceAttributes attrs = new RcsResourceAttributes();
            attrs.put(ResourceProperties.ATTR_KEY_TEMPERATURE, 10);

            mResourceObject = new RcsResourceObject.Builder(
                    ResourceProperties.URI, ResourceProperties.TYPE,
                    ResourceProperties.INTERFACE).setAttributes(attrs).build();

            btn.setText(R.string.stop_server);
            mListView.setVisibility(View.VISIBLE);
            mLogView.setText("Resource created successfully");
        } else {
            mResourceObject.destroy();
            mResourceObject = null;

            btn.setText(R.string.start_server);
            mListView.setVisibility(View.INVISIBLE);
            mLogView.setText("Resource stopped successfully");
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

                        EditText editText = (EditText) dialog
                                .findViewById(R.id.edit_text);

                        try {
                            RcsValue value = new RcsValue(Integer
                                    .parseInt(editText.getText().toString()));

                            mResourceObject.setAttribute(
                                    ResourceProperties.ATTR_KEY_TEMPERATURE,
                                    value);
                            printLog(
                                    "Attribute set successfully\nTemperature : "
                                            + value);
                        } catch (NumberFormatException e) {
                            showError("Please enter the Integer Value");
                        } catch (RcsException e) {
                            printLog(e);
                        }
                    }
                });
        dialog.show();
    }

    private void showError(String message) {
        Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
        Log.i(LOG_TAG, message);
    }

    private void printLog(String msg) {
        mLogView.setText(msg);
        Log.i(LOG_TAG, msg);
    }

    private void printLog(Exception e) {
        mLogView.setText(e.getMessage());
        Log.i(LOG_TAG, e.getMessage(), e);
    }

    private interface Item {
        void execute() throws RcsException;
    }

    private static class LocalHandler extends Handler {
        private WeakReference<CustomServerActivity> mActivityRef;

        private LocalHandler(CustomServerActivity activity) {
            mActivityRef = new WeakReference<>(activity);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);

            CustomServerActivity activity = mActivityRef.get();
            if (activity == null)
                return;

            switch (msg.what) {
                case MSG_ID_PRINT_LOG:
                    activity.printLog(msg.obj.toString());
                    break;
            }
        }
    }

    private abstract class ToggleItem implements Item {
        private int mUncheckedStrId;
        private int mCheckedStrId;

        private boolean mIsChecked;

        public ToggleItem(int uncheckedStrId, int checkedStrId) {
            mUncheckedStrId = uncheckedStrId;
            mCheckedStrId = checkedStrId;
        }

        @Override
        public String toString() {
            return getString(mIsChecked ? mCheckedStrId : mUncheckedStrId);
        }

        protected final boolean isChecked() {
            return mIsChecked;
        }

        public final void toggle() {
            mIsChecked = !mIsChecked;
        }
    }
}
