package com.example.sample.provider;

import org.iotivity.base.OcPlatform;

public class StringConstants {
    public static final String RESOURCE_URI       = "/a/TempHumSensor/hosting";
    public static final String RESOURCE_TYPENAME  = "oic.r.resourcehosting";
    public static final String RESOURCE_INTERFACE = OcPlatform.DEFAULT_INTERFACE; // resource interface
    public static final String HUMIDITY           = "humidity";
    public static final String TEMPERATURE        = "temperature";
    public static final String MESSAGE            = "message";
    public static final int    ERROR_CODE         = 200;

}