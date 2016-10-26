//******************************************************************
//
// Copyright 2014 Intel Corporation.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

package org.iotivity.guiclient;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ExpandableListView;
import android.widget.ProgressBar;

import java.util.ArrayList;
import java.util.List;

import static org.iotivity.guiclient.OcAttributeInfo.OC_ATTRIBUTE_TYPE.LIGHT_DIMMER;
import static org.iotivity.guiclient.OcAttributeInfo.OC_ATTRIBUTE_TYPE.LIGHT_SWITCH;
import static org.iotivity.guiclient.OcAttributeInfo.OC_ATTRIBUTE_TYPE.PLATFORM_LED_SWITCH;
import static org.iotivity.guiclient.R.id.expandableResourceListView;

/**
 * MainActivity instantiates a ExpandableListView of type ExpandableResourceListView, and
 * also creates and starts a OcWorker object to handle the IoTivity specific work.
 *
 * @see org.iotivity.guiclient.OcWorker
 * @see org.iotivity.guiclient.OcWorkerListener
 * @see org.iotivity.guiclient.ExpandableResourceListAdapter
 */
public class MainActivity
        extends ActionBarActivity
        implements OcWorkerListener, View.OnClickListener, ExpandableListView.OnChildClickListener {
    /**
     * Hardcoded TAG... if project never uses proguard then
     * MyOcClient.class.getName() is the better way.
     */
    private static final String TAG = "MainActivity";

    private static final boolean LOCAL_LOGV = true; // set to false to compile out verbose logging

    /**
     * The data structure behind the displayed List of resources and attributes.
     */
    private List<OcResourceInfo> mResourceList;

    /**
     * The custom adapter for displaying the ResourceListItem List
     */
    private ExpandableResourceListAdapter mResourceListAdapter;

    /**
     * The OIC-aware worker class which does all the OIC API interaction
     * and handles the results, notifying MainActivity whenever an event
     * requires a UI update.
     */
    private OcWorker mOcWorker;

    /**
     * Preserve a ref to Action Bar Menu for changing progress icon
     */
    private Menu optionsMenu;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        if (LOCAL_LOGV) Log.v(TAG, "onCreate()");

        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        // start the OcWorker thread and register as a listener
        if(null == this.mOcWorker) {
            this.mOcWorker = new OcWorker(this);
            this.mOcWorker.start(); // configures the OIC platform and wait for further calls
            this.mOcWorker.registerListener(this);
        }

        // init the Resource display list
        if(null == this.mResourceList) {
            this.mResourceList = new ArrayList<>();
        }

        // init the ListView Adapter
        if(null == this.mResourceListAdapter) {
            this.mResourceListAdapter = new ExpandableResourceListAdapter(this.mResourceList,
                    this);
        }

        // init the Expandable List View
        ExpandableListView exListView =
                (ExpandableListView) findViewById(expandableResourceListView);
        exListView.setIndicatorBounds(5, 5);
        exListView.setIndicatorBounds(0, 20);
        exListView.setAdapter(this.mResourceListAdapter);
        exListView.setOnChildClickListener(this);
    }

    @Override
    public void onRestoreInstanceState(Bundle savedInstanceState) {
        if (LOCAL_LOGV) Log.v(TAG, "onRestoreInstanceState()");
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        if (LOCAL_LOGV) Log.v(TAG, "onSaveInstanceState()");
    }

    @Override
    public void onClick(View v) {
        if (LOCAL_LOGV) Log.v(TAG, "onClick()");

        this.setRefreshActionButtonState(false);
    }

    @Override
    public boolean onChildClick(ExpandableListView parent,
                                View v,
                                int groupPosition,
                                int childPosition,
                                long id) {
        if (LOCAL_LOGV) Log.v(TAG, "onChildClick()");

        this.mOcWorker.doGetResource(mResourceList.get(groupPosition));

        return false;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        if (LOCAL_LOGV) Log.v(TAG, "onCreateOptionsMenu()");

        // save a reference for use in controlling refresh icon later
        this.optionsMenu = menu;

        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (LOCAL_LOGV) Log.v(TAG, String.format("onOptionsItemSelected(%s)", item.toString()));
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        // Handle the "settings" icon/text click
        if (id == R.id.action_settings) {
            return true;
        }

        // Handle the "developer test" icon/text click
        if (id == R.id.action_test) {
            return true;
        }

        // Handle the trash can "discard" icon click
        if (id == R.id.action_discard) {
            AlertDialog diaBox = confirmDiscard();
            diaBox.show();
        }

        // Handle the refresh/progress icon click
        if (id == R.id.action_refresh) {
            // show the indeterminate progress bar
            this.setRefreshActionButtonState(true);
            // use OcWorker to discover resources
            this.mOcWorker.doDiscoverResources();
        }

        return super.onOptionsItemSelected(item);
    }

    /**
     * Handle a resource changed callback from OcWorker by posting a runnable to our
     * own UI-safe looper/handler
     */
    @Override
    public void onResourceChanged(final OcResourceInfo resourceInfo) {
        if (LOCAL_LOGV) Log.v(TAG, "onResourceChanged()");

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // in case we were waiting for a refresh, hide the indeterminate progress bar
                setRefreshActionButtonState(false);

                mResourceListAdapter.notifyDataSetChanged();
            }
        });
    }

    /**
     * Handle a new resource found callback from OcWorker by posting a runnable to our
     * own UI-safe looper/handler
     */
    @Override
    public void onResourceFound(final OcResourceInfo resourceInfo) {
        if (LOCAL_LOGV) Log.v(TAG, "onResourceFound()");

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // in case we were waiting for a refresh, hide the indeterminate progress bar
                setRefreshActionButtonState(false);

                // if resource not already in list, add it
                if(!mResourceList.contains(resourceInfo)) {
                    mResourceList.add(resourceInfo);
                }

                mResourceListAdapter.notifyDataSetChanged();
            }
        });
    }

    public void toggleLedSwitch(OcResourceInfo resourceInfo, boolean onOff) {
        if (LOCAL_LOGV) Log.d(TAG, String.format("toggleLedSwitch(%s, %s)",
                resourceInfo.getHost() + resourceInfo.getUri(), String.valueOf(onOff)));

        // send a msg to OcWorker to put the switch value
        for(OcAttributeInfo ai : resourceInfo.getAttributes()) {
            if(ai.getType() == PLATFORM_LED_SWITCH) {
                if(onOff) {
                    ai.setValueInt(1);
                } else {
                    ai.setValueInt(0);
                }
            }
        }
        this.mOcWorker.doPutResource(resourceInfo);
    }

    public void toggleLightSwitch(OcResourceInfo resourceInfo, boolean onOff) {
        if (LOCAL_LOGV) Log.d(TAG, String.format("toggleLightSwitch(%s, %s)",
                resourceInfo.getHost() + resourceInfo.getUri(), String.valueOf(onOff)));

        // send a msg to OcWorker to put the switch value
        for(OcAttributeInfo ai : resourceInfo.getAttributes()) {
            if(ai.getType() == LIGHT_SWITCH) {
                ai.setValueBool(onOff);
            }
        }
        this.mOcWorker.doPutResource(resourceInfo);
    }

    public void setLightDimmerLevel(OcResourceInfo resourceInfo, int value) {
        if (LOCAL_LOGV) Log.d(TAG, String.format("setLightDimmerLevel(%s, %s)",
                resourceInfo.getHost() + resourceInfo.getUri(), String.valueOf(value)));

        // send a msg to OcWorker to put the switch value
        for(OcAttributeInfo ai : resourceInfo.getAttributes()) {
            if(ai.getType() == LIGHT_DIMMER) {
                ai.setValueInt(value);
            }
        }
        this.mOcWorker.doPutResource(resourceInfo);
    }

    /**
     * Sets the Action Bar icon to "progress" (spinning circle), or returns it to refresh icon
     *
     * @param refreshing true sets icon to animated "progress" spinner; false to static
     *                   refresh icon
     */
    private void setRefreshActionButtonState(final boolean refreshing) {
        if (this.optionsMenu != null) {
            final MenuItem refreshItem
         = this.optionsMenu
                    .findItem(R.id.action_refresh);
            if (refreshItem != null) {
                if (refreshing) {
                    refreshItem.setActionView(R.layout.actionbar_indeterminate_progress);
                    ProgressBar progressBar =
                            (ProgressBar) findViewById(R.id.find_resource_progress_bar);
                    progressBar.setOnClickListener(this);

                } else {
                    refreshItem.setActionView(null);
                }
            }
        }
    }

    private AlertDialog confirmDiscard()
    {
        if (LOCAL_LOGV) Log.v(TAG, "confirmDiscard()");

        return new AlertDialog.Builder(this)
                .setTitle("Clear Resource List?")
                .setIcon(R.drawable.ic_action_discard_dark)
                .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        // clear OcWorker's list
                        mOcWorker.doClearResources();
                        // in case its running, hide the indeterminate progress bar
                        setRefreshActionButtonState(false);
                        // clear our local data model list
                        mResourceList.clear();
                        mResourceListAdapter.notifyDataSetChanged();
                        dialog.dismiss();
                    }
                })

                .setNegativeButton("No", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                })

                .create();
    }

}
