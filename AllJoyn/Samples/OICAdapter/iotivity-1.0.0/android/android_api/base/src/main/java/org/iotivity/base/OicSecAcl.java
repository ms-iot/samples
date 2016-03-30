/*
 * //******************************************************************
 * //
 * // Copyright 2015 Samsung Electronics All Rights Reserved.
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

import java.io.Serializable;
import java.util.List;
import java.util.Arrays;

public class OicSecAcl implements Serializable {

    private String       subject;
    private int          permission;
    private List<String> resources;
    private List<String> periods;
    private List<String> recurrences;
    private List<String> owners;

    public OicSecAcl(String subject, List<String> recurrences, List<String> periods, int permission,
            List<String> resources, List<String> owners) {
        this.subject = subject;
        this.recurrences = recurrences;
        this.periods = periods;
        this.permission = permission;
        this.resources = resources;
        this.owners = owners;
    }

    public String getSubject() {
        return this.subject;
    }

    public void setSubject(String subject) {
        this.subject = subject;
    }

    public List<String> getOwners() {
        return owners;
    }

    public void setOwners(List<String> owners) {
        this.owners = owners;
    }

    public List<String> getRecurrences() {
        return recurrences;
    }

    public void setRecurrences(List<String> recurrences) {
        this.recurrences = recurrences;
    }

    public List<String> getPeriods() {
        return periods;
    }

    public void setPeriods(List<String> periods) {
        this.periods = periods;
    }

    public int getPermission() {
        return this.permission;
    }

    public void setPermission(int permission) {
        this.permission = permission;
    }

    public List<String> getResources() {
        return resources;
    }

    public void setResources(List<String> resources) {
        this.resources = resources;
    }

    public int getResourcesCount() {
        return this.resources.size();
    }

    public String getResources(int i) {
        return this.resources.get(i);
    }

    public int getPeriodsCount() {
        return this.periods.size();
    }

    public String getPeriods(int i) {
        return this.periods.get(i);
    }

    public String getRecurrences(int i) {
        return this.recurrences.get(i);
    }

    public int getOwnersCount() {
        return this.owners.size();
    }

    public String getOwners(int i) {
        return this.owners.get(i);
    }
}
