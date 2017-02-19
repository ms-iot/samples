/*
 * //******************************************************************
 * //
 * // Copyright 2015 Intel Corporation.
 * //
 * //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //      http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

package org.iotivity.base;

public enum ErrorCode {
    /* Success status code - START HERE */
    OK("OK", ""),
    RESOURCE_CREATED("RESOURCE_CREATED", ""),
    RESOURCE_DELETED("RESOURCE_DELETED", ""),
    CONTINUE("CONTINUE", ""),
    /** Success status code - END HERE.*/
    /* Error status code - START HERE */
    INVALID_URI("INVALID_URI", ""),
    INVALID_QUERY("INVALID_QUERY", ""),
    INVALID_IP("INVALID_IP", ""),
    INVALID_PORT("INVALID_PORT", ""),
    INVALID_CALLBACK("INVALID_CALLBACK", ""),
    INVALID_METHOD("INVALID_METHOD", ""),
    INVALID_PARAM("INVALID_PARAM", "Invalid parameter"),
    INVALID_OBSERVE_PARAM("INVALID_OBSERVE_PARAM", ""),
    NO_MEMORY("NO_MEMORY", ""),
    COMM_ERROR("COMM_ERROR", ""),
    TIMEOUT("TIMEOUT", ""),
    ADAPTER_NOT_ENABLED("ADAPTER_NOT_ENABLED", ""),
    NOT_IMPL("NOTIMPL", ""),
    NO_RESOURCE("NO_RESOURCE", "Resource not found"),
    RESOURCE_ERROR("RESOURCE_ERROR", "Not supported method or interface"),
    SLOW_RESOURCE("SLOW_RESOURCE", ""),
    DUPLICATE_REQUEST("DUPLICATE_REQUEST", ""),
    NO_OBSERVERS("NO_OBSERVERS", "Resource has no registered observers"),
    OBSERVER_NOT_FOUND("OBSERVER_NOT_FOUND", ""),
    VIRTUAL_DO_NOT_HANDLE("VIRTUAL_DO_NOT_HANDLE", ""),
    INVALID_OPTION("INVALID_OPTION", ""),
    MALFORMED_RESPONSE("MALFORMED_RESPONSE", "Remote reply contained malformed data"),
    PERSISTENT_BUFFER_REQUIRED("PERSISTENT_BUFFER_REQUIRED", ""),
    INVALID_REQUEST_HANDLE("INVALID_REQUEST_HANDLE", ""),
    INVALID_DEVICE_INFO("INVALID_DEVICE_INFO", ""),
    INVALID_JSON("INVALID_JSON", ""),
    UNAUTHORIZED_REQ("UNAUTHORIZED_REQ", "Request is not authorized by Resource Server"),
    /** Error code from PDM */
    PDM_IS_NOT_INITIALIZED("PDM_IS_NOT_INITIALIZED", ""),
    DUPLICATE_UUID("DUPLICATE_UUID", ""),
    INCONSISTENT_DB("INCONSISTENT_DB", ""),
    /** Insert all new error codes here!.*/
    PRESENCE_STOPPED("PRESENCE_STOPPED", ""),
    PRESENCE_TIMEOUT("PRESENCE_TIMEOUT", ""),
    PRESENCE_DO_NOT_HANDLE("PRESENCE_DO_NOT_HANDLE", ""),
    /** ERROR in stack.*/
    ERROR("ERROR", "Generic error"),

    JNI_EXCEPTION("JNI_EXCEPTION", "Generic Java binder error"),
    JNI_NO_NATIVE_OBJECT("JNI_NO_NATIVE_OBJECT", ""),
    JNI_INVALID_VALUE("JNI_INVALID_VALUE", ""),
    INVALID_CLASS_CAST("INVALID_CLASS_CAST", ""),;

    private String error;
    private String description;

    private ErrorCode(String error, String description) {
        this.error = error;
        this.description = description;
    }

    public String getError() {
        return error;
    }

    public String getDescription() {
        return description;
    }

    public static ErrorCode get(String errorCode) {
        for (ErrorCode eCode : ErrorCode.values()) {
            if (eCode.getError().equals(errorCode)) {
                return eCode;
            }
        }
        throw new IllegalArgumentException("Unexpected ErrorCode value");
    }

    @Override
    public String toString() {
        return error + (description.isEmpty() ? "" : " : " + description);
    }
}
