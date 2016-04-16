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

package oic.simulator.clientcontroller.view;

import java.util.List;

import oic.simulator.clientcontroller.Activator;
import oic.simulator.clientcontroller.listener.IResourceSelectionChangedUIListener;
import oic.simulator.clientcontroller.manager.ResourceManager;
import oic.simulator.clientcontroller.remoteresource.MetaProperty;
import oic.simulator.clientcontroller.remoteresource.RemoteResource;

import org.eclipse.jface.viewers.ColumnLabelProvider;
import org.eclipse.jface.viewers.IStructuredContentProvider;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.jface.viewers.TableViewerColumn;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Table;
import org.eclipse.ui.part.ViewPart;

/**
 * This class manages and shows the meta properties view in the perspective.
 */
public class MetaPropertiesView extends ViewPart {

    public static final String                  VIEW_ID       = "oic.simulator.clientcontroller.view.metaproperties";

    private TableViewer                         tableViewer;

    private final String[]                      columnHeaders = { "Property",
            "Value"                                          };

    private final Integer[]                     columnWidth   = { 150, 150 };

    private IResourceSelectionChangedUIListener resourceSelectionChangedListener;

    private ResourceManager                     resourceManager;

    public MetaPropertiesView() {

        resourceManager = Activator.getDefault().getResourceManager();

        resourceSelectionChangedListener = new IResourceSelectionChangedUIListener() {

            @Override
            public void onResourceSelectionChange(final RemoteResource resource) {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {
                        if (null != tableViewer) {
                            updateViewer(getData(resource));
                        }
                    }
                });
            }
        };
    }

    @Override
    public void createPartControl(Composite parent) {
        parent.setLayout(new GridLayout(1, false));

        tableViewer = new TableViewer(parent, SWT.SINGLE | SWT.H_SCROLL
                | SWT.V_SCROLL | SWT.FULL_SELECTION | SWT.BORDER);

        createColumns(tableViewer);

        // make lines and header visible
        final Table table = tableViewer.getTable();
        table.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
        table.setHeaderVisible(true);
        table.setLinesVisible(true);

        tableViewer.setContentProvider(new PropertycontentProvider());

        addManagerListeners();

        // Check whether there is any resource selected already
        List<MetaProperty> propertyList = getData(null);
        if (null != propertyList) {
            updateViewer(propertyList);
        }

    }

    private List<MetaProperty> getData(RemoteResource resource) {
        if (null == resource) {
            resource = Activator.getDefault().getResourceManager()
                    .getCurrentResourceInSelection();
        }
        List<MetaProperty> metaPropertyList = resourceManager
                .getMetaProperties(resource);
        return metaPropertyList;
    }

    private void updateViewer(List<MetaProperty> metaPropertyList) {
        if (null != tableViewer) {
            Table tbl = tableViewer.getTable();
            if (null != metaPropertyList) {
                tableViewer.setInput(metaPropertyList.toArray());
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

    public void createColumns(TableViewer tableViewer) {
        TableViewerColumn propName = new TableViewerColumn(tableViewer,
                SWT.NONE);
        propName.getColumn().setWidth(columnWidth[0]);
        propName.getColumn().setText(columnHeaders[0]);
        propName.setLabelProvider(new ColumnLabelProvider() {
            @Override
            public String getText(Object element) {
                MetaProperty prop = (MetaProperty) element;
                if (null != prop) {
                    return prop.getPropName();
                } else {
                    return "";
                }
            }
        });

        TableViewerColumn propValue = new TableViewerColumn(tableViewer,
                SWT.NONE);
        propValue.getColumn().setWidth(columnWidth[1]);
        propValue.getColumn().setText(columnHeaders[1]);
        propValue.setLabelProvider(new ColumnLabelProvider() {
            @Override
            public String getText(Object element) {
                MetaProperty prop = (MetaProperty) element;
                if (null != prop) {
                    return prop.getPropValue();
                } else {
                    return "";
                }
            }
        });
    }

    private void addManagerListeners() {
        resourceManager
                .addResourceSelectionChangedUIListener(resourceSelectionChangedListener);
    }

    class PropertycontentProvider implements IStructuredContentProvider {

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

    @Override
    public void dispose() {
        // Unregister the listener
        if (null != resourceSelectionChangedListener) {
            resourceManager
                    .removeResourceSelectionChangedUIListener(resourceSelectionChangedListener);
        }
        super.dispose();
    }

    @Override
    public void setFocus() {
    }
}