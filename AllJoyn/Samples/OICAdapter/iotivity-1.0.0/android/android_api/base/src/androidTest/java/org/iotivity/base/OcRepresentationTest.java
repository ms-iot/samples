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

import android.test.InstrumentationTestCase;

import java.security.InvalidParameterException;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;

public class OcRepresentationTest extends InstrumentationTestCase {

    private static final String TAG = "OcRepresentationTest";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }

    public void testChildrenManagement() throws OcException {
        OcRepresentation representation = new OcRepresentation();

        List<OcRepresentation> emptyList = representation.getChildren();
        assertTrue(emptyList.isEmpty());

        OcRepresentation child1 = new OcRepresentation();
        OcRepresentation child2 = new OcRepresentation();
        String key = "key";
        int value = 75;

        child1.setValue(key, value);
        child2.setValue(key, value);
        representation.addChild(child1);
        representation.addChild(child2);
        List<OcRepresentation> twoChildren = representation.getChildren();
        assertEquals(2, twoChildren.size());
        for (OcRepresentation rep : twoChildren) {
            assertEquals(value, rep.getValue(key));
        }

        representation.clearChildren();
        emptyList = representation.getChildren();
        assertTrue(emptyList.isEmpty());
    }

    public void testUriGetSet() {
        OcRepresentation representation = new OcRepresentation();

        String emptyUri = representation.getUri();
        assertTrue(emptyUri.isEmpty());

        String expected = "a/resource/uri";
        representation.setUri(expected);
        String actual = representation.getUri();
        assertEquals(expected, actual);
    }

    public void testResourceTypesGetSet() {
        OcRepresentation representation = new OcRepresentation();

        List<String> emptyResourceTypeList = representation.getResourceTypes();
        assertTrue(emptyResourceTypeList.isEmpty());

        representation.setResourceTypes(emptyResourceTypeList);
        emptyResourceTypeList = representation.getResourceTypes();
        assertTrue(emptyResourceTypeList.isEmpty());

        List<String> resourceTypeListExpected = new LinkedList<String>();
        resourceTypeListExpected.add("type1");
        resourceTypeListExpected.add("type2");
        resourceTypeListExpected.add("type3");

        representation.setResourceTypes(resourceTypeListExpected);
        List<String> resourceTypeListActual = representation.getResourceTypes();
        assertEquals(resourceTypeListExpected.size(), resourceTypeListActual.size());
        for (int i = 0; i < resourceTypeListExpected.size(); i++) {
            assertEquals(resourceTypeListExpected.get(i), resourceTypeListActual.get(i));
        }

        boolean thrown = false;
        try {
            representation.setResourceTypes(null);
        } catch (InvalidParameterException e) {
            thrown = true;
        }
        assertTrue(thrown);
    }

    public void testResourceInterfacesGetSet() {
        OcRepresentation representation = new OcRepresentation();

        List<String> emptyResourceInterfaceList = representation.getResourceInterfaces();
        assertTrue(emptyResourceInterfaceList.isEmpty());

        representation.setResourceInterfaces(emptyResourceInterfaceList);
        emptyResourceInterfaceList = representation.getResourceInterfaces();
        assertTrue(emptyResourceInterfaceList.isEmpty());

        List<String> resourceInterfaceListExpected = new LinkedList<String>();
        resourceInterfaceListExpected.add("Interface1");
        resourceInterfaceListExpected.add("Interface2");
        resourceInterfaceListExpected.add("Interface3");

        representation.setResourceInterfaces(resourceInterfaceListExpected);
        List<String> resourceInterfaceListActual = representation.getResourceInterfaces();
        assertEquals(resourceInterfaceListExpected.size(), resourceInterfaceListActual.size());
        for (int i = 0; i < resourceInterfaceListExpected.size(); i++) {
            assertEquals(resourceInterfaceListExpected.get(i), resourceInterfaceListActual.get(i));
        }

        boolean thrown = false;
        try {
            representation.setResourceInterfaces(null);
        } catch (InvalidParameterException e) {
            thrown = true;
        }
        assertTrue(thrown);
    }

    public void testAttributeManagement() {
        OcRepresentation representation = new OcRepresentation();

        assertTrue(representation.isEmpty());
        assertEquals(0, representation.size());

        try {
            String integerKey = "integerKey";
            int integerValue = 75;
            representation.setValue(integerKey, integerValue);
            assertFalse(representation.isEmpty());
            assertEquals(1, representation.size());

            int actualIntValue = representation.getValue(integerKey);
            assertEquals(integerValue, actualIntValue);

            String stringKey = "stringKey";
            String stringValue = "stringValue";
            representation.setValue(stringKey, stringValue);
            assertEquals(2, representation.size());

            assertTrue(representation.hasAttribute(integerKey));
            representation.remove(integerKey);
            assertFalse(representation.hasAttribute(integerKey));
            assertEquals(1, representation.size());

            representation.setValue(integerKey, integerValue);
            assertFalse(representation.isNull(integerKey));
            representation.setNull(integerKey);
            assertTrue(representation.isNull(integerKey));
        } catch (OcException e) {
            assertTrue(false);
        }

        String nonexistentKey = "nonexistentKey";
        assertFalse(representation.hasAttribute(nonexistentKey));
        representation.setNull(nonexistentKey);
        assertTrue(representation.isNull(nonexistentKey));

        String nonexistentKey2 = "nonexistentKey2";
        boolean thrown = false;
        try {
            boolean nonexistentValue = representation.getValue(nonexistentKey2);
        } catch (OcException e) {
            thrown = true;
        }
        assertTrue(thrown);
    }

    public void testAttributeAccessByType() throws OcException {
        OcRepresentation rep = new OcRepresentation();

        //null
        OcRepresentation repNull = null;
        rep.setValue("nullKey", repNull);
        OcRepresentation repNullActual = rep.getValue("nullKey");
        assertNull(repNullActual);

        //integer
        String intK = "intK";
        int intV = 4;
        rep.setValue(intK, intV);
        int intVa = rep.getValue(intK);
        assertEquals(intV, intVa);

        //double
        String doubleK = "doubleK";
        double doubleV = 4.5;
        rep.setValue(doubleK, doubleV);
        double doubleVa = rep.getValue(doubleK);
        assertEquals(doubleV, doubleVa);

        //boolean
        String booleanK = "booleanK";
        boolean booleanV = true;
        rep.setValue(booleanK, booleanV);
        boolean booleanVa = rep.getValue(booleanK);
        assertEquals(booleanV, booleanVa);

        //String
        String stringK = "stringK";
        String stringV = "stringV";
        rep.setValue(stringK, stringV);
        String stringVa = rep.getValue(stringK);
        assertEquals(stringV, stringVa);

        //OcRepresentation
        String repK = "repK";
        OcRepresentation repV = new OcRepresentation();
        repV.setValue(intK, intV);
        rep.setValue(repK, repV);
        OcRepresentation repVa = rep.getValue(repK);
        assertEquals(intV, repVa.getValue(intK));
    }

    public void testAttributeAccessBySequenceType() throws OcException {
        OcRepresentation rep = new OcRepresentation();

        //integer
        String intK = "intK";
        int[] intArrV = {1, 2, 3, 4};
        rep.setValue(intK, intArrV);
        int[] intArrVa = rep.getValue(intK);
        assertTrue(Arrays.equals(intArrV, intArrVa));

        int[] intArrVEmpty = {};
        rep.setValue(intK, intArrVEmpty);
        int[] intArrVEmptyA = rep.getValue(intK);
        assertTrue(Arrays.equals(intArrVEmpty, intArrVEmptyA));

        //double
        String doubleK = "doubleK";
        double[] doubleArrV = {1.1, 2.2, 3.3, 4.4};
        rep.setValue(doubleK, doubleArrV);
        double[] doubleArrVa = rep.getValue(doubleK);
        assertTrue(Arrays.equals(doubleArrV, doubleArrVa));

        double[] doubleArrVEmpty = {};
        rep.setValue(doubleK, doubleArrVEmpty);
        double[] doubleArrVEmptyA = rep.getValue(doubleK);
        assertTrue(Arrays.equals(doubleArrVEmpty, doubleArrVEmptyA));

        //boolean
        String booleanK = "booleanK";
        boolean[] booleanArrV = {true, false, true, false};
        rep.setValue(booleanK, booleanArrV);
        boolean[] booleanArrVa = rep.getValue(booleanK);
        assertTrue(Arrays.equals(booleanArrV, booleanArrVa));

        boolean[] booleanArrVEmpty = {};
        rep.setValue(booleanK, booleanArrVEmpty);
        boolean[] booleanArrVEmptyA = rep.getValue(booleanK);
        assertTrue(Arrays.equals(booleanArrVEmpty, booleanArrVEmptyA));

        //String
        String stringK = "stringK";
        String[] stringArrV = {"aaa", "bbb", "ccc", "ddd"};
        rep.setValue(stringK, stringArrV);
        String[] stringArrVa = rep.getValue(stringK);
        assertTrue(Arrays.equals(stringArrV, stringArrVa));

        String[] stringArrVEmpty = {};
        rep.setValue(stringK, stringArrVEmpty);
        String[] stringArrVEmptyA = rep.getValue(stringK);
        assertTrue(Arrays.equals(stringArrVEmpty, stringArrVEmptyA));

        //OcRepresentation
        String representationK = "representationK";
        OcRepresentation[] representationArrV = {
                new OcRepresentation(),
                new OcRepresentation(),
                new OcRepresentation(),
                new OcRepresentation()};
        representationArrV[0].setValue(intK, 0);
        representationArrV[1].setValue(intK, 1);
        representationArrV[2].setValue(intK, 2);
        representationArrV[3].setValue(intK, 3);

        rep.setValue(representationK, representationArrV);
        OcRepresentation[] representationArrVa = rep.getValue(representationK);

        assertEquals(representationArrV.length, representationArrVa.length);
        for (int i = 0; i < representationArrV.length; ++i) {
            assertEquals(representationArrV[i].getValue(intK),
                    representationArrVa[i].getValue(intK));
        }

        OcRepresentation[] representationArrVEmpty = {};
        rep.setValue(representationK, representationArrVEmpty);
        OcRepresentation[] representationArrVEmptyA = rep.getValue(representationK);
        assertEquals(representationArrVEmpty.length, representationArrVEmptyA.length);
    }

    public void testAttributeAccessBy2DType() throws OcException {
        OcRepresentation rep = new OcRepresentation();
        //integer
        String int2DK = "int2DK";
        int[] intArrV1 = {1, 2, 3, 4};
        int[] intArrV2 = {5, 6, 7, 8};
        int[][] int2DArrV = {intArrV1, intArrV2};
        rep.setValue(int2DK, int2DArrV);
        int[][] int2DArrVa = rep.getValue(int2DK);
        for (int i = 0; i < int2DArrV.length; i++) {
            assertTrue(Arrays.equals(int2DArrV[i], int2DArrVa[i]));
        }
        //double
        String double2DK = "double2DK";
        double[] doubleArrV1 = {1.1, 2.2, 3.3, 4.4};
        double[] doubleArrV2 = {5, 6, 7, 8};
        double[][] double2DArrV = {doubleArrV1, doubleArrV2};
        rep.setValue(double2DK, double2DArrV);
        double[][] double2DArrVa = rep.getValue(double2DK);
        for (int i = 0; i < double2DArrV.length; i++) {
            assertTrue(Arrays.equals(double2DArrV[i], double2DArrVa[i]));
        }
        double[][] double2DArrVEmpty = {{}};
        rep.setValue(double2DK, double2DArrVEmpty);
        double[][] double2DArrVEmptyA = rep.getValue(double2DK);
        for (int i = 0; i < double2DArrVEmpty.length; i++) {
            assertTrue(Arrays.equals(double2DArrVEmpty[i], double2DArrVEmptyA[i]));
        }
        //boolean
        String boolean2DK = "boolean2DK";
        boolean[] booleanArrV1 = {true, true, false};
        boolean[] booleanArrV2 = {true, false, false, true};
        boolean[][] boolean2DArrV = {booleanArrV1, booleanArrV2};
        rep.setValue(boolean2DK, boolean2DArrV);
        boolean[][] boolean2DArrVa = rep.getValue(boolean2DK);
        for (int i = 0; i < boolean2DArrV.length; i++) {
            assertTrue(Arrays.equals(boolean2DArrV[i], boolean2DArrVa[i]));
        }
        boolean[][] boolean2DArrVEmpty = {{}};
        rep.setValue(boolean2DK, boolean2DArrVEmpty);
        boolean[][] boolean2DArrVEmptyA = rep.getValue(boolean2DK);
        for (int i = 0; i < boolean2DArrVEmpty.length; i++) {
            assertTrue(Arrays.equals(boolean2DArrVEmpty[i], boolean2DArrVEmptyA[i]));
        }

        //String
        String string2DK = "string2DK";
        String[] stringArrV1 = {"aaa", "bbb", "ccc"};
        String[] stringArrV2 = {"111", "222", "333", "444"};
        String[][] string2DArrV = {stringArrV1, stringArrV2};
        rep.setValue(string2DK, string2DArrV);
        String[][] string2DArrVa = rep.getValue(string2DK);
        for (int i = 0; i < string2DArrV.length; i++) {
            assertTrue(Arrays.equals(string2DArrV[i], string2DArrVa[i]));
        }
        String[][] string2DArrVEmpty = {{}};
        rep.setValue(string2DK, string2DArrVEmpty);
        String[][] string2DArrVEmptyA = rep.getValue(string2DK);
        for (int i = 0; i < string2DArrVEmpty.length; i++) {
            assertTrue(Arrays.equals(string2DArrVEmpty[i], string2DArrVEmptyA[i]));
        }

        //OcRepresentation
        String intK = "intK";
        String representation2DK = "representation2DK";
        OcRepresentation[] representation2DArrV1 = {
                new OcRepresentation(),
                new OcRepresentation(),
                new OcRepresentation(),
                new OcRepresentation()};
        representation2DArrV1[0].setValue(intK, 0);
        representation2DArrV1[1].setValue(intK, 1);
        representation2DArrV1[2].setValue(intK, 2);
        representation2DArrV1[3].setValue(intK, 3);

        OcRepresentation[] representation2DArrV2 = {
                new OcRepresentation(),
                new OcRepresentation(),
                new OcRepresentation(),
                new OcRepresentation()};
        representation2DArrV2[0].setValue(intK, 4);
        representation2DArrV2[1].setValue(intK, 5);
        representation2DArrV2[2].setValue(intK, 6);
        representation2DArrV2[3].setValue(intK, 7);

        OcRepresentation[][] representation2DArrV = {representation2DArrV1, representation2DArrV2};
        rep.setValue(representation2DK, representation2DArrV);
        OcRepresentation[][] representation2DArrVa = rep.getValue(representation2DK);
        assertEquals(representation2DArrV.length, representation2DArrVa.length);
        for (int i = 0; i < representation2DArrV.length; ++i) {
            OcRepresentation[] repArrV = representation2DArrV[i];
            OcRepresentation[] repArrVa = representation2DArrVa[i];
            assertEquals(repArrV.length, repArrVa.length);
            for (int j = 0; j < representation2DArrV.length; ++j) {
                assertEquals(repArrV[j].getValue(intK),
                        repArrVa[j].getValue(intK));
            }
        }

        OcRepresentation[][] representation2DArrVEmpty = {{}};
        rep.setValue(representation2DK, representation2DArrVEmpty);
        OcRepresentation[][] representation2DArrVEmptyA = rep.getValue(representation2DK);
        assertEquals(representation2DArrVEmpty.length, representation2DArrVEmptyA.length);
    }

    public void testAttributeAccessBy3DType() throws OcException {
        OcRepresentation rep = new OcRepresentation();
        //integer
        String int3DK = "int3DK";
        int[] intArrV1 = {0, 1, 2, 3, 4};
        int[] intArrV2 = {5, 6, 7, 8};
        int[][] int2DArrV1 = {intArrV1, intArrV2};
        int[] intArrV3 = {9, 10};
        int[] intArrV4 = {11};
        int[][] int2DArrV2 = {intArrV3, intArrV4};
        int[][][] int3DArrV = {int2DArrV1, int2DArrV2};
        rep.setValue(int3DK, int3DArrV);
        int[][][] int3DArrVa = rep.getValue(int3DK);
        assertEquals(int3DArrV.length, int3DArrVa.length);
        for (int i = 0; i < int3DArrV.length; i++) {
            int[][] int2DT = int3DArrV[i];
            int[][] int2DTa = int3DArrVa[i];
            assertEquals(int2DT.length, int2DTa.length);
            for (int j = 0; j < int2DT.length; j++) {
                assertTrue(Arrays.equals(int2DT[j], int2DTa[j]));
            }
        }
        //double
        String double3DK = "double3DK";
        double[] doubleArrV1 = {0.0, 1.1, 2.2, 3.3, 4.4};
        double[] doubleArrV2 = {5.5, 6.6, 7.7, 8.8};
        double[][] double2DArrV1 = {doubleArrV1, doubleArrV2};
        double[] doubleArrV3 = {9.9, 10.1};
        double[] doubleArrV4 = {11.1};
        double[][] double2DArrV2 = {doubleArrV3, doubleArrV4};
        double[][][] double3DArrV = {double2DArrV1, double2DArrV2};
        rep.setValue(double3DK, double3DArrV);
        double[][][] double3DArrVa = rep.getValue(double3DK);
        assertEquals(double3DArrV.length, double3DArrVa.length);
        for (int i = 0; i < double3DArrV.length; i++) {
            double[][] double2DT = double3DArrV[i];
            double[][] double2DTa = double3DArrVa[i];
            assertEquals(double2DT.length, double2DTa.length);
            for (int j = 0; j < double2DT.length; j++) {
                assertTrue(Arrays.equals(double2DT[j], double2DTa[j]));
            }
        }
        double[][][] double3DArrVEmpty = {};
        rep.setValue(double3DK, double3DArrVEmpty);
        double[][][] double3DArrVEmptyA = rep.getValue(double3DK);
        assertEquals(double3DArrVEmpty.length, double3DArrVEmptyA.length);
        for (int i = 0; i < double3DArrVEmpty.length; i++) {
            double[][] double2DT = double3DArrVEmpty[i];
            double[][] double2DTa = double3DArrVEmptyA[i];
            assertEquals(double2DT.length, double2DTa.length);
            for (int j = 0; j < double2DT.length; j++) {
                assertTrue(Arrays.equals(double2DT[j], double2DTa[j]));
            }
        }

        //boolean
        String boolean3DK = "boolean3DK";
        boolean[] booleanArrV1 = {true, false, true, true, false};
        boolean[] booleanArrV2 = {false, false, false, true};
        boolean[][] boolean2DArrV1 = {booleanArrV1, booleanArrV2};
        boolean[] booleanArrV3 = {true, true};
        boolean[] booleanArrV4 = {false};
        boolean[][] boolean2DArrV2 = {booleanArrV3, booleanArrV4};
        boolean[][][] boolean3DArrV = {boolean2DArrV1, boolean2DArrV2};
        rep.setValue(boolean3DK, boolean3DArrV);
        boolean[][][] boolean3DArrVa = rep.getValue(boolean3DK);
        assertEquals(boolean3DArrV.length, boolean3DArrVa.length);
        for (int i = 0; i < boolean3DArrV.length; i++) {
            boolean[][] boolean2DT = boolean3DArrV[i];
            boolean[][] boolean2DTa = boolean3DArrVa[i];
            assertEquals(boolean2DT.length, boolean2DTa.length);
            for (int j = 0; j < boolean2DT.length; j++) {
                assertTrue(Arrays.equals(boolean2DT[j], boolean2DTa[j]));
            }
        }
        boolean[][][] boolean3DArrVEmpty = {};
        rep.setValue(boolean3DK, boolean3DArrVEmpty);
        boolean[][][] boolean3DArrVEmptyA = rep.getValue(boolean3DK);
        assertEquals(boolean3DArrVEmpty.length, boolean3DArrVEmptyA.length);
        for (int i = 0; i < boolean3DArrVEmpty.length; i++) {
            boolean[][] boolean2DT = boolean3DArrVEmpty[i];
            boolean[][] boolean2DTa = boolean3DArrVEmptyA[i];
            assertEquals(boolean2DT.length, boolean2DTa.length);
            for (int j = 0; j < boolean2DT.length; j++) {
                assertTrue(Arrays.equals(boolean2DT[j], boolean2DTa[j]));
            }
        }

        //String
        String string3DK = "string3DK";
        String[] stringArrV1 = {"a", "bb", "ccc", "dddd", "eeee"};
        String[] stringArrV2 = {"f", "gg", "hhh", "ii"};
        String[][] string2DArrV1 = {stringArrV1, stringArrV2};
        String[] stringArrV3 = {"j", "jj"};
        String[] stringArrV4 = {"jjj"};
        String[][] string2DArrV2 = {stringArrV3, stringArrV4};
        String[][][] string3DArrV = {string2DArrV1, string2DArrV2};
        rep.setValue(string3DK, string3DArrV);
        String[][][] string3DArrVa = rep.getValue(string3DK);
        assertEquals(string3DArrV.length, string3DArrVa.length);
        for (int i = 0; i < string3DArrV.length; i++) {
            String[][] string2DT = string3DArrV[i];
            String[][] string2DTa = string3DArrVa[i];
            assertEquals(string2DT.length, string2DTa.length);
            for (int j = 0; j < string2DT.length; j++) {
                assertTrue(Arrays.equals(string2DT[j], string2DTa[j]));
            }
        }
        String[][][] string3DArrVEmpty = {};
        rep.setValue(string3DK, string3DArrVEmpty);
        String[][][] string3DArrVEmptyA = rep.getValue(string3DK);
        assertEquals(string3DArrVEmpty.length, string3DArrVEmptyA.length);
        for (int i = 0; i < string3DArrVEmpty.length; i++) {
            String[][] string2DT = string3DArrVEmpty[i];
            String[][] string2DTa = string3DArrVEmptyA[i];
            assertEquals(string2DT.length, string2DTa.length);
            for (int j = 0; j < string2DT.length; j++) {
                assertTrue(Arrays.equals(string2DT[j], string2DTa[j]));
            }
        }

        //OcRepresentation
        String intK = "intK";
        String representation3DK = "representation3DK";
        OcRepresentation[] representation2DArrV1 = {
                new OcRepresentation(),
                new OcRepresentation(),
                new OcRepresentation(),
                new OcRepresentation()};
        representation2DArrV1[0].setValue(intK, 0);
        representation2DArrV1[1].setValue(intK, 1);
        representation2DArrV1[2].setValue(intK, 2);
        representation2DArrV1[3].setValue(intK, 3);

        OcRepresentation[] representation2DArrV2 = {
                new OcRepresentation(),
                new OcRepresentation(),
                new OcRepresentation(),
                new OcRepresentation()};
        representation2DArrV2[0].setValue(intK, 4);
        representation2DArrV2[1].setValue(intK, 5);
        representation2DArrV2[2].setValue(intK, 6);
        representation2DArrV2[3].setValue(intK, 7);

        OcRepresentation[][] representation2DArrV = {representation2DArrV1, representation2DArrV2};
        OcRepresentation[][][] representation3DArrV = {representation2DArrV, representation2DArrV};

        rep.setValue(representation3DK, representation3DArrV);
        OcRepresentation[][][] representation3DArrVa = rep.getValue(representation3DK);
        assertEquals(representation3DArrV.length, representation3DArrVa.length);
        for (int i = 0; i < representation3DArrV.length; ++i) {
            OcRepresentation[][] repArr2V = representation3DArrV[i];
            OcRepresentation[][] repArr2Va = representation3DArrVa[i];
            assertEquals(repArr2V.length, repArr2Va.length);
            for (int j = 0; j < repArr2V.length; ++j) {
                OcRepresentation[] repArrV = repArr2V[j];
                OcRepresentation[] repArrVa = repArr2Va[j];
                assertEquals(repArrV.length, repArrVa.length);
                for (int k = 0; k < repArrV.length; ++k) {
                    assertEquals(repArrV[k].getValue(intK), repArrVa[k].getValue(intK));
                }
            }
        }
    }
}