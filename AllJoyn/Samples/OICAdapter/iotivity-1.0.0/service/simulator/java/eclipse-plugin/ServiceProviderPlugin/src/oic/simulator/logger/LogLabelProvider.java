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

package oic.simulator.logger;

import java.text.DateFormat;
import java.text.SimpleDateFormat;

import oic.simulator.serviceprovider.manager.LogManager;

import org.eclipse.jface.viewers.ITableLabelProvider;
import org.eclipse.jface.viewers.LabelProvider;
import org.eclipse.swt.graphics.Image;

/**
 * Label provider which determines what data has to be shown in the log view.
 */
public class LogLabelProvider extends LabelProvider implements
        ITableLabelProvider {

    DateFormat dateFormat = new SimpleDateFormat("HH:mm:ss.SSS");

    @Override
    public Image getColumnImage(Object element, int columnIndex) {
        if (columnIndex == 0) {
            LogEntry entry = (LogEntry) element;
            return LogManager.getSeverityIcon(entry.getSeverity());
        }
        return null;
    }

    @Override
    public String getColumnText(Object element, int columnIndex) {
        LogEntry entry = (LogEntry) element;
        if (columnIndex == 0) {
            return LogManager.getSeverityName(entry.getSeverity());
        } else if (columnIndex == 1) {
            return dateFormat.format(entry.getDate());
        } else {
            return entry.getMessage();
        }
    }

}