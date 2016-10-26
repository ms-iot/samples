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

import java.util.List;
import java.util.Set;

import oic.simulator.serviceprovider.Activator;
import oic.simulator.serviceprovider.listener.IAutomationUIListener;
import oic.simulator.serviceprovider.listener.IResourceModelChangedUIListener;
import oic.simulator.serviceprovider.listener.IResourceSelectionChangedUIListener;
import oic.simulator.serviceprovider.manager.ResourceManager;
import oic.simulator.serviceprovider.resource.LocalResourceAttribute;
import oic.simulator.serviceprovider.resource.ModelChangeNotificationType;
import oic.simulator.serviceprovider.resource.SimulatorResource;
import oic.simulator.serviceprovider.utils.Constants;

import org.eclipse.jface.viewers.ColumnLabelProvider;
import org.eclipse.jface.viewers.IStructuredContentProvider;
import org.eclipse.jface.viewers.StyledCellLabelProvider;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.jface.viewers.TableViewerColumn;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.viewers.ViewerCell;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Table;
import org.eclipse.ui.part.ViewPart;

/**
 * This class manages and shows the attribute view in the perspective.
 */
public class AttributeView extends ViewPart {

    public static final String                  VIEW_ID        = "oic.simulator.serviceprovider.view.attribute";

    private TableViewer                         attTblViewer;

    private AttributeEditingSupport             attributeEditor;

    private IResourceSelectionChangedUIListener resourceSelectionChangedListener;
    private IResourceModelChangedUIListener     resourceModelChangedUIListener;
    private IAutomationUIListener               automationUIListener;

    private final String[]                      attTblHeaders  = { "Name",
            "Value", "Automation"                             };
    private final Integer[]                     attTblColWidth = { 150, 190,
            150                                               };

    private ResourceManager                     resourceManager;

    public AttributeView() {

        resourceManager = Activator.getDefault().getResourceManager();

        resourceSelectionChangedListener = new IResourceSelectionChangedUIListener() {

            @Override
            public void onResourceSelectionChange() {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {
                        if (null != attTblViewer) {
                            updateViewer(getData());
                            SimulatorResource resource = resourceManager
                                    .getCurrentResourceInSelection();
                            Table tbl = attTblViewer.getTable();
                            if (!tbl.isDisposed()) {
                                if (null != resource
                                        && resource
                                                .isResourceAutomationInProgress()) {
                                    tbl.setEnabled(false);
                                } else {
                                    tbl.setEnabled(true);
                                }
                            }
                        }
                    }
                });
            }
        };

        resourceModelChangedUIListener = new IResourceModelChangedUIListener() {

            @Override
            public void onResourceModelChange(
                    final ModelChangeNotificationType notificationType,
                    final String resourceURI,
                    final Set<LocalResourceAttribute> valueChangeSet) {
                Display.getDefault().asyncExec(new Runnable() {
                    @Override
                    public void run() {
                        // Handle the notification only if it is for the current
                        // resource in selection
                        SimulatorResource resource = resourceManager
                                .getCurrentResourceInSelection();
                        if (null == resource) {
                            return;
                        }
                        if (!resourceURI.equals(resource.getResourceURI())) {
                            // This notification is for a different resource
                            // whose attributes are not
                            // currently not being shown in UI. So ignoring this
                            // notification.
                            return;
                        }
                        // Refresh the table viewers which will display
                        // the updated values
                        if (null != attTblViewer) {
                            if (notificationType == ModelChangeNotificationType.ATTRIBUTE_ADDED
                                    || notificationType == ModelChangeNotificationType.ATTRIBUTE_REMOVED) {
                                updateViewer(getData());
                            } else if (notificationType == ModelChangeNotificationType.NO_ATTRIBUTES_IN_MODEL) {
                                attTblViewer.setInput(null);
                            } else if (notificationType == ModelChangeNotificationType.ATTRIBUTE_VALUE_CHANGED) {
                                if (null != valueChangeSet) {
                                    attTblViewer.update(
                                            valueChangeSet.toArray(), null);
                                }
                            }
                        }
                    }
                });
            }
        };

        automationUIListener = new IAutomationUIListener() {

            @Override
            public void onResourceAutomationStart(final String resourceURI) {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {
                        if (null == resourceURI) {
                            return;
                        }
                        SimulatorResource resource = resourceManager
                                .getCurrentResourceInSelection();
                        if (null == resource) {
                            return;
                        }
                        String uri = resource.getResourceURI();
                        // Checking whether attributes view is currently
                        // displaying the attributes of the
                        // resource whose automation has just started
                        if (null != uri && uri.equals(resourceURI)) {
                            Table tbl;
                            tbl = attTblViewer.getTable();
                            if (!tbl.isDisposed()) {
                                attTblViewer.refresh();

                                // Disabling the table to prevent interactions
                                // during the automation
                                tbl.setEnabled(false);
                                tbl.deselectAll();
                            }
                        }
                    }
                });
            }

            @Override
            public void onAutomationComplete(final String resourceURI,
                    final String attName) {
                // This method notifies the completion of attribute level
                // automation.
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {
                        if (null == resourceURI) {
                            return;
                        }
                        // Check if the given resourceURI is the uri of the
                        // resource whose attributes are currently being
                        // displayed by this view.
                        SimulatorResource resource = resourceManager
                                .getCurrentResourceInSelection();
                        if (null == resource) {
                            return;
                        }
                        String uri = resource.getResourceURI();
                        if (null == uri || !uri.equals(resourceURI)) {
                            return;
                        }
                        Table tbl;
                        tbl = attTblViewer.getTable();
                        if (!tbl.isDisposed()) {
                            if (null != attName) {
                                // Attribute level automation has stopped
                                LocalResourceAttribute att = resourceManager
                                        .getAttributeByResourceURI(resourceURI,
                                                attName);
                                if (null == att) {
                                    return;
                                } else {
                                    attTblViewer.update(att, null);
                                }
                            } else {
                                // Resource level automation has stopped
                                // Enabling the table which was disabled at the
                                // beginning of automation
                                tbl.setEnabled(true);
                                attTblViewer.refresh();
                            }
                        }
                    }
                });
            }
        };
    }

    @Override
    public void createPartControl(Composite parent) {
        Color color = Display.getDefault().getSystemColor(SWT.COLOR_WHITE);

        parent.setLayout(new GridLayout());
        GridData gd = new GridData(SWT.FILL, SWT.FILL, true, true);
        parent.setLayoutData(gd);

        Group attGroup = new Group(parent, SWT.NONE);
        attGroup.setLayout(new GridLayout());
        gd = new GridData(SWT.FILL, SWT.FILL, true, true);
        attGroup.setLayoutData(gd);
        attGroup.setText("Attributes");
        attGroup.setBackground(color);

        attTblViewer = new TableViewer(attGroup, SWT.SINGLE | SWT.H_SCROLL
                | SWT.V_SCROLL | SWT.FULL_SELECTION | SWT.BORDER);

        createAttributeColumns(attTblViewer);

        // make lines and header visible
        Table table = attTblViewer.getTable();
        table.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
        table.setHeaderVisible(true);
        table.setLinesVisible(true);

        attTblViewer.setContentProvider(new AttributeContentProvider());

        addManagerListeners();

        // Check whether there is any resource selected already
        List<LocalResourceAttribute> propertyList = getData();
        if (null != propertyList) {
            updateViewer(propertyList);
        }
    }

    public void createAttributeColumns(TableViewer tableViewer) {

        attributeEditor = new AttributeEditingSupport();

        TableViewerColumn attName = new TableViewerColumn(tableViewer, SWT.NONE);
        attName.getColumn().setWidth(attTblColWidth[0]);
        attName.getColumn().setText(attTblHeaders[0]);
        attName.setLabelProvider(new StyledCellLabelProvider() {
            @Override
            public void update(ViewerCell cell) {
                Object element = cell.getElement();
                if (element instanceof LocalResourceAttribute) {
                    LocalResourceAttribute attribute = (LocalResourceAttribute) element;
                    if (null != attribute) {
                        cell.setText(attribute.getAttributeName());
                    }
                }
            }
        });

        TableViewerColumn attValue = new TableViewerColumn(tableViewer,
                SWT.NONE);
        attValue.getColumn().setWidth(attTblColWidth[1]);
        attValue.getColumn().setText(attTblHeaders[1]);
        attValue.setLabelProvider(new ColumnLabelProvider() {
            @Override
            public String getText(Object element) {
                if (element instanceof LocalResourceAttribute) {
                    LocalResourceAttribute attribute = (LocalResourceAttribute) element;
                    if (null != attribute) {
                        Object val = attribute.getAttributeValue();
                        if (null != val) {
                            return String.valueOf(val);
                        }
                    }
                }
                return "";
            }
        });
        attValue.setEditingSupport(attributeEditor
                .createAttributeValueEditor(attTblViewer));

        TableViewerColumn automation = new TableViewerColumn(tableViewer,
                SWT.NONE);
        automation.getColumn().setWidth(attTblColWidth[2]);
        automation.getColumn().setText(attTblHeaders[2]);
        automation.setLabelProvider(new ColumnLabelProvider() {
            @Override
            public String getText(Object element) {
                LocalResourceAttribute att = (LocalResourceAttribute) element;
                if (att.isAutomationInProgress()) {
                    return Constants.ENABLED;
                }
                return Constants.DISABLED;
            }

            @Override
            public Image getImage(Object element) {
                LocalResourceAttribute att = (LocalResourceAttribute) element;
                if (att.isAutomationInProgress()) {
                    return Activator.getDefault().getImageRegistry()
                            .get(Constants.CHECKED);
                } else {
                    return Activator.getDefault().getImageRegistry()
                            .get(Constants.UNCHECKED);
                }
            }
        });
        automation.setEditingSupport(attributeEditor
                .createAutomationEditor(attTblViewer));
    }

    private void addManagerListeners() {
        resourceManager
                .addResourceSelectionChangedUIListener(resourceSelectionChangedListener);
        resourceManager
                .addResourceModelChangedUIListener(resourceModelChangedUIListener);
        resourceManager.addAutomationUIListener(automationUIListener);
    }

    private List<LocalResourceAttribute> getData() {
        SimulatorResource resourceInSelection = resourceManager
                .getCurrentResourceInSelection();
        if (null != resourceInSelection) {
            List<LocalResourceAttribute> attList = resourceManager
                    .getAttributes(resourceInSelection);
            return attList;
        } else {
            return null;
        }
    }

    private void updateViewer(List<LocalResourceAttribute> attList) {
        Table tbl;
        if (null != attList) {
            tbl = attTblViewer.getTable();
            if (null != tbl && !tbl.isDisposed()) {
                tbl.setLinesVisible(true);
                attTblViewer.setInput(attList.toArray());
            }
        } else {
            // Clear the attributes table viewer
            if (null != attTblViewer) {
                tbl = attTblViewer.getTable();
                if (null != tbl && !tbl.isDisposed()) {
                    // tbl.deselectAll();
                    tbl.removeAll();
                    tbl.setLinesVisible(false);
                }
            }
        }
    }

    class AttributeContentProvider implements IStructuredContentProvider {

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
        // Unregister the selection listener
        if (null != resourceSelectionChangedListener) {
            resourceManager
                    .removeResourceSelectionChangedUIListener(resourceSelectionChangedListener);
        }

        // Unregister the model change listener
        if (null != resourceModelChangedUIListener) {
            resourceManager
                    .removeResourceModelChangedUIListener(resourceModelChangedUIListener);
        }

        // Unregister the automation complete listener
        if (null != automationUIListener) {
            resourceManager.removeAutomationUIListener(automationUIListener);
        }

        super.dispose();
    }

    @Override
    public void setFocus() {

    }
}