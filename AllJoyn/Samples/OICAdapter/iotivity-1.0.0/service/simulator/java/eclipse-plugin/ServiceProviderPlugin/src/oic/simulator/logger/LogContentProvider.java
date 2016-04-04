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

import java.util.ArrayList;
import java.util.List;

import org.eclipse.jface.viewers.ITreeContentProvider;
import org.eclipse.jface.viewers.Viewer;

/**
 * Maintains simulator log entries and provides content to the log view.
 */
public class LogContentProvider implements ITreeContentProvider {

    List<LogEntry> logEntryList = new ArrayList<LogEntry>();

    @SuppressWarnings("unchecked")
    @Override
    public void inputChanged(Viewer viewer, Object oldInput, Object newInput) {
        logEntryList = (List<LogEntry>) newInput;
    }

    @Override
    public Object[] getChildren(Object element) {
        return new Object[0];
    }

    @Override
    public Object[] getElements(Object input) {
        return logEntryList.toArray();
    }

    @Override
    public Object getParent(Object element) {
        return null;
    }

    @Override
    public boolean hasChildren(Object element) {
        return false;
    }

    @Override
    public void dispose() {
    }

    public synchronized void addLog(LogEntry newElement) {
        logEntryList.add(newElement);
    }
}