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

package oic.simulator.serviceprovider.view;

import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import oic.simulator.serviceprovider.Activator;
import oic.simulator.serviceprovider.listener.IObserverListChangedUIListener;
import oic.simulator.serviceprovider.listener.IResourceSelectionChangedUIListener;
import oic.simulator.serviceprovider.manager.ResourceManager;
import oic.simulator.serviceprovider.resource.ObserverDetail;
import oic.simulator.serviceprovider.resource.SimulatorResource;
import oic.simulator.serviceprovider.utils.Constants;

import org.eclipse.jface.viewers.CellEditor;
import org.eclipse.jface.viewers.CheckboxCellEditor;
import org.eclipse.jface.viewers.ColumnLabelProvider;
import org.eclipse.jface.viewers.EditingSupport;
import org.eclipse.jface.viewers.IStructuredContentProvider;
import org.eclipse.jface.viewers.StyledCellLabelProvider;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.jface.viewers.TableViewerColumn;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.viewers.ViewerCell;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Table;
import org.eclipse.ui.part.ViewPart;

/**
 * This class manages and shows the resource observer view in the perspective.
 */
public class ResourceObserverView extends ViewPart {
    public static final String                  VIEW_ID       = "oic.simulator.serviceprovider.view.observer";

    private TableViewer                         tblViewer;

    private final String[]                      columnHeaders = {
            "Client Address", "Port", "Notify"               };

    private final Integer[]                     columnWidth   = { 150, 75, 50 };

    private IResourceSelectionChangedUIListener resourceSelectionChangedListener;

    private IObserverListChangedUIListener      resourceObserverListChangedListener;

    private ResourceManager                     resourceManagerRef;

    public ResourceObserverView() {

        resourceManagerRef = Activator.getDefault().getResourceManager();

        resourceSelectionChangedListener = new IResourceSelectionChangedUIListener() {

            @Override
            public void onResourceSelectionChange() {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {
                        if (null != tblViewer) {
                            changeButtonStatus();
                            updateViewer(getData(resourceManagerRef
                                    .getCurrentResourceInSelection()));
                        }
                    }
                });
            }
        };

        resourceObserverListChangedListener = new IObserverListChangedUIListener() {

            @Override
            public void onObserverListChanged(final String resourceURI) {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {
                        if (null == resourceURI) {
                            return;
                        }
                        SimulatorResource resource = resourceManagerRef
                                .getCurrentResourceInSelection();
                        if (null == resource) {
                            return;
                        }
                        if (resource.getResourceURI().equals(resourceURI)) {
                            if (null != tblViewer) {
                                updateViewer(getData(resource));
                            }
                        }
                    }
                });

            }
        };
    }

    private Map<Integer, ObserverDetail> getData(SimulatorResource resource) {
        if (null == resource) {
            return null;
        }
        return resource.getObserver();
    }

    private void updateViewer(Map<Integer, ObserverDetail> observer) {
        if (null != tblViewer) {
            Table tbl = tblViewer.getTable();
            if (null != observer && observer.size() > 0) {
                tblViewer.setInput(observer.entrySet().toArray());
                if (!tbl.isDisposed()) {
                    tbl.setLinesVisible(true);
                }
            } else {
                if (!tbl.isDisposed()) {
                    tbl.removeAll();
                    tbl.setLinesVisible(false);
                }
            }
        }
    }

    @Override
    public void createPartControl(Composite parent) {
        parent.setLayout(new GridLayout(1, false));

        tblViewer = new TableViewer(parent, SWT.SINGLE | SWT.H_SCROLL
                | SWT.V_SCROLL | SWT.FULL_SELECTION | SWT.BORDER);

        createColumns(tblViewer);

        // make lines and header visible
        final Table table = tblViewer.getTable();
        table.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
        table.setHeaderVisible(true);
        table.setLinesVisible(true);

        tblViewer.setContentProvider(new ObserverContentProvider());

        addManagerListeners();

        // Check whether there is any resource selected already
        Map<Integer, ObserverDetail> observerList = getData(resourceManagerRef
                .getCurrentResourceInSelection());
        if (null != observerList) {
            updateViewer(observerList);
        }
    }

    public void createColumns(TableViewer tableViewer) {
        TableViewerColumn addressColumn = new TableViewerColumn(tableViewer,
                SWT.NONE);
        addressColumn.getColumn().setWidth(columnWidth[0]);
        addressColumn.getColumn().setText(columnHeaders[0]);
        addressColumn.setLabelProvider(new StyledCellLabelProvider() {
            @Override
            public void update(ViewerCell cell) {
                Object element = cell.getElement();
                if (element instanceof Map.Entry) {
                    @SuppressWarnings("unchecked")
                    Map.Entry<Integer, ObserverDetail> observer = (Map.Entry<Integer, ObserverDetail>) element;
                    cell.setText(observer.getValue().getObserverInfo()
                            .getAddress());
                }
            }
        });

        TableViewerColumn portColumn = new TableViewerColumn(tableViewer,
                SWT.NONE);
        portColumn.getColumn().setWidth(columnWidth[1]);
        portColumn.getColumn().setText(columnHeaders[1]);
        portColumn.setLabelProvider(new StyledCellLabelProvider() {
            @Override
            public void update(ViewerCell cell) {
                Object element = cell.getElement();
                if (element instanceof Map.Entry) {
                    @SuppressWarnings("unchecked")
                    Map.Entry<Integer, ObserverDetail> observer = (Map.Entry<Integer, ObserverDetail>) element;
                    cell.setText(String.valueOf(observer.getValue()
                            .getObserverInfo().getPort()));
                }
            }
        });

        TableViewerColumn notifyColumn = new TableViewerColumn(tableViewer,
                SWT.NONE);
        notifyColumn.getColumn().setWidth(columnWidth[2]);
        notifyColumn.getColumn().setText(columnHeaders[2]);
        notifyColumn.setLabelProvider(new ColumnLabelProvider() {

            @Override
            public String getText(Object element) {
                return "";
            }

            @Override
            public Image getImage(Object element) {
                @SuppressWarnings("unchecked")
                Map.Entry<Integer, ObserverDetail> observer = (Map.Entry<Integer, ObserverDetail>) element;
                if (observer.getValue().isClicked()) {
                    return Activator.getDefault().getImageRegistry()
                            .get(Constants.NOTIFY_BUTTON_SELECTED);
                }
                return Activator.getDefault().getImageRegistry()
                        .get(Constants.NOTIFY_BUTTON_UNSELECTED);
            }
        });
        notifyColumn.setEditingSupport(new NotifyEditor(tableViewer));
    }

    private void addManagerListeners() {
        resourceManagerRef
                .addResourceSelectionChangedUIListener(resourceSelectionChangedListener);
        resourceManagerRef
                .addObserverListChangedUIListener(resourceObserverListChangedListener);
    }

    class ObserverContentProvider implements IStructuredContentProvider {

        @Override
        public void dispose() {
        }

        @Override
        public void inputChanged(Viewer arg0, Object arg1, Object arg2) {
        }

        @Override
        public Object[] getElements(Object element) {
            return (Object[]) element;
        }

    }

    class NotifyEditor extends EditingSupport {

        private final TableViewer viewer;

        public NotifyEditor(TableViewer viewer) {
            super(viewer);
            this.viewer = viewer;
        }

        @Override
        protected boolean canEdit(Object arg0) {
            return true;
        }

        @Override
        protected CellEditor getCellEditor(Object element) {
            return new CheckboxCellEditor(null, SWT.CHECK | SWT.READ_ONLY);
        }

        @Override
        protected Object getValue(Object element) {
            System.out.println("getValue()");
            @SuppressWarnings("unchecked")
            Map.Entry<Integer, ObserverDetail> observer = (Map.Entry<Integer, ObserverDetail>) element;
            return observer.getValue().isClicked();
        }

        @Override
        protected void setValue(Object element, Object value) {
            System.out.println("setValue()");
            // Change the button status of all the resources
            changeButtonStatus();

            @SuppressWarnings("unchecked")
            Map.Entry<Integer, ObserverDetail> observer = (Map.Entry<Integer, ObserverDetail>) element;
            observer.getValue().setClicked(true);
            viewer.refresh();

            // Call Native Method
            resourceManagerRef.notifyObserverRequest(
                    resourceManagerRef.getCurrentResourceInSelection(),
                    observer.getValue().getObserverInfo().getId());
        }
    }

    private void changeButtonStatus() {
        SimulatorResource resource = resourceManagerRef
                .getCurrentResourceInSelection();
        if (null == resource) {
            return;
        }
        Map<Integer, ObserverDetail> observerMap = resource.getObserver();
        if (null == observerMap) {
            return;
        }
        Set<Integer> keySet = observerMap.keySet();
        Iterator<Integer> itr = keySet.iterator();
        while (itr.hasNext()) {
            observerMap.get(itr.next()).setClicked(false);
        }
    }

    @Override
    public void dispose() {
        // Unregister the listener
        if (null != resourceSelectionChangedListener) {
            resourceManagerRef
                    .removeResourceSelectionChangedUIListener(resourceSelectionChangedListener);
        }

        if (null != resourceObserverListChangedListener) {
            resourceManagerRef
                    .removeObserverListChangedUIListener(resourceObserverListChangedListener);
        }
        super.dispose();
    }

    @Override
    public void setFocus() {
    }

}
