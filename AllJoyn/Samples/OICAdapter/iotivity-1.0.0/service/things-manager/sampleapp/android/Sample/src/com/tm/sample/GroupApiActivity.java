/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/
package com.tm.sample;

import java.util.ArrayList;
import java.util.Calendar;

import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.DatePicker;
import android.widget.DatePicker.OnDateChangedListener;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TimePicker;
import android.widget.TimePicker.OnTimeChangedListener;
import android.widget.Toast;

/*
 * Activity for handling user's selection on UI for GroupApis.
 * & for updating UI.
 */
public class GroupApiActivity extends Activity {

    private ListView                list;
    private Button                  findGroup;
    private ArrayAdapter<String>    groupApis;
    private ArrayList<String>       groupApisList;
    private static GroupApiActivity groupApiActivityObj;
    private GroupClient             groupClientObj;
    private static Handler          mHandler;

    private static EditText         logs;
    private static String           logMessage;

    // For Scheduled ActionSet
    public static Context           mcontext;
    public static Calendar          scheduleTime;

    private final String            LOG_TAG = "[TMSample] "
                                                    + this.getClass()
                                                            .getSimpleName(); ;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.groupapis);

        groupApiActivityObj = this;
        mcontext = this;

        groupClientObj = new GroupClient();
        groupApisList = new ArrayList<String>();
        list = (ListView) findViewById(R.id.groupaApiList);
        findGroup = (Button) findViewById(R.id.button1);
        logs = (EditText) findViewById(R.id.EditText);

        // adding the item to list that will be displayed on the UI.
        groupApisList.add("1. Create ActionSet (ALLBULBON & ALLBULBOFF)");
        groupApisList.add("2. Execute ActionSet (ALLBULBON)");
        groupApisList.add("3. Execute ActionSet (AllBULBOFF)");

        // Recursive GroupAction
        groupApisList.add("4. Create ActionSet (Recursive_ALLBULBON)");
        groupApisList.add("      4.1 Execute ActionSet");
        groupApisList.add("      4.2 Cancel ActionSet");

        // scheduled GroupAction
        groupApisList.add("5. Create ActionSet (Scheduled_ALLBULBOFF)");
        groupApisList.add("      5.1 Execute ActionSet");
        groupApisList.add("      5.2 Cancel ActionSet");

        groupApisList.add("6. Get ActionSet(ALLBULBOFF)");
        groupApisList.add("7. Delete ActionSet(ALLBULBOFF)");
        groupApisList.add("8. Find BookMark to Observe");

        // handler for updating the UI i.e. MessageLog (TextBox) & ListView
        mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case 0:
                        groupApis = new ArrayAdapter<String>(
                                groupApiActivityObj,
                                android.R.layout.simple_list_item_1,
                                groupApisList);
                        list.setAdapter(groupApis);
                        list.bringToFront();
                        break;
                    case 1:
                        logs.setText("");
                        logs.setText(logMessage);
                        Log.i(LOG_TAG, logMessage);
                }
            }
        };
        setHandler(mHandler);

        // find group Button Listener
        findGroup.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                groupClientObj.findGroup();
            }
        });

        // Listener for item clicked by the user on the UI
        list.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view,
                    int position, long id) {
                if (position == 0) {
                    groupClientObj.createActionSetBulbOn();
                    groupClientObj.createActionSetBulbOff();
                } else if (position == 1) {
                    groupClientObj.executeActionSetBulbOn(0);
                } else if (position == 2) {
                    groupClientObj.executeActionSetBulbOff(0);
                } else if (position == 3) {
                    groupClientObj.createRecursiveActionSetBulbOn();
                } else if (position == 4) {
                    groupClientObj.executeRecursiveActionSetBulbOn(0);
                } else if (position == 5) {
                    groupClientObj.cancelRecursiveActionSetBulbOn();
                } else if (position == 6) {
                    showDateAndTimeDialog();
                } else if (position == 7) {
                    groupClientObj.executeScheduledActionSetBulbOff(0);
                } else if (position == 8) {
                    groupClientObj.cancelScheduledActionSetBulbOff();
                } else if (position == 9) {
                    groupClientObj.getActionSetBulbOff();
                } else if (position == 10) {
                    groupClientObj.deleteActionSetBulbOff();
                } else if (position == 11) {
                    groupClientObj.findBookMarkResources();
                }
            }
        });

        // creating group and find light resources
        groupClientObj.createGroup();
        groupClientObj.findLightResources();
    }

    public void showDateAndTimeDialog() {
        // for scheduled actionSet
        scheduleTime = Calendar.getInstance();

        final Dialog dialog = new Dialog(mcontext);
        dialog.setContentView(R.layout.custom_dialog);
        dialog.setTitle("Choose date and time for Secheduling");

        TimePicker tp = (TimePicker) dialog.findViewById(R.id.timePicker1);
        DatePicker dp = (DatePicker) dialog.findViewById(R.id.datePicker1);
        Button ok = (Button) dialog.findViewById(R.id.ok);
        Button cancel = (Button) dialog.findViewById(R.id.cancel);

        dialog.setCancelable(false);
        dialog.show();
        tp.setOnTimeChangedListener(new OnTimeChangedListener() {
            @Override
            public void onTimeChanged(TimePicker view, int hourOfDay, int minute) {
                scheduleTime.set(Calendar.HOUR_OF_DAY, hourOfDay);
                scheduleTime.set(Calendar.MINUTE, minute);
            }
        });
        dp.init(dp.getYear(), dp.getMonth(), dp.getDayOfMonth(),
                new OnDateChangedListener() {
                    @Override
                    public void onDateChanged(DatePicker arg0, int arg1,
                            int arg2, int arg3) {
                        scheduleTime.set(arg1, arg2, arg3);
                    }
                });
        ok.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                dialog.dismiss();

                // Calculate the time difference in delay
                Calendar localTime = Calendar.getInstance();
                if (scheduleTime.compareTo(localTime) != 1) {
                    groupApiActivityObj
                            .displayToastMessage("Invalid set time!");
                    return;
                }

                long delay = scheduleTime.getTimeInMillis()
                        - localTime.getTimeInMillis();
                delay /= 1000;

                groupClientObj.createScheduledActionSetBulbOff(delay);
            }
        });
        cancel.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                dialog.dismiss();
            }
        });
    }

    public static void setMessageLog(String message) {
        logMessage = message;
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
    }

    // for update UI these functions will be called from GroupClient Class
    public static GroupApiActivity getGroupApiActivityObj() {
        return groupApiActivityObj;
    }

    public Handler getHandler() {
        return mHandler;
    }

    public void setHandler(Handler mHandler) {
        GroupApiActivity.mHandler = mHandler;
    }

    public void displayToastMessage(String message) {
        Toast toast = Toast.makeText(this, message, Toast.LENGTH_SHORT);
        toast.show();
        Log.i(LOG_TAG, message);
    }
}
