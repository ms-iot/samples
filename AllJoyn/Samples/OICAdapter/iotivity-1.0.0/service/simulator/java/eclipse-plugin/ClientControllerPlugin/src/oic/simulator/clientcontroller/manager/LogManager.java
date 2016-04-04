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

package oic.simulator.clientcontroller.manager;

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.LinkedList;

import oic.simulator.clientcontroller.Activator;
import oic.simulator.clientcontroller.listener.ILogUIListener;
import oic.simulator.clientcontroller.utils.Constants;
import oic.simulator.logger.LogEntry;
import oic.simulator.logger.LoggerCallback;

import org.eclipse.jface.resource.ImageRegistry;
import org.eclipse.swt.graphics.Image;
import org.oic.simulator.ILogger;
import org.oic.simulator.ILogger.Level;
import org.oic.simulator.SimulatorManager;

/**
 * Class which handles the native logs, maintains log entries and updates the
 * UI.
 */
public class LogManager {
    private LinkedList<LogEntry>         entries           = new LinkedList<LogEntry>();
    private ArrayList<ILogUIListener>    listeners         = new ArrayList<ILogUIListener>();
    private LinkedList<LogEntry>         visibleEntries    = new LinkedList<LogEntry>();
    private HashMap<Integer, Boolean>    visibleSeverities = new HashMap<Integer, Boolean>();

    private ILogger                      logger;
    private LogManagerSynchronizerThread synchronizerThread;
    private Thread                       threadHandle;

    public LogManager() {
        synchronizerThread = new LogManagerSynchronizerThread();
        threadHandle = new Thread(synchronizerThread);
        threadHandle.setName("OIC Simulator event queue");
        threadHandle.start();

        // Set the logger callback with the native layer
        logger = new LoggerCallback();
        SimulatorManager.setLogger(logger);
    }

    private static class LogManagerSynchronizerThread implements Runnable {

        LinkedList<Runnable> eventQueue = new LinkedList<Runnable>();

        @Override
        public void run() {
            while (!Thread.interrupted()) {

                synchronized (this) {
                    try {
                        while (eventQueue.isEmpty()) {
                            this.wait();
                            break;
                        }
                    } catch (InterruptedException e) {
                        return;
                    }
                }

                Runnable pop;
                synchronized (this) {
                    pop = eventQueue.pop();
                }
                try {
                    pop.run();
                } catch (Exception e) {
                    if (e instanceof InterruptedException) {
                        return;
                    }
                    e.printStackTrace();
                }
            }
        }

        public void addToQueue(Runnable event) {
            synchronized (this) {
                eventQueue.add(event);
                this.notify();
            }
        }
    }

    public void log(int severity, Date date, String msg) {
        final LogEntry logEntry = new LogEntry(severity, date, msg);
        synchronizerThread.addToQueue(new Runnable() {
            @Override
            public void run() {
                boolean notify = false;
                synchronized (entries) {
                    entries.add(logEntry);
                    Boolean showEntry = LogManager.this.visibleSeverities
                            .get(logEntry.getSeverity());
                    if (showEntry != null) {
                        if (showEntry) {
                            visibleEntries.add(logEntry);
                            notify = true;
                        }
                    }
                    if (entries.size() > Constants.LOG_SIZE) {
                        entries.pop();
                    }
                    if (visibleEntries.size() > Constants.LOG_SIZE) {
                        visibleEntries.pop();
                        notify = true;
                    }
                }
                if (notify) {
                    notifyListeners(logEntry);
                }
            }
        });
    }

    public void applyFilter(final HashMap<Integer, Boolean> visibleSeverities) {
        synchronizerThread.addToQueue(new Runnable() {

            @Override
            public void run() {
                LinkedList<LogEntry> newLogs = new LinkedList<LogEntry>();
                synchronized (entries) {
                    LogManager.this.visibleSeverities = visibleSeverities;
                    for (LogEntry logEntry : entries) {
                        if (LogManager.this.visibleSeverities.get(logEntry
                                .getSeverity())) {
                            newLogs.add(logEntry);
                        }
                    }
                }
                visibleEntries = newLogs;
                notifyListeners();
            }
        });

    }

    private void notifyListeners() {
        for (ILogUIListener l : listeners) {
            l.logChanged(new ArrayList<LogEntry>(visibleEntries));
        }
    }

    private void notifyListeners(LogEntry added) {
        for (ILogUIListener l : listeners) {
            l.logAdded(added);
        }
    }

    public void clearLog() {
        synchronizerThread.addToQueue(new Runnable() {

            @Override
            public void run() {
                synchronized (entries) {
                    entries = new LinkedList<LogEntry>();
                    visibleEntries = new LinkedList<LogEntry>();
                }
                notifyListeners();
            }
        });
    }

    public void removeEntry(final LogEntry element) {
        synchronizerThread.addToQueue(new Runnable() {

            @Override
            public void run() {
                synchronized (entries) {
                    entries.remove(element);
                    visibleEntries.remove(element);
                }
                notifyListeners();
            }
        });
    }

    public ArrayList<LogEntry> getLogEntries() {
        synchronized (entries) {
            return new ArrayList<LogEntry>(entries);
        }
    }

    public void addLogListener(final ILogUIListener listener) {
        synchronizerThread.addToQueue(new Runnable() {
            @Override
            public void run() {
                if (!listeners.contains(listener)) {
                    listeners.add(listener);
                }
            }
        });
    }

    public void removeLogListener(final ILogUIListener listener) {
        synchronizerThread.addToQueue(new Runnable() {
            @Override
            public void run() {
                if (!listeners.contains(listener)) {
                    listeners.remove(listener);
                }
            }
        });
    }

    public void shutdown() {
        threadHandle.interrupt();
    }

    public static String getSeverityName(int severity) {
        if (severity == Level.INFO.ordinal()) {
            return Constants.INFO;
        } else if (severity == Level.WARNING.ordinal()) {
            return Constants.WARNING;
        } else if (severity == Level.ERROR.ordinal()) {
            return Constants.ERROR;
        } else if (severity == Level.DEBUG.ordinal()) {
            return Constants.DEBUG;
        } else {
            return Constants.UNKNOWN;
        }
    }

    public static Image getSeverityIcon(int severity) {
        ImageRegistry r = Activator.getDefault().getImageRegistry();
        if (severity == Level.INFO.ordinal()) {
            return r.get(Constants.INFO_LOG);
        } else if (severity == Level.WARNING.ordinal()) {
            return r.get(Constants.WARNING_LOG);
        } else if (severity == Level.ERROR.ordinal()) {
            return r.get(Constants.ERROR_LOG);
        } else if (severity == Level.DEBUG.ordinal()) {
            return r.get(Constants.DEBUG_LOG);
        } else {
            return r.get(Constants.UNKNOWN_LOG);
        }
    }
}