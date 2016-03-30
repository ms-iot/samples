//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
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
package com.example.sample.consumer;

import java.util.EnumSet;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.iotivity.base.ModeType;
import org.iotivity.base.ObserveType;
import org.iotivity.base.OcConnectivityType;
import org.iotivity.base.OcException;
import org.iotivity.base.OcHeaderOption;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcRepresentation;
import org.iotivity.base.OcResource;
import org.iotivity.base.OcResource.OnDeleteListener;
import org.iotivity.base.OcResource.OnGetListener;
import org.iotivity.base.OcResource.OnObserveListener;
import org.iotivity.base.OcResource.OnPutListener;
import org.iotivity.base.PlatformConfig;
import org.iotivity.base.QualityOfService;
import org.iotivity.base.ServiceType;

import com.sec.android.iot.sampleconsumer.R;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class SampleConsumer extends Activity implements View.OnClickListener,
        OcPlatform.OnResourceFoundListener, OnGetListener, OnDeleteListener,
        OnObserveListener, OnPutListener {
    private final String                      TAG                = "NMConsumer : "
                                                                         + this.getClass()
                                                                                 .getSimpleName();

    public static final int                   OC_STACK_OK        = 0;
    public static final String                OBSERVE            = "Observe";
    public static final String                GET                = "Get";
    public static final String                PUT                = "Put";
    public static final String                DELETE             = "Delete";
    public static final String                POST               = "Post";
    public static final String                MESSAGE            = "message";
    public static String                      outputString       = "";

    private Button                            btn_observe;
    private Button                            btn_get;
    private Button                            btn_put;
    private Button                            btn_post;
    private Button                            btn_delete;
    private Button                            btn_clean;

    public TextView                           tv_found_uri;
    public TextView                           tv_select_method_type;
    public TextView                           tv_receive_result;
    public TextView                           tv_current_log_result;

    public String                             current_log_result = "";
    public String                             found_uri          = "";
    public String                             select_method_type = "";
    public String                             receive_result     = "";

    public static OcResource                  curResource;
    public OcPlatform.OnResourceFoundListener OnResourceFoundListener;

    public static int                         oc                 = 0;

    /*
     * To initialize UI Function Setting To execute initOICStack for running
     * find resource
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.sampleconsumer_layout);

        btn_observe = (Button) findViewById(R.id.btn_observe);
        btn_get = (Button) findViewById(R.id.btn_get);
        btn_put = (Button) findViewById(R.id.btn_put);
        btn_post = (Button) findViewById(R.id.btn_post);
        btn_delete = (Button) findViewById(R.id.btn_delete);
        btn_clean = (Button) findViewById(R.id.btn_clean);
        tv_found_uri = (TextView) findViewById(R.id.tv_found_resource_result);
        tv_select_method_type = (TextView) findViewById(R.id.tv_selected_method_type);
        tv_receive_result = (TextView) findViewById(R.id.tv_receive_result);
        tv_current_log_result = (TextView) findViewById(R.id.tv_current_log_result);
        btn_observe.setOnClickListener(this);
        btn_get.setOnClickListener(this);
        btn_put.setOnClickListener(this);
        btn_post.setOnClickListener(this);
        btn_delete.setOnClickListener(this);
        btn_clean.setOnClickListener(this);

        initOICStack();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        onStop();
    }

    /*
     * To reset text String is null for cleaning log
     */
    @Override
    protected void onStop() {
        super.onStop();

        tv_found_uri = null;
        tv_select_method_type = null;
        tv_receive_result = null;
        tv_current_log_result = null;

        current_log_result = "";
        found_uri = "";
        select_method_type = "";
        receive_result = "";
        btn_observe.setClickable(true);
        btn_get.setClickable(true);
        btn_put.setClickable(true);
        btn_delete.setClickable(true);
    }

    public void cleanLogString() {
        current_log_result = "";
        found_uri = "";
        select_method_type = "";
        receive_result = "";

        tv_current_log_result.setText(current_log_result);
        tv_found_uri.setText(found_uri);
        tv_select_method_type.setText(select_method_type);
        tv_receive_result.setText(receive_result);
    }

    public void initOICStack() {
        PlatformConfig cfg = new PlatformConfig(this, ServiceType.IN_PROC,
                ModeType.CLIENT, "0.0.0.0", 0, QualityOfService.LOW);
        OcPlatform.Configure(cfg);
        current_log_result += "Created Platform...\n";
        tv_current_log_result.setText(current_log_result);
        Log.i(TAG, current_log_result);
        findResourceCandidate();
        PRINT();
    }

    public void findResourceCandidate() {
        nmfindResource("", "/oic/res?rt=oic.r.resourcehosting");
        current_log_result += "Finding Resource... \n";
        tv_current_log_result.setText(current_log_result);
        Log.i(TAG, current_log_result);
    }

    public void nmfindResource(String host, String resourceName) {
        try {
            EnumSet<OcConnectivityType> propertySet = EnumSet
                    .of(OcConnectivityType.CT_DEFAULT);
            OcPlatform.findResource(host, resourceName, propertySet, this);

        } catch (OcException e) {
            e.printStackTrace();
            current_log_result += e.getMessage() + "\n";
            tv_current_log_result.setText(current_log_result);
            Log.i(TAG, current_log_result);
        }
    }

    /*
     * No Use until Yet
     */
    public void getRepresentation(OcResource resource) {
        if (resource != null) {
            current_log_result += "Getting Light Representation...\n";
            tv_current_log_result.setText(current_log_result);
            Log.i(TAG, current_log_result);
        }
    }

    /*
     * No Use until Yet
     */
    public void getLightRepresentation(OcResource resource) {
        if (resource != null) {
            current_log_result += "Getting Light Representation...\n";
            tv_current_log_result.setText(current_log_result);
            Log.i(TAG, current_log_result);

            try {
                Map<String, String> queryParamsMap = new HashMap<String, String>();
                resource.get(queryParamsMap, this);
            } catch (OcException e) {
                e.printStackTrace();
                current_log_result += e.getMessage() + "\n";
                tv_current_log_result.setText(current_log_result);
                Log.i(TAG, current_log_result);
            }
        }
    }

    public void PRINT() {
        current_log_result += "********************************************\n";
        current_log_result += "*  method Type : 1 - Observe               *\n";
        current_log_result += "*  method Type : 2 - Get                   *\n";
        current_log_result += "*  method Type : 3 - Put                   *\n";
        current_log_result += "*  method Type : 4 - Delete                *\n";
        current_log_result += "********************************************\n";
    }

    public void startObserve(OcResource resource) {
        if (resource != null) {
            Map<String, String> queryParamsMap = new HashMap<String, String>();
            current_log_result += "startObserve\n";
            tv_current_log_result.setText(current_log_result);
            Log.i(TAG, current_log_result);

            try {
                resource.observe(ObserveType.OBSERVE, queryParamsMap, this);
            } catch (OcException e) {
                e.printStackTrace();
                current_log_result += e.getMessage() + "\n";
                tv_current_log_result.setText(current_log_result);
                Log.i(TAG, current_log_result);
            }
        }
    }

    public void startGet(OcResource resource) {
        if (resource != null) {
            Map<String, String> queryParamsMap = new HashMap<String, String>();
            current_log_result += "startGet\n";
            tv_current_log_result.setText(current_log_result);
            Log.i(TAG, current_log_result);

            try {
                resource.get(queryParamsMap, this);
            } catch (OcException e) {
                e.printStackTrace();
                current_log_result += e.getMessage() + "\n";
                tv_current_log_result.setText(current_log_result);
                Log.i(TAG, current_log_result);
            }
        }
    }

    public void startPut(OcResource resource) {
        if (resource != null) {
            curResource = resource;
            OcRepresentation rep = new OcRepresentation();
            rep.setValueInt("temperature", 25);
            rep.setValueInt("humidity", 10);
            Map<String, String> queryParamsMap = new HashMap<String, String>();
            current_log_result += "startPut\n";
            tv_current_log_result.setText(current_log_result);
            Log.i(TAG, current_log_result);

            try {
                resource.put(rep, queryParamsMap, this);
            } catch (OcException e) {
                e.printStackTrace();
                current_log_result += e.getMessage() + "\n";
                tv_current_log_result.setText(current_log_result);
                Log.i(TAG, current_log_result);
            }
        }
    }

    public void startDelete(OcResource resource) throws OcException {
        curResource = resource;
        if (resource != null) {
            resource.deleteResource(this);
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_observe:
                tv_select_method_type.setText(OBSERVE);
                Log.i(TAG, "Method: " + OBSERVE);
                startObserve(curResource);
                btn_observe.setClickable(false);
                break;
            case R.id.btn_get:
                tv_select_method_type.setText(GET);
                Log.i(TAG, "Method: " + GET);
                startGet(curResource);
                btn_get.setClickable(false);
                break;
            case R.id.btn_put:
                tv_select_method_type.setText(PUT);
                Log.i(TAG, "Method: " + PUT);
                startPut(curResource);
                btn_put.setClickable(false);
                break;
            case R.id.btn_post:
                tv_select_method_type.setText(POST);
                Toast.makeText(this, "Not Supported Yet", Toast.LENGTH_SHORT)
                        .show();
                Log.i(TAG, "Method: " + POST);
                break;
            case R.id.btn_delete:
                tv_select_method_type.setText(DELETE);
                Log.i(TAG, "Method: " + DELETE);
                try {
                    startDelete(curResource);
                } catch (OcException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
                btn_delete.setClickable(false);
                break;
            case R.id.btn_clean:
                cleanLogString();
                Log.i(TAG, "Log textbox is cleared");
                break;

            default:
                break;
        }

    }

    protected static int observe_count() {
        return ++oc;
    }

    public void viewText() {
        SampleConsumer.this.runOnUiThread(new Runnable() {
            public void run() {
                if (receive_result != null) {
                    tv_receive_result.setText(receive_result);
                    Log.i(TAG, "Received: " + receive_result);
                }
                if (found_uri != null) {
                    tv_found_uri.setText(found_uri);
                    Log.i(TAG, "URI: " + found_uri);
                }
                if (current_log_result != null) {
                    tv_current_log_result.setText(current_log_result);
                    Log.i(TAG, current_log_result);
                }

            }
        });
    }

    @Override
    public synchronized void onResourceFound(OcResource resource) {
        // synchronized (this) {
        receive_result = "FoundResource";
        String resourceURI;
        String hostAddress;
        if (SampleConsumer.curResource != null) {
            current_log_result += "Found another resource, ignoring\n";
        }
        if (resource != null) {
            if (resource.getUri().equals("/a/TempHumSensor")) {
                current_log_result += "==============================\n";
                current_log_result += "DISCOVERED Resource(Consumer):\n";
                resourceURI = resource.getUri();
                hostAddress = resource.getHost();
                current_log_result += "URI of the resource: " + resourceURI
                        + "\n";
                current_log_result += "Host address of the resource: "
                        + hostAddress + "\n";
                SampleConsumer.curResource = resource;
            } else {
                current_log_result += "Uri is not correct.";
            }
        } else {
            current_log_result += "Resource is invalid\n";
        }

        viewText();
    }

    public String getCurrent_log_result() {
        return current_log_result;
    }

    public void setCurrent_log_result(String current_log_result) {
        this.current_log_result = current_log_result;
    }

    public String getFound_uri() {
        return found_uri;
    }

    public void setFound_uri(String found_uri) {
        this.found_uri = found_uri;
    }

    public String getSelect_method_type() {
        return select_method_type;
    }

    public void setSelect_method_type(String select_method_type) {
        this.select_method_type = select_method_type;
    }

    public String getReceive_result() {
        return receive_result;
    }

    public void setReceive_result(String recieve_result) {
        this.receive_result = recieve_result;
    }

    public class MessageReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String message = intent.getStringExtra(MESSAGE);
            tv_current_log_result.setText(message);
            Log.i(TAG, message);
            viewText();
        }
    }

    @Override
    public void onGetCompleted(List<OcHeaderOption> options,
            OcRepresentation rep) {
        setReceive_result("onGet");
        setCurrent_log_result(getCurrent_log_result()
                + "GET request was successful\n"
                + "GET request was successful\n" + "URI: " + rep.getUri()
                + "\n" + "Temperature: " + rep.getValueInt("temperature")
                + "\n" + "Humidity: " + rep.getValueInt("humidity") + "\n");
        viewText();
        btn_get.setClickable(true);
    }

    @Override
    public void onPutCompleted(List<OcHeaderOption> options,
            OcRepresentation rep) {
        setReceive_result("onPut");
        current_log_result += "PUT request was successful\n";

        int humidity;
        int temperature;
        humidity = rep.getValueInt("humidity");
        temperature = rep.getValueInt("temperature");

        setCurrent_log_result(getCurrent_log_result() + "temperature: "
                + temperature + "\n");
        setCurrent_log_result(getCurrent_log_result() + "humidity: " + humidity
                + "\n");
        viewText();
        btn_put.setClickable(true);
    }

    @Override
    public void onObserveCompleted(List<OcHeaderOption> options,
            OcRepresentation rep, int seqNum) {
        setReceive_result("onObserve");
        setCurrent_log_result(getCurrent_log_result() + "SequenceNumber : "
                + seqNum + "\n");
        setCurrent_log_result("========================================================\n"
                + "Receive OBSERVE RESULT:\n"
                + "URI: "
                + rep.getUri()
                + "\n"
                + "SequenceNumber: "
                + seqNum
                + "\n"
                + "Temperature: "
                + rep.getValueInt("temperature")
                + "\n"
                + "Humidity: "
                + rep.getValueInt("humidity") + "\n");
        if (SampleConsumer.observe_count() > 30) {
            setCurrent_log_result(getCurrent_log_result()
                    + "Cancelling Observe...\n");
            try {
                SampleConsumer.curResource.cancelObserve();
            } catch (OcException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                setCurrent_log_result(getCurrent_log_result()
                        + "Cancel result :" + e.getMessage() + "\n");
            }
        }
        viewText();
    }

    @Override
    public void onDeleteCompleted(List<OcHeaderOption> options) {
        setReceive_result("onDelete");
        viewText();
        btn_delete.setClickable(true);
    }

    @Override
    public void onPutFailed(Throwable arg0) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onObserveFailed(Throwable arg0) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onDeleteFailed(Throwable arg0) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onGetFailed(Throwable arg0) {
        // TODO Auto-generated method stub

    }
}
