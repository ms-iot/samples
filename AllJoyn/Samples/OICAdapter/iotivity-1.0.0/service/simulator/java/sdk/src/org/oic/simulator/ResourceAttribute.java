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

package org.oic.simulator;

/**
 * This class represents an attribute of a resource. It has a set of native
 * methods for getting the attribute's information.
 */
public class ResourceAttribute {
    /**
     * Type of attribute value.
     */
    public enum Type {
        UNKNOWN, INT, DOUBLE, BOOL, STRING;

        private static Type[] m_cvalues = Type.values();

        @SuppressWarnings("unused")
        private static Type getType(int x) {
            return m_cvalues[x];
        }
    };

    /**
     * Class contains range property in min and max value.
     */
    public class Range {
        public int getMin() {
            return m_min;
        }

        public int getMax() {
            return m_max;
        }

        private Range(int min, int max) {
            m_min = min;
            m_max = max;
        }

        private int m_min;
        private int m_max;
    }

    @SuppressWarnings("unused")
    private void setRange(int min, int max) {
        m_range = new Range(min, max);
    }

    /**
     * This generic API is used to get the value of an attribute whose type is
     * given by the caller of the method.
     *
     * @param <T>
     *            This specifies the type in which the value has to be returned.
     *
     * @return The attribute's value in a specified type.
     */
    public <T> T getValue() {
        @SuppressWarnings("unchecked")
        T t = (T) m_value;
        return t;
    }

    /**
     * Method for getting the attribute's name.
     *
     * @return Attribute's name
     */
    public String getName() {
        return m_name;
    }

    /**
     * Method for getting the attribute's value type.
     *
     * @return Attribute's value type as {@link Type}
     */
    public Type getType() {
        return m_type;
    }

    /**
     * Method for getting the attribute's value base type. For example If the
     * attribute value object is of type Vector of {@link Integer} then its type
     * is Vector and base type is INT.
     *
     * @return Attribute's value type as {@link Type}
     */
    public Type getBaseType() {
        return m_type;
    }

    /**
     * Method for getting the attribute's range property. Range will be valid
     * only for Integer type.
     *
     * @return Attribute's value range as {@link Range}.
     */
    public Range getRange() {
        return m_range;
    }

    /**
     * Method for getting the attribute's allowed values property. Allowed
     * values property will be valid only for Integer, Double, String types.
     *
     * @param <T>
     *            Attribute's allowed values whose type is given by the caller
     *            of the method.
     *
     * @return Attribute's value range as {@link Range}.
     */
    public <T> T getAllowedValues() {
        @SuppressWarnings("unchecked")
        T t = (T) m_AllowedValues;
        return t;
    }

    private String m_name          = null;
    private Object m_value         = null;
    private Type   m_type          = Type.STRING;
    private Range  m_range         = null;
    private Object m_AllowedValues = null;
}