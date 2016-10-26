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

package org.iotivity.service.server;

import org.iotivity.service.RcsResourceAttributes;

class RcsResponse {
    private native static int nativeGetDefaultErrorCode();

    public static final int DEFAULT_ERROR_CODE;

    static {
        DEFAULT_ERROR_CODE = nativeGetDefaultErrorCode();
    }

    private final int                   mErrorCode;
    private final RcsResourceAttributes mAttrs;

    RcsResponse() {
        this(DEFAULT_ERROR_CODE);
    }

    RcsResponse(RcsResourceAttributes attrs) {
        this(attrs, DEFAULT_ERROR_CODE);
    }

    RcsResponse(int errorCode) {
        this(null, errorCode);
    }

    RcsResponse(RcsResourceAttributes attrs, int errorCode) {
        mErrorCode = errorCode;
        mAttrs = attrs;
    }
}
