package org.iotivity.ca.service;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;
import java.io.File;

import org.iotivity.ca.sample_service.R;
import org.iotivity.ca.service.FileChooser;
import org.iotivity.ca.service.FileChooser.FileSelectedListener;

public class MainActivity extends Activity {

    static RMInterface RM = new RMInterface();

    private final static String TAG = "Sample_Service : MainActivity";

    private final CharSequence[] mNetworkCheckBoxItems = { Network.IP.name(),
            Network.LE.name(), Network.EDR.name()};

    private final CharSequence[] mDTLSCheckBoxItems = { DTLS.UNSECURED.name(),
            DTLS.SECURED.name() };

    private final CharSequence[] mMsgTyleCheckBoxItems = { MsgType.CON.name(),
            MsgType.NON.name(), MsgType.ACK.name(), MsgType.RESET.name() };

    private final CharSequence[] mResponseResultCheckBoxItems = {
            ResponseResult.CA_CREATED.name(), ResponseResult.CA_DELETED.name(),
            ResponseResult.CA_VALID.name(), ResponseResult.CA_CHANGED.name(),
            ResponseResult.CA_CONTENT.name(), ResponseResult.CA_EMPTY.name(),
            ResponseResult.CA_BAD_REQ.name(), ResponseResult.CA_BAD_OPT.name(),
            ResponseResult.CA_NOT_FOUND.name(), ResponseResult.CA_INTERNAL_SERVER_ERROR.name(),
            ResponseResult.CA_RETRANSMIT_TIMEOUT.name() };

    private enum Mode {
        SERVER, CLIENT, BOTH, UNKNOWN
    };

    private enum Network {
        IP, LE, EDR
    };

    private enum DTLS {
        UNSECURED, SECURED
    };

    private enum MsgType {
        CON, NON, ACK, RESET
    };

    private enum ResponseResult {
        CA_CREATED, CA_DELETED, CA_VALID, CA_CHANGED, CA_CONTENT, CA_EMPTY,
        CA_BAD_REQ, CA_BAD_OPT, CA_NOT_FOUND, CA_INTERNAL_SERVER_ERROR, CA_RETRANSMIT_TIMEOUT
    }

    private boolean mCheckedItems[] = {
            false, false, false, false
    };

    private int mSelectedItems[] = { 0, 0, 0 };

    private int mUnSelectedItems[] = { 0, 0, 0 };

    private Mode mCurrentMode = Mode.UNKNOWN;

    private RelativeLayout mSendNotificationLayout = null;

    private RelativeLayout mSendRequestLayout = null;

    private RelativeLayout mSendRequestToAllLayout = null;

    private RelativeLayout mSendRequestSettingLayout = null;

    private RelativeLayout mSendRequestToAllSettingLayout = null;

    private RelativeLayout mSendResponseNotiSettingLayout = null;

    private RelativeLayout mReceiveLayout = null;

    private RelativeLayout mRequestTitleLayout = null;

    private RelativeLayout mRequestToAllTitleLayout = null;

    private RelativeLayout mResponseNotificationTitleLayout = null;

    private RelativeLayout mHandleTitleLayout = null;

    private TextView mMode_tv = null;

    private TextView mNetwork_tv = null;

    private EditText mNotification_ed = null;

    private EditText mReqData_ed = null;

    private EditText mReqToAllData_ed = null;

    private Button mNotify_btn = null;

    private Button mReqeust_btn = null;

    private Button mReqeust_setting_btn = null;

    private Button mReqeustToAll_btn = null;

    private Button mReqeustToAll_setting_btn = null;

    private Button mResponse_Notify_setting_btn = null;

    private Button mResponse_btn = null;

    private Button mGetNetworkInfo_btn = null;

    private Button mRecv_btn = null;

    private Button mBig_btn = null;

    private Handler mLogHandler = null;

    /**
     * Defined ConnectivityType in cacommon.c
     *
     * CA_IP = (1 << 0) CA_LE = (1 << 2) CA_EDR = (1 << 3)
     */
    private int CA_IP = (1 << 0);
    private int CA_LE = (1 << 1);
    private int CA_EDR = (1 << 2);
    private int isSecured = 0;
    private int msgType = 1;
    private int responseValue = 0;
    private int selectedNetworkType = -1;
    private int selectedMsgType = 1;
    private int selectedMsgSecured = 0;
    private int selectedResponseValue = 0;
    int selectedNetwork = -1;
    int interestedNetwork = 0;
    int uninterestedNetwork = 0;
    private boolean isSendResponseSetting = false;
    private boolean isSendRequestToAllSetting = false;
    private boolean isBigData = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        RM.setResponseListener(this);

        mLogHandler = new Handler();
        TextView logView = (TextView) findViewById(R.id.tv_result);
        DLog.setTextView(mLogHandler, logView);

        // Initialize UI
        // common
        mReceiveLayout = (RelativeLayout) findViewById(R.id.layout_receive);

        // client
        mSendRequestLayout = (RelativeLayout) findViewById(R.id.layout_request);
        mSendRequestToAllLayout = (RelativeLayout) findViewById(R.id.layout_request_to_all);
        mSendRequestSettingLayout = (RelativeLayout)
                findViewById(R.id.layout_request_setting_for_client);
        mSendRequestToAllSettingLayout = (RelativeLayout)
                findViewById(R.id.layout_request_to_all_setting_for_client);
        mRequestTitleLayout = (RelativeLayout) findViewById(R.id.layout_request_title);
        mRequestToAllTitleLayout = (RelativeLayout) findViewById(R.id.layout_request_to_all_title);
        mHandleTitleLayout = (RelativeLayout) findViewById(R.id.layout_handle_title);

        // server
        mSendNotificationLayout = (RelativeLayout) findViewById(R.id.layout_notify);
        mSendResponseNotiSettingLayout = (RelativeLayout)
                findViewById(R.id.layout_request_setting_for_server);
        mResponseNotificationTitleLayout = (RelativeLayout)
                findViewById(R.id.layout_Response_Noti_title);

        mMode_tv = (TextView) findViewById(R.id.tv_mode);
        mNetwork_tv = (TextView) findViewById(R.id.tv_network);

        mNotification_ed = (EditText) findViewById(R.id.et_notification);
        mReqData_ed = (EditText) findViewById(R.id.et_req_data);
        mReqToAllData_ed = (EditText) findViewById(R.id.et_req_to_all_data);

        mResponse_btn = (Button) findViewById(R.id.btn_sendresponse);
        mNotify_btn = (Button) findViewById(R.id.btn_notify);
        mReqeust_btn = (Button) findViewById(R.id.btn_Request);
        mReqeust_setting_btn = (Button) findViewById(R.id.btn_Request_setting_for_client);
        mReqeustToAll_btn = (Button) findViewById(R.id.btn_request_to_all);
        mReqeustToAll_setting_btn = (Button)
                findViewById(R.id.btn_request_to_all_setting_for_client);
        mResponse_Notify_setting_btn = (Button) findViewById(R.id.btn_Request_setting_for_server);
        mGetNetworkInfo_btn = (Button) findViewById(R.id.btn_get_network_info);
        mRecv_btn = (Button) findViewById(R.id.btn_receive);
        mBig_btn = (Button) findViewById(R.id.btn_big_data);
        mBig_btn.setOnClickListener(mSelectLargeDataButtonHandler);

        mResponse_btn.setOnClickListener(mSendResponseHandler);
        mNotify_btn.setOnClickListener(mNotifyHandler);
        mReqeust_btn.setOnClickListener(mSendRequestHandler);
        mReqeust_setting_btn.setOnClickListener(mSendRequestSettingHandler);
        mReqeustToAll_btn.setOnClickListener(mSendRequestToAllHandler);
        mReqeustToAll_setting_btn.setOnClickListener(mSendRequestToAllSettingHandler);
        mResponse_Notify_setting_btn
                .setOnClickListener(mSendResponseNotiSettingHandler);
        mRecv_btn.setOnClickListener(mResponseHandler);
        mGetNetworkInfo_btn.setOnClickListener(mGetNetworkInfoHandler);

        showSelectModeView();

        // Initialize Connectivity Abstraction
        RM.RMInitialize(getApplicationContext());

        // set handler
        RM.RMRegisterHandler();
    }

    private void showSelectModeView() {

        mSendNotificationLayout.setVisibility(View.INVISIBLE);
        mSendRequestLayout.setVisibility(View.INVISIBLE);
        mSendRequestToAllLayout.setVisibility(View.INVISIBLE);
        mSendRequestSettingLayout.setVisibility(View.INVISIBLE);
        mSendRequestToAllSettingLayout.setVisibility(View.INVISIBLE);
        mReceiveLayout.setVisibility(View.INVISIBLE);
        mRequestTitleLayout.setVisibility(View.INVISIBLE);
        mRequestToAllTitleLayout.setVisibility(View.INVISIBLE);
        mHandleTitleLayout.setVisibility(View.INVISIBLE);
        mResponseNotificationTitleLayout.setVisibility(View.INVISIBLE);
        mSendResponseNotiSettingLayout.setVisibility(View.INVISIBLE);

        mMode_tv.setText("Select Mode (Server or Client)");
        Log.i(TAG, "Select Mode (Server or Client)");
    }

    private void showNetworkView() {

        mNetwork_tv.setText("Select Network Type");
        Log.i(TAG, "Select Network Type");
    }

    private void showModeView() {

        if (mCurrentMode == Mode.SERVER) {

            mSendNotificationLayout.setVisibility(View.VISIBLE);
            mSendRequestLayout.setVisibility(View.INVISIBLE);
            mSendRequestToAllLayout.setVisibility(View.VISIBLE);
            mSendRequestSettingLayout.setVisibility(View.INVISIBLE);
            mSendRequestToAllSettingLayout.setVisibility(View.VISIBLE);
            mReceiveLayout.setVisibility(View.VISIBLE);

            mRequestTitleLayout.setVisibility(View.INVISIBLE);
            mRequestToAllTitleLayout.setVisibility(View.VISIBLE);
            mHandleTitleLayout.setVisibility(View.VISIBLE);

            mResponseNotificationTitleLayout.setVisibility(View.VISIBLE);
            mSendResponseNotiSettingLayout.setVisibility(View.VISIBLE);

            mNetwork_tv.setText("");

        } else if (mCurrentMode == Mode.CLIENT) {

            mSendNotificationLayout.setVisibility(View.INVISIBLE);
            mSendRequestLayout.setVisibility(View.VISIBLE);
            mSendRequestToAllLayout.setVisibility(View.VISIBLE);
            mSendRequestSettingLayout.setVisibility(View.VISIBLE);
            mSendRequestToAllSettingLayout.setVisibility(View.VISIBLE);
            mReceiveLayout.setVisibility(View.VISIBLE);

            mRequestTitleLayout.setVisibility(View.VISIBLE);
            mRequestToAllTitleLayout.setVisibility(View.VISIBLE);
            mHandleTitleLayout.setVisibility(View.VISIBLE);

            mResponseNotificationTitleLayout.setVisibility(View.INVISIBLE);
            mSendResponseNotiSettingLayout.setVisibility(View.INVISIBLE);

            mNetwork_tv.setText("");
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        // Terminate Connectivity Abstraction
        RM.RMTerminate();
        android.os.Process.killProcess(android.os.Process.myPid());
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {

        menu.add(0, 1, Menu.NONE, "Start Server");
        menu.add(0, 2, Menu.NONE, "Start Client");
        menu.add(0, 3, Menu.NONE, "Select Network");

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {

        switch (item.getItemId()) {

        case 1:

            RM.RMStartListeningServer();

            if (interestedNetwork == 0) {
                mCurrentMode = Mode.SERVER;
                mMode_tv.setText("MODE: " + mCurrentMode.toString());
                Log.i(TAG, "MODE: " + mCurrentMode.toString());
                showNetworkView();

            } else {
                mCurrentMode = Mode.SERVER;
                mMode_tv.setText("MODE: " + mCurrentMode.toString());
                Log.i(TAG, "MODE: " + mCurrentMode.toString());
                showModeView();
            }

            break;

        case 2:

            RM.RMStartDiscoveryServer();

            if (interestedNetwork == 0) {
                mCurrentMode = Mode.CLIENT;
                mMode_tv.setText("MODE: " + mCurrentMode.toString());
                Log.i(TAG, "MODE: " + mCurrentMode.toString());
                showNetworkView();

            } else {
                mCurrentMode = Mode.CLIENT;
                mMode_tv.setText("MODE: " + mCurrentMode.toString());
                Log.i(TAG, "MODE: " + mCurrentMode.toString());
                showModeView();
            }

            break;

        case 3:

            checkInterestedNetwork("Select Network");

            break;
        }

        return super.onOptionsItemSelected(item);
    }

    private OnClickListener mSendResponseHandler = new OnClickListener() {

        @Override
        public void onClick(View v) {

            DLog.v(TAG, "SendResponse click");
            if ( selectedNetwork != -1) {
                RM.RMSendResponse(selectedNetwork, isSecured, msgType, responseValue);
            }
            else {
                DLog.v(TAG, "Please Select Network Type");
            }
        }
    };

    private OnClickListener mNotifyHandler = new OnClickListener() {

        @Override
        public void onClick(View v) {

            DLog.v(TAG, "SendNotification click");
            if ( selectedNetwork != -1) {
                RM.RMSendNotification(mNotification_ed.getText().toString(),
                    null, selectedNetwork, isSecured, msgType);
            }
            else {
                DLog.v(TAG, "Please Select Network Type");
            }
        }
    };

    private OnClickListener mSendRequestHandler = new OnClickListener() {

        @Override
        public void onClick(View v) {

            DLog.v(TAG, "SendRequest click");
            if ( selectedNetwork != -1) {
                RM.RMSendRequest(mReqData_ed.getText().toString(), null,
                    selectedNetwork, isSecured, msgType, false);
            }
            else {
                DLog.v(TAG, "Please Select Network Type");
            }
        }
    };

    private OnClickListener mSendRequestSettingHandler = new OnClickListener() {

        @Override
        public void onClick(View v) {
            checkSendNetworkType("Select Send Network Type");
        }
    };

    private OnClickListener mSendRequestToAllHandler = new OnClickListener() {

        @Override
        public void onClick(View v) {

            DLog.v(TAG, "SendRequestToAll click");
            if ( selectedNetwork != -1) {
                RM.RMSendReqestToAll(mReqToAllData_ed.getText().toString(), selectedNetwork);
            }
            else {
                DLog.v(TAG, "Please Select Network Type");
            }
        }
    };

    private OnClickListener mSendRequestToAllSettingHandler = new OnClickListener() {

        @Override
        public void onClick(View v) {
            isSendRequestToAllSetting = true;
            checkSendNetworkType("Select Send Network Type");
        }
    };

    private OnClickListener mSendResponseNotiSettingHandler = new OnClickListener() {

        @Override
        public void onClick(View v) {
            isSendResponseSetting = true;
            checkSendNetworkType("Select Send Network Type");
        }
    };

    private OnClickListener mGetNetworkInfoHandler = new OnClickListener() {
        @Override
        public void onClick(View v) {

            RM.RMGetNetworkInfomation();
        }
    };

    private OnClickListener mResponseHandler = new OnClickListener() {

        @Override
        public void onClick(View v) {

            RM.RMHandleRequestResponse();
        }
    };

    private OnClickListener mSelectLargeDataButtonHandler = new OnClickListener() {
        @Override
        public void onClick(View v) {

            isBigData = true;
            checkSendNetworkType("Select Send Network Type");
        }
    };

    private void checkInterestedNetwork(String title) {

        AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
        builder.setTitle(title)
                .setMultiChoiceItems(mNetworkCheckBoxItems, mCheckedItems,
                        new DialogInterface.OnMultiChoiceClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog,
                                    int which, boolean isChecked) {

                                if (isChecked) {

                                    mSelectedItems[which] = 1;
                                    mUnSelectedItems[which] = 0;

                                } else if (mSelectedItems[which] == 1) {

                                    mSelectedItems[which] = 0;
                                    mUnSelectedItems[which] = 1;
                                }
                            }
                        })
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {

                        interestedNetwork = 0;

                        for (int i = 0; i < mSelectedItems.length; i++) {
                            if (mSelectedItems[i] == 1) {
                                interestedNetwork |= (1 << i);
                            }
                        }
                        if(0 != interestedNetwork)
                            RM.RMSelectNetwork(interestedNetwork);

                        uninterestedNetwork = 0;

                        for (int i = 0; i < mUnSelectedItems.length; i++) {
                            if (mUnSelectedItems[i] == 1) {
                                uninterestedNetwork |= (1 << i);
                            }
                        }
                        if(0 != uninterestedNetwork)
                            RM.RMUnSelectNetwork(uninterestedNetwork);

                    }
                }).show();
    }

    private void checkMsgSecured(String title) {

        AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);

        builder.setTitle(title)
                .setSingleChoiceItems(mDTLSCheckBoxItems, selectedMsgSecured,
                        new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog,
                                    int which) {
                                selectedMsgSecured = which;
                            }
                        })
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {

                        if (selectedMsgSecured == DTLS.SECURED.ordinal()) {
                            isSecured = 1;
                            DLog.v(TAG, "Send secured message");

                        } else if (selectedMsgSecured == DTLS.UNSECURED.ordinal()) {
                            isSecured = 0;
                            DLog.v(TAG, "Send unsecured message");
                        }
                        checkMsgType("Select Msg Type");
                    }

                }).show();
    }

    private void checkMsgType(String title) {

        AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
        builder.setTitle(title)
                .setSingleChoiceItems(mMsgTyleCheckBoxItems, selectedMsgType,
                        new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog,
                                    int which) {
                                selectedMsgType = which;
                            }
                        })
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {

                        if (selectedMsgType == MsgType.CON.ordinal()) {
                            msgType = 0;
                            DLog.v(TAG, "Message Type is CON");

                        } else if (selectedMsgType == MsgType.NON.ordinal()) {
                            msgType = 1;
                            DLog.v(TAG, "Message Type is NON");
                        } else if (selectedMsgType == MsgType.ACK.ordinal()) {
                            msgType = 2;
                            DLog.v(TAG, "Message Type is ACK");
                        } else if (selectedMsgType == MsgType.RESET.ordinal()) {
                            msgType = 3;
                            DLog.v(TAG, "Message Type is RESET");
                            }

                        if (isSendResponseSetting == true && msgType != 3) {
                            checkResponseResult("Select Value of Response Result");
                            isSendResponseSetting = false;
                        }
                    }
                }).show();
    }

    private void checkResponseResult(String title) {

        AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
        builder.setTitle(title)
                .setSingleChoiceItems(mResponseResultCheckBoxItems, selectedResponseValue,
                        new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog,
                                    int which) {
                                selectedResponseValue = which;
                            }
                        })
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {

                        if (selectedResponseValue == ResponseResult.CA_CREATED.ordinal()) {
                            responseValue = 201;
                            DLog.v(TAG, "Response Value is CA_CREATED");
                        } else if (selectedResponseValue == ResponseResult.CA_DELETED
                                .ordinal()) {
                            responseValue = 202;
                            DLog.v(TAG, "Response Value is CA_DELETED");
                        } else if (selectedResponseValue == ResponseResult.CA_VALID
                                .ordinal()) {
                            responseValue = 203;
                            DLog.v(TAG, "Response Value is CA_VALID");
                        } else if (selectedResponseValue == ResponseResult.CA_CHANGED
                                .ordinal()) {
                            responseValue = 204;
                            DLog.v(TAG, "Response Value is CA_CHANGED");
                        } else if (selectedResponseValue == ResponseResult.CA_CONTENT
                                .ordinal()) {
                            responseValue = 205;
                            DLog.v(TAG, "Response Value is CA_CONTENT");
                        } else if (selectedResponseValue == ResponseResult.CA_EMPTY
                                .ordinal()) {
                            responseValue = 0;
                            DLog.v(TAG, "Response Value is CA_EMPTY");
                        } else if (selectedResponseValue == ResponseResult.CA_BAD_REQ
                                .ordinal()) {
                            responseValue = 400;
                            DLog.v(TAG, "Response Value is CA_BAD_REQ");
                        } else if (selectedResponseValue == ResponseResult.CA_BAD_OPT
                                .ordinal()) {
                            responseValue = 402;
                            DLog.v(TAG, "Response Value is CA_BAD_OPT");
                        } else if (selectedResponseValue == ResponseResult.CA_NOT_FOUND
                                .ordinal()) {
                            responseValue = 404;
                            DLog.v(TAG, "Response Value is CA_NOT_FOUND");
                        } else if (selectedResponseValue ==
                                ResponseResult.CA_INTERNAL_SERVER_ERROR
                                .ordinal()) {
                            responseValue = 500;
                            DLog.v(TAG, "Response Value is CA_INTERNAL_SERVER_ERROR");
                        } else if (selectedResponseValue == ResponseResult.CA_RETRANSMIT_TIMEOUT
                                .ordinal()) {
                            responseValue = 504;
                            DLog.v(TAG, "Response Value is CA_RETRANSMIT_TIMEOUT");
                        }
                    }
                }).show();
    }

    private void checkSendNetworkType(String title) {
        selectedNetworkType = -1;
        AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);

        builder.setTitle(title)
                .setSingleChoiceItems(mNetworkCheckBoxItems, -1,
                        new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog,
                                    int which) {
                                selectedNetworkType = which;
                            }
                        })
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {

                        if (selectedNetworkType == Network.IP.ordinal()) {
                            selectedNetwork = CA_IP;
                            DLog.v(TAG, "Selected Network is IP");
                        } else if (selectedNetworkType == Network.LE.ordinal()) {
                            selectedNetwork = CA_LE;
                            DLog.v(TAG, "Selected Network is LE");
                        } else if (selectedNetworkType == Network.EDR.ordinal()) {
                            selectedNetwork = CA_EDR;
                            DLog.v(TAG, "Selected Network is EDR");
                        } else {
                            DLog.v(TAG, "Selected Network is NULL");
                            selectedNetwork = -1;
                        }

                        if (isBigData)
                        {
                            new FileChooser(MainActivity.this).setFileListener(new FileSelectedListener() {
                                public void fileSelected(final File file) {
                                    if (selectedNetwork != -1) {

                                        String path = file.getAbsolutePath();
                                        Log.d(TAG, "File Path: " + path);

                                        RM.RMSendRequest(mReqData_ed.getText().toString(), path,
                                                         selectedNetwork, isSecured, msgType, true);
                                    } else {
                                        Toast.makeText(getApplicationContext(),
                                                       "Request Setting Fisrt!!", Toast.LENGTH_LONG).show();
                                    }
                                }
                            } ).showDialog();
                            isBigData = false;
                        } else {
                            if (isSendRequestToAllSetting != true) {
                                checkMsgSecured("Select DTLS Type");
                            }
                        }

                        isSendRequestToAllSetting = false;
                        isBigData = false;
                    }
                }).show();
    }

    public void OnResponseReceived(String subject, String receivedData) {
        String callbackData = subject + receivedData;
        DLog.v(TAG, callbackData);

        if (subject.equals(getString(R.string.remote_address))) {
            StringBuilder sb = new StringBuilder();
            sb.append(getString(R.string.coap_prefix)).append(receivedData);
            mReqData_ed.setText(sb.toString());
            mNotification_ed.setText(sb.toString());
        } else if (subject.equals(getString(R.string.remote_port))) {
            StringBuilder sb = new StringBuilder();
            String uri = mReqData_ed.getText().toString();
            sb.append(uri);
            if (null != receivedData && uri.contains("."))
            {
                sb.append(":").append(receivedData);
            }
            sb.append(getString(R.string.uri));
            mReqData_ed.setText(sb.toString());
            mNotification_ed.setText(sb.toString());
        }
    }
}