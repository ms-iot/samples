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
import java.util.Map;

import oic.simulator.clientcontroller.Activator;
import oic.simulator.clientcontroller.listener.IConfigurationUpload;
import oic.simulator.clientcontroller.listener.IGetUIListener;
import oic.simulator.clientcontroller.listener.IObserveUIListener;
import oic.simulator.clientcontroller.listener.IPostUIListener;
import oic.simulator.clientcontroller.listener.IPutUIListener;
import oic.simulator.clientcontroller.listener.IResourceSelectionChangedUIListener;
import oic.simulator.clientcontroller.listener.IVerificationUIListener;
import oic.simulator.clientcontroller.manager.ResourceManager;
import oic.simulator.clientcontroller.remoteresource.PutPostAttributeModel;
import oic.simulator.clientcontroller.remoteresource.RemoteResource;
import oic.simulator.clientcontroller.remoteresource.RemoteResourceAttribute;
import oic.simulator.clientcontroller.utils.Constants;
import oic.simulator.clientcontroller.view.dialogs.PostRequestDialog;
import oic.simulator.clientcontroller.view.dialogs.PutRequestDialog;
import oic.simulator.clientcontroller.view.dialogs.VerificationDialog;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.IStructuredContentProvider;
import org.eclipse.jface.viewers.StyledCellLabelProvider;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.jface.viewers.TableViewerColumn;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.viewers.ViewerCell;
import org.eclipse.jface.window.Window;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Table;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.part.ViewPart;

/**
 * This class manages and shows the attribute view in the perspective.
 */
public class AttributeView extends ViewPart {

    public static final String                  VIEW_ID        = "oic.simulator.clientcontroller.view.attribute";

    private TableViewer                         attTblViewer;

    private Button                              getButton;
    private Button                              putButton;
    private Button                              postButton;
    private Button                              automateButton;
    private Button                              observeResButton;

    private final String[]                      attTblHeaders  = {
            "Attribute Name", "Attribute Value"               };
    private final Integer[]                     attTblColWidth = { 200, 200 };

    private ResourceManager                     resourceManager;

    private IResourceSelectionChangedUIListener resourceSelectionChangedListener;
    private IGetUIListener                      getUIListener;
    private IPutUIListener                      putUIListener;
    private IPostUIListener                     postUIListener;
    private IObserveUIListener                  observeUIListener;
    private IVerificationUIListener             verificationUIListener;
    private IConfigurationUpload                configUploadUIListener;

    private RemoteResource                      resourceInSelection;

    public AttributeView() {
        resourceManager = Activator.getDefault().getResourceManager();

        resourceSelectionChangedListener = new IResourceSelectionChangedUIListener() {

            @Override
            public void onResourceSelectionChange(final RemoteResource resource) {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {
                        System.out
                                .println("AttributeView: onResourceSelectionChange");
                        resourceInSelection = resource;

                        // Set visibility of manual and automation controls
                        setVisibility((resource == null) ? false : true);

                        // Update the attribute table
                        if (null != attTblViewer
                                && !attTblViewer.getControl().isDisposed()) {
                            System.out.println("viewer is alive");
                            updateViewer(getData(resource));
                        }

                        // Update the observe status
                        updateObserve(resource);

                        // Update the pay-load details if any
                    }
                });
            }
        };

        getUIListener = new IGetUIListener() {

            @Override
            public void onGetCompleted(final RemoteResource resource) {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {

                        System.out.println("AttributeView: onGetCompleted");
                        if (null == resource) {
                            return;
                        }
                        // Update the attribute table
                        if (resourceInSelection != resource) {
                            System.out
                                    .println("AttributeView: get response arrived for a different resource");
                            return;
                        }
                        updateViewer(getData(resource));

                        // Update the observe status
                        updateObserve(resource);

                        // Update the pay-load details if any
                    }
                });
            }

            @Override
            public void onGetFailed(RemoteResource resource) {
            }
        };

        putUIListener = new IPutUIListener() {

            @Override
            public void onPutCompleted(final RemoteResource resource) {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {

                        System.out.println("AttributeView: onPutCompleted");
                        if (null == resource) {
                            return;
                        }
                        // Update the attribute table
                        if (resourceInSelection != resource) {
                            System.out
                                    .println("AttributeView: put response arrived for a different resource");
                            return;
                        }
                        updateViewer(getData(resource));

                        // Update the observe status
                        updateObserve(resource);

                        // Update the pay-load details if any
                    }
                });
            }

            @Override
            public void onPutFailed(RemoteResource resource) {
            }
        };

        postUIListener = new IPostUIListener() {

            @Override
            public void onPostCompleted(final RemoteResource resource) {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {

                        System.out.println("AttributeView: onPostCompleted");
                        if (null == resource) {
                            return;
                        }
                        // Update the attribute table
                        if (resourceInSelection != resource) {
                            System.out
                                    .println("AttributeView: post response arrived for a different resource");
                            return;
                        }
                        updateViewer(getData(resource));

                        // Update the observe status
                        updateObserve(resource);

                        // Update the pay-load details if any
                    }
                });
            }

            @Override
            public void onPostFailed(RemoteResource resource) {
            }
        };

        observeUIListener = new IObserveUIListener() {

            @Override
            public void onObserveCompleted(final RemoteResource resource) {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {

                        System.out.println("AttributeView: onObserveCompleted");
                        if (null == resource) {
                            return;
                        }
                        // Update the attribute table
                        if (resourceInSelection != resource) {
                            System.out
                                    .println("AttributeView: observe response arrived for a different resource");
                            return;
                        }
                        updateViewer(getData(resource));

                        // Update the observe status
                        updateObserve(resource);

                        // Update the pay-load details if any
                    }
                });
            }

            @Override
            public void onObserveFailed(RemoteResource resource) {
            }
        };

        verificationUIListener = new IVerificationUIListener() {

            @Override
            public void onVerificationStarted(final RemoteResource resource,
                    final int autoType) {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {
                        changeReqBtnVisibility(autoType, false);
                    }
                });
            }

            @Override
            public void onVerificationCompleted(final RemoteResource resource,
                    final int autoType) {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {
                        changeReqBtnVisibility(autoType, true);
                    }
                });
            }

            @Override
            public void onVerificationAborted(final RemoteResource resource,
                    final int autoType) {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {
                        changeReqBtnVisibility(autoType, true);
                    }
                });
            }
        };

        configUploadUIListener = new IConfigurationUpload() {

            @Override
            public void onConfigurationUploaded(final RemoteResource resource) {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {

                        System.out
                                .println("AttributeView: onConfigurationUploaded");
                        if (null == resource) {
                            return;
                        }
                        if (resourceInSelection != resource) {
                            System.out
                                    .println("AttributeView: config upload response arrived for a different resource");
                            return;
                        }
                        if (!automateButton.isDisposed()) {
                            automateButton.setEnabled(true);
                        }
                    }
                });
            }
        };
    }

    private void changeReqBtnVisibility(int reqType, boolean visibility) {
        if (reqType == Constants.GET_AUTOMATION_INDEX) {
            if (!getButton.isDisposed()) {
                getButton.setEnabled(visibility);
            }
        } else if (reqType == Constants.PUT_AUTOMATION_INDEX) {
            if (!putButton.isDisposed()) {
                putButton.setEnabled(visibility);
            }
        } else {// if(reqType == Constants.POST_AUTOMATION_INDEX) {
            if (!postButton.isDisposed()) {
                postButton.setEnabled(visibility);
            }
        }
    }

    private Map<String, RemoteResourceAttribute> getData(RemoteResource resource) {
        if (null == resource) {
            return null;
        }
        Map<String, RemoteResourceAttribute> attMap = resource
                .getResourceAttributesMap();
        System.out.println("AttributeView: \n" + attMap);
        return attMap;
    }

    private void updateViewer(Map<String, RemoteResourceAttribute> attMap) {
        if (null != attTblViewer) {
            Table tbl = attTblViewer.getTable();
            if (null != attMap) {
                attTblViewer.setInput(attMap.entrySet().toArray());
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

    private void updateObserve(RemoteResource resource) {
        if (null == resource) {
            return;
        }
        boolean observed = resource.isObserved();
        if (!observeResButton.isDisposed()) {
            observeResButton.setSelection(observed);
        }
    }

    @Override
    public void createPartControl(Composite parent) {
        Color color = Display.getDefault().getSystemColor(SWT.COLOR_WHITE);

        parent.setLayout(new GridLayout(2, false));
        GridData gd = new GridData(SWT.FILL, SWT.FILL, true, true);
        parent.setLayoutData(gd);

        Group attGroup = new Group(parent, SWT.NONE);
        attGroup.setLayout(new GridLayout());
        gd = new GridData();
        gd.grabExcessHorizontalSpace = true;
        gd.horizontalAlignment = SWT.FILL;
        gd.grabExcessVerticalSpace = true;
        gd.verticalAlignment = SWT.FILL;
        gd.horizontalSpan = 2;
        // gd.heightHint = 175;
        attGroup.setLayoutData(gd);
        attGroup.setText("Attributes");
        attGroup.setBackground(color);

        setupAttributeTable(attGroup);

        setupMessageArea(parent);

        setupResourceLevelOpsArea(parent);

        setUIListeners();

        addManagerListeners();

        setVisibility(false);

        // Updating the data in the UI as per the resource in selection.
        if (null != attTblViewer && !attTblViewer.getControl().isDisposed()) {
            updateViewer(getData(resourceManager
                    .getCurrentResourceInSelection()));
        }
    }

    private void setupMessageArea(Composite parent) {
        GridData gd;
        Color color = Display.getDefault().getSystemColor(SWT.COLOR_WHITE);
        Group msgTypesGrp = new Group(parent, SWT.NONE);
        gd = new GridData();
        gd.grabExcessHorizontalSpace = true;
        gd.horizontalAlignment = SWT.FILL;
        msgTypesGrp.setLayoutData(gd);
        GridLayout grid = new GridLayout(3, false);
        msgTypesGrp.setLayout(grid);
        msgTypesGrp.setText("Request Types");
        msgTypesGrp.setBackground(color);

        getButton = new Button(msgTypesGrp, SWT.PUSH);
        getButton.setText("GET");
        gd = new GridData();
        gd.grabExcessHorizontalSpace = true;
        gd.horizontalAlignment = SWT.FILL;
        gd.widthHint = 50;
        getButton.setLayoutData(gd);

        putButton = new Button(msgTypesGrp, SWT.PUSH);
        putButton.setText("PUT");
        gd = new GridData();
        gd.grabExcessHorizontalSpace = true;
        gd.horizontalAlignment = SWT.FILL;
        gd.widthHint = 50;
        putButton.setLayoutData(gd);

        postButton = new Button(msgTypesGrp, SWT.PUSH);
        postButton.setText("POST");
        gd = new GridData();
        gd.grabExcessHorizontalSpace = true;
        gd.horizontalAlignment = SWT.FILL;
        gd.widthHint = 50;
        postButton.setLayoutData(gd);
    }

    private void setupResourceLevelOpsArea(Composite parent) {
        GridData gd;
        Color color = Display.getDefault().getSystemColor(SWT.COLOR_WHITE);
        Group resOpsGrp = new Group(parent, SWT.NONE);
        gd = new GridData();
        gd.grabExcessHorizontalSpace = true;
        gd.horizontalAlignment = SWT.FILL;
        resOpsGrp.setLayoutData(gd);
        GridLayout grid = new GridLayout(2, false);
        resOpsGrp.setLayout(grid);
        resOpsGrp.setText("Resource-Level Operations");
        resOpsGrp.setBackground(color);

        automateButton = new Button(resOpsGrp, SWT.PUSH);
        automateButton.setText("Automation");
        gd = new GridData();
        gd.grabExcessHorizontalSpace = true;
        gd.horizontalAlignment = SWT.FILL;
        automateButton.setLayoutData(gd);

        observeResButton = new Button(resOpsGrp, SWT.CHECK);
        observeResButton.setText("Observe");
        gd = new GridData();
        gd.grabExcessHorizontalSpace = true;
        gd.horizontalAlignment = SWT.FILL;
        observeResButton.setLayoutData(gd);
    }

    private void setupAttributeTable(Group attGroup) {
        attTblViewer = new TableViewer(attGroup, SWT.SINGLE | SWT.H_SCROLL
                | SWT.V_SCROLL | SWT.FULL_SELECTION | SWT.BORDER);

        createAttributeColumns(attTblViewer);

        // make lines and header visible
        Table table = attTblViewer.getTable();
        table.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
        table.setHeaderVisible(true);
        table.setLinesVisible(true);

        attTblViewer.setContentProvider(new AttributeContentProvider());
    }

    public void createAttributeColumns(TableViewer tableViewer) {

        TableViewerColumn attName = new TableViewerColumn(tableViewer, SWT.NONE);
        attName.getColumn().setWidth(attTblColWidth[0]);
        attName.getColumn().setText(attTblHeaders[0]);
        attName.setLabelProvider(new StyledCellLabelProvider() {
            @Override
            public void update(ViewerCell cell) {
                Object element = cell.getElement();
                if (element instanceof Map.Entry) {
                    @SuppressWarnings("unchecked")
                    Map.Entry<String, RemoteResourceAttribute> entry = (Map.Entry<String, RemoteResourceAttribute>) element;
                    cell.setText(entry.getKey());
                }
            }
        });

        TableViewerColumn attValue = new TableViewerColumn(tableViewer,
                SWT.NONE);
        attValue.getColumn().setWidth(attTblColWidth[1]);
        attValue.getColumn().setText(attTblHeaders[1]);
        attValue.setLabelProvider(new StyledCellLabelProvider() {
            @Override
            public void update(ViewerCell cell) {
                Object element = cell.getElement();
                if (element instanceof Map.Entry) {
                    @SuppressWarnings("unchecked")
                    Map.Entry<String, RemoteResourceAttribute> entry = (Map.Entry<String, RemoteResourceAttribute>) element;
                    Object value = entry.getValue().getAttributeValue();
                    if (null == value) {
                        cell.setText("");
                    } else {
                        cell.setText(String.valueOf(value));
                    }
                }
            }
        });
    }

    private void setUIListeners() {

        getButton.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                if (resourceInSelection.isGetAutomtnInProgress()) {
                    MessageDialog
                            .openInformation(Display.getDefault()
                                    .getActiveShell(), "GET Request",
                                    "GET Automation is in progress. Please wait or stop the automation.");
                } else {
                    resourceManager.sendGetRequest(resourceInSelection);
                }
            }
        });

        putButton.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                PlatformUI.getWorkbench().getDisplay().syncExec(new Thread() {
                    @Override
                    public void run() {
                        if (resourceInSelection.isPutAutomtnInProgress()) {
                            MessageDialog
                                    .openInformation(Display.getDefault()
                                            .getActiveShell(), "PUT Request",
                                            "PUT Automation is in progress. Please wait or stop the automation.");
                            return;
                        }
                        List<PutPostAttributeModel> putPostModelList;
                        putPostModelList = resourceInSelection
                                .getPutPostModel();
                        if (null == putPostModelList) {
                            MessageDialog
                                    .openInformation(Display.getDefault()
                                            .getActiveShell(), "PUT Request",
                                            "No attributes exist in the resource model.");
                            return;
                        }
                        PutRequestDialog putDialog = new PutRequestDialog(
                                Display.getDefault().getActiveShell(),
                                putPostModelList);
                        if (putDialog.open() == Window.OK) {
                            // Call the native PUT method
                            resourceManager.sendPutRequest(resourceInSelection,
                                    putPostModelList);
                        }
                    }
                });
            }
        });

        postButton.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                PlatformUI.getWorkbench().getDisplay().syncExec(new Thread() {
                    @Override
                    public void run() {
                        if (resourceInSelection.isPostAutomtnInProgress()) {
                            MessageDialog
                                    .openInformation(Display.getDefault()
                                            .getActiveShell(), "POST Request",
                                            "POST Automation is in progress. Please wait or stop the automation.");
                            return;
                        }
                        List<PutPostAttributeModel> putPostModelList;
                        putPostModelList = resourceInSelection
                                .getPutPostModel();
                        if (null == putPostModelList) {
                            MessageDialog
                                    .openInformation(Display.getDefault()
                                            .getActiveShell(), "PUT Request",
                                            "No attributes exist in the resource model.");
                            return;
                        }

                        PostRequestDialog postDialog = new PostRequestDialog(
                                Display.getDefault().getActiveShell(),
                                putPostModelList);
                        if (postDialog.open() == Window.OK) {
                            // Call the native POST method
                            resourceManager.sendPostRequest(
                                    resourceInSelection, putPostModelList);
                        }
                    }
                });
            }
        });

        observeResButton.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                boolean checked = observeResButton.getSelection();
                if (checked) {
                    resourceManager.sendObserveRequest(resourceInSelection);
                } else {
                    resourceManager
                            .sendCancelObserveRequest(resourceInSelection);
                }
            }
        });

        automateButton.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                PlatformUI.getWorkbench().getDisplay().syncExec(new Thread() {
                    @Override
                    public void run() {
                        RemoteResource resource = resourceManager
                                .getCurrentResourceInSelection();
                        if (null == resource) {
                            return;
                        }
                        Map<String, Boolean> autoStatus = resourceManager
                                .getAutomationStatus(resource);
                        if (null == autoStatus) {
                            return;
                        }
                        VerificationDialog ad = new VerificationDialog(Display
                                .getDefault().getActiveShell(), autoStatus);
                        ad.open();
                    }
                });
            }
        });
    }

    private void addManagerListeners() {
        resourceManager
                .addResourceSelectionChangedUIListener(resourceSelectionChangedListener);
        resourceManager.addGetUIListener(getUIListener);
        resourceManager.addPutUIListener(putUIListener);
        resourceManager.addPostUIListener(postUIListener);
        resourceManager.addObserveUIListener(observeUIListener);
        resourceManager.addVerificationUIListener(verificationUIListener);
        resourceManager.addConfigUploadUIListener(configUploadUIListener);
    }

    private void setVisibility(boolean visibility) {
        if (!getButton.isDisposed())
            getButton.setEnabled(visibility);
        if (!putButton.isDisposed())
            putButton.setEnabled(visibility);
        if (!postButton.isDisposed())
            postButton.setEnabled(visibility);
        if (!automateButton.isDisposed()) {
            if (visibility && null != resourceInSelection
                    && resourceInSelection.isConfigUploaded()) {
                automateButton.setEnabled(true);
            } else {
                automateButton.setEnabled(false);
            }
        }
        if (!observeResButton.isDisposed())
            observeResButton.setEnabled(visibility);
    }

    class AttributeContentProvider implements IStructuredContentProvider {

        @Override
        public void dispose() {
        }

        @Override
        public void inputChanged(Viewer viewer, Object arg1, Object arg2) {
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

        // Unregister the GET listener
        if (null != getUIListener) {
            resourceManager.removeGetUIListener(getUIListener);
        }

        super.dispose();
    }

    @Override
    public void setFocus() {
    }

}
