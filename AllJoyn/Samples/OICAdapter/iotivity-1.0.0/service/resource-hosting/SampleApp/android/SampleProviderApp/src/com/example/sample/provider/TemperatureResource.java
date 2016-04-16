package com.example.sample.provider;

import java.util.EnumSet;
import java.util.LinkedList;
import java.util.List;

import org.iotivity.base.EntityHandlerResult;
import org.iotivity.base.ObservationInfo;
import org.iotivity.base.OcException;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcRepresentation;
import org.iotivity.base.OcResourceHandle;
import org.iotivity.base.OcResourceRequest;
import org.iotivity.base.OcResourceResponse;
import org.iotivity.base.RequestHandlerFlag;
import org.iotivity.base.RequestType;
import org.iotivity.base.ResourceProperty;

import android.content.Context;
import android.content.Intent;
import android.os.Message;
import android.util.Log;

public class TemperatureResource implements IMessageLogger {
    private Context          mContext;
    public int               mtemp;
    public int               mhumidity;
    private OcRepresentation mTempRep;
    private OcResourceHandle mResourceHandle;
    private List<Byte>       mObservationIds;

    private static String    TAG = "NMProvider : TemperatureResource";

    TemperatureResource(Context context) {
        mContext = context;
        mtemp = 0;
        mhumidity = 0;
        mResourceHandle = null;
        mObservationIds = new LinkedList<Byte>();
        mTempRep = new OcRepresentation();
        mTempRep.setValueInt(StringConstants.TEMPERATURE, mtemp);
        mTempRep.setValueInt(StringConstants.HUMIDITY, mhumidity);
    }

    // accessor methods
    protected int getTemp() {
        return mtemp;
    }

    protected void setTemp(int temp) {
        mtemp = temp;
    }

    protected int getHumidity() {
        return mhumidity;
    }

    protected void setHumidity(int humidity) {
        mhumidity = humidity;
    }

    protected OcResourceHandle getHandle() {
        return mResourceHandle;
    }

    protected OcResourceHandle destroyResource() {
        if (mResourceHandle != null) {
            try {
                OcPlatform.unregisterResource(mResourceHandle);
            } catch (OcException e) {
                e.printStackTrace();
            }
            mResourceHandle = null;
        }
        return null;
    }

    protected OcResourceHandle createResource() {

        OcPlatform.EntityHandler eh = new OcPlatform.EntityHandler() {
            @Override
            public EntityHandlerResult handleEntity(
                    OcResourceRequest ocResourceRequest) {
                // this is where the main logic of simpleserver is handled as
                // different requests (GET, PUT, POST, OBSERVE, DELETE) are
                // handled
                return entityHandler(ocResourceRequest);
            }
        };

        try {
            Log.e(TAG, "before registerResource!");
            mResourceHandle = OcPlatform.registerResource(
                    StringConstants.RESOURCE_URI,
                    StringConstants.RESOURCE_TYPENAME,
                    StringConstants.RESOURCE_INTERFACE, eh, EnumSet.of(
                            ResourceProperty.DISCOVERABLE,
                            ResourceProperty.OBSERVABLE));
            Log.e(TAG, "after regiterResource");
        } catch (OcException e) {
            Log.e(TAG, "go exception");
            logMessage(TAG + "RegisterResource error. " + e.getMessage());
            Log.e(TAG, "RegisterResource error. " + e.getMessage());
        }

        // logMessage(TAG + "Successfully registered resource");
        return mResourceHandle;
    }

    private void put(OcRepresentation rep) {
        mtemp = rep.getValueInt(StringConstants.TEMPERATURE);
        mhumidity = rep.getValueInt(StringConstants.HUMIDITY);
        logMessage(TAG + "PUT Request" + "temperature : " + mtemp
                + "humidity : " + mhumidity);
        notifyObserver();
        // " Power: " + mPower);
        String message = mtemp + ":" + mhumidity;
        Message msg = Message.obtain();
        msg.what = 0;
        SampleProvider mainActivityObj = SampleProvider
                .getSampleProviderObject();
        SampleProvider.setmessage(message);
        mainActivityObj.getmHandler().sendMessage(msg);

    }

    protected OcRepresentation get() {
        mTempRep.setValueInt(StringConstants.TEMPERATURE, mtemp);
        mTempRep.setValueInt(StringConstants.HUMIDITY, mhumidity);
        return mTempRep;
    }

    private EntityHandlerResult entityHandler(OcResourceRequest request) {
        EntityHandlerResult result = EntityHandlerResult.ERROR;
        if (null != request) {
            RequestType requestType = request.getRequestType();
            EnumSet<RequestHandlerFlag> requestFlag = request
                    .getRequestHandlerFlagSet();

            if (requestFlag.contains(RequestHandlerFlag.INIT)) {
                logMessage(TAG + "Init");
            }

            if (requestFlag.contains(RequestHandlerFlag.REQUEST)) {
                try {
                    logMessage(TAG + requestType + "Request");
                    OcResourceResponse ocResourceResponse = new OcResourceResponse();
                    ocResourceResponse.setRequestHandle(request
                            .getRequestHandle());
                    ocResourceResponse.setResourceHandle(request
                            .getResourceHandle());

                    switch (requestType) {
                    // handle GET request
                        case GET:
                            logMessage("GET");
                            ocResourceResponse
                                    .setResponseResult(EntityHandlerResult.OK);
                            ocResourceResponse
                                    .setErrorCode(StringConstants.ERROR_CODE);
                            ocResourceResponse.setResourceRepresentation(get());
                            OcPlatform.sendResponse(ocResourceResponse);
                            break;
                        // handle PUT request
                        case PUT:
                            logMessage(TAG + "PUT");
                            OcRepresentation rep = request
                                    .getResourceRepresentation();
                            put(rep);
                            ocResourceResponse
                                    .setErrorCode(StringConstants.ERROR_CODE);
                            ocResourceResponse
                                    .setResponseResult(EntityHandlerResult.OK);
                            ocResourceResponse.setResourceRepresentation(get());
                            OcPlatform.sendResponse(ocResourceResponse);
                            break;
                        // handle POST request
                        case POST:
                            break;
                        // handle DELETE request
                        case DELETE:
                            logMessage(TAG + "DELETE");
                            OcPlatform.unregisterResource(getHandle());
                            break;

                    }

                    result = EntityHandlerResult.OK;
                } catch (Exception e) {
                    logMessage(TAG + "Error in Request " + e.getMessage());
                    Log.e(TAG, e.getMessage());
                }
            }
            // handle OBSERVER request
            if (requestFlag.contains(RequestHandlerFlag.OBSERVER)) {
                logMessage(TAG + "OBSERVER");
                ObservationInfo observationInfo = request.getObservationInfo();
                switch (observationInfo.getObserveAction()) {
                    case REGISTER:
                        mObservationIds.add(observationInfo
                                .getOcObservationId());

                        break;
                    case UNREGISTER:
                        mObservationIds.remove(observationInfo
                                .getOcObservationId());
                        break;
                }

                result = EntityHandlerResult.OK;
            }
        }
        return result;
    }

    public void logMessage(String msg) {
        logMsg(msg);
        Log.i(TAG, msg);
    }

    public void logMsg(final String text) {
        Intent intent = new Intent("com.example.sample.provider.SampleProvider");
        intent.putExtra(StringConstants.MESSAGE, text);
        mContext.sendBroadcast(intent);
    }

    public void notifyObserver() {
        try {
            // if observationList is not empty, call notifyListOfObservers
            if (mObservationIds.size() > 0) {
                OcResourceResponse ocResourceResponse = new OcResourceResponse();
                ocResourceResponse.setErrorCode(StringConstants.ERROR_CODE);
                ocResourceResponse.setResponseResult(EntityHandlerResult.OK);
                OcRepresentation r = get();
                ocResourceResponse.setResourceRepresentation(r,
                        OcPlatform.DEFAULT_INTERFACE);
                OcPlatform.notifyListOfObservers(getHandle(), mObservationIds,
                        ocResourceResponse);
            } else {
                // notify all observers if mObservationList is empty
                OcPlatform.notifyAllObservers(getHandle());
            }
        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
        }
    }
}
