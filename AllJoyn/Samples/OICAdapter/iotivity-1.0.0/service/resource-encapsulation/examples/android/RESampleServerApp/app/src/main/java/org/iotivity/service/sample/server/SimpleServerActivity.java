/******************************************************************
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 * <p/>
 * <p/>
 * <p/>
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * <p/>
 * http://www.apache.org/licenses/LICENSE-2.0
 * <p/>
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************/

package org.iotivity.service.sample.server;

import org.iotivity.service.RcsException;
import org.iotivity.service.RcsResourceAttributes;
import org.iotivity.service.RcsValue;
import org.iotivity.service.server.RcsResourceObject;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

/**
 * Activity for handling user's selection on UI for changing temperature. & for
 * updating UI.
 */
public class SimpleServerActivity extends Activity {

    private static final String LOG_TAG = SimpleServerActivity.class
            .getSimpleName();

    private TextView          mLogView;
    private RcsResourceObject mResourceObject;
    private View              mSetTempBtn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_simple_server);

        mLogView = (TextView) findViewById(R.id.text_log);
        mSetTempBtn = findViewById(R.id.btn_set_temp);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (mResourceObject != null)
            mResourceObject.destroy();
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
            mSetTempBtn.setEnabled(true);
        } else {
            mResourceObject.destroy();
            mResourceObject = null;

            btn.setText(R.string.start_server);
            mSetTempBtn.setEnabled(false);
        }
    }

    public void onSetTempClick(View v) {
        showInputValueDialog();
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
        Log.e(LOG_TAG, msg);
        mLogView.setText(msg);
    }

    private void printLog(Exception e) {
        Log.e(LOG_TAG, e.getMessage(), e);
        mLogView.setText(e.getMessage());
    }
}
