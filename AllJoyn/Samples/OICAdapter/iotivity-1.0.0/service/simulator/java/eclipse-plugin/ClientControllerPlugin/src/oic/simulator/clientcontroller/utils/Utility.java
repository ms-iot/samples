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

package oic.simulator.clientcontroller.utils;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

/**
 * This class has common utility methods.
 */
public class Utility {
    public static List<String> convertSetToList(Set<String> typeSet) {
        if (null == typeSet) {
            return null;
        }
        List<String> list = new ArrayList<String>();
        Iterator<String> typeItr = typeSet.iterator();
        while (typeItr.hasNext()) {
            list.add(typeItr.next());
        }
        return list;
    }

    public static String getObservableInString(boolean observable) {
        if (observable) {
            return Constants.YES;
        } else {
            return Constants.NO;
        }
    }

    public static String[] convertListToString(List<String> valueList) {
        String[] strArr;
        if (null != valueList && valueList.size() > 0) {
            strArr = valueList.toArray(new String[1]);
        } else {
            strArr = new String[1];
        }
        return strArr;
    }

    /*
     * public static List<Object> converArrayToList(int[] arr) { if(null == arr
     * || arr.length < 1) { return null; } List<Object> valueList = new
     * ArrayList<Object>(); for(Object val:arr) { valueList.add(val); } return
     * valueList; }
     * 
     * public static List<Object> converArrayToList(double[] arr) { if(null ==
     * arr || arr.length < 1) { return null; } List<Object> valueList = new
     * ArrayList<Object>(); for(Object val:arr) { valueList.add(val); } return
     * valueList; }
     * 
     * public static List<Object> converArrayToList(boolean[] arr) { if(null ==
     * arr || arr.length < 1) { return null; } List<Object> valueList = new
     * ArrayList<Object>(); for(Object val:arr) { valueList.add(val); } return
     * valueList; }
     * 
     * public static List<Object> converArrayToList(String[] arr) { if(null ==
     * arr || arr.length < 1) { return null; } List<Object> valueList = new
     * ArrayList<Object>(); for(Object val:arr) { valueList.add(val); } return
     * valueList; }
     */
}