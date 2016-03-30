/*
 * Copyright 2015 Samsung Electronics All Rights Reserved.
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
 */

package oic.simulator.clientcontroller.remoteresource;

import java.util.List;

/**
 * This is a helper class for showing the resource attributes in PUT and POST
 * dialogs.
 */
public class PutPostAttributeModel {

    private String       attName;
    private String       attValue;
    private List<String> values;
    boolean              modified;

    public String getAttName() {
        return attName;
    }

    public void setAttName(String attName) {
        this.attName = attName;
    }

    public String getAttValue() {
        return attValue;
    }

    public void setAttValue(String attValue) {
        this.attValue = attValue;
    }

    public List<String> getValues() {
        return values;
    }

    public void setValues(List<String> values) {
        this.values = values;
    }

    public boolean isModified() {
        return modified;
    }

    public void setModified(boolean modified) {
        this.modified = modified;
    }

    public static PutPostAttributeModel getModel(
            RemoteResourceAttribute attribute) {
        PutPostAttributeModel putPostModel = null;
        if (null != attribute) {
            putPostModel = new PutPostAttributeModel();
            putPostModel.setAttName(attribute.getAttributeName());
            putPostModel.setAttValue(String.valueOf(attribute
                    .getAttributeValue()));
            putPostModel.setValues(attribute.getAllValues());
            putPostModel.setModified(false);
        }
        return putPostModel;
    }

    @Override
    public String toString() {
        return attName + "," + attValue + "\n";
    }

}
