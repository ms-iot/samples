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

import java.util.ArrayList;
import java.util.List;

import oic.simulator.serviceprovider.Activator;
import oic.simulator.serviceprovider.listener.IResourceListChangedUIListener;
import oic.simulator.serviceprovider.manager.ResourceManager;
import oic.simulator.serviceprovider.resource.DeleteCategory;
import oic.simulator.serviceprovider.utils.Constants;
import oic.simulator.serviceprovider.utils.Utility;
import oic.simulator.serviceprovider.view.dialogs.CreateResourceWizard;
import oic.simulator.serviceprovider.view.dialogs.DeleteResourceWizard;
import oic.simulator.serviceprovider.view.dialogs.ResourceWizardDialog;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ITreeContentProvider;
import org.eclipse.jface.viewers.LabelProvider;
import org.eclipse.jface.viewers.TreeViewer;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.wizard.WizardDialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.MenuAdapter;
import org.eclipse.swt.events.MenuEvent;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.MenuItem;
import org.eclipse.swt.widgets.Tree;
import org.eclipse.swt.widgets.TreeItem;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.dialogs.FilteredTree;
import org.eclipse.ui.dialogs.PatternFilter;
import org.eclipse.ui.part.ViewPart;

/**
 * This class manages and shows the resource manager view in the perspective.
 */
public class ResourceManagerView extends ViewPart {

    public static final String             VIEW_ID = "oic.simulator.serviceprovider.view.resourcemanager";

    private Button                         createButton;
    private Button                         deleteButton;

    private TreeViewer                     treeViewer;

    private IResourceListChangedUIListener resourceListChangedListener;

    private ResourceManager                resourceManager;

    public ResourceManagerView() {

        resourceManager = Activator.getDefault().getResourceManager();

        resourceListChangedListener = new IResourceListChangedUIListener() {

            @Override
            public void onResourceCreation() {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {
                        if (null != treeViewer) {
                            treeViewer.refresh();
                        }

                        // Trigger the visibility of delete button
                        changeDeleteVisibility();
                    }
                });
            }

            @Override
            public void onResourceDeletion() {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {
                        if (null != treeViewer) {
                            treeViewer.refresh();
                        }

                        // Trigger the visibility of delete button
                        changeDeleteVisibility();
                    }
                });
            }
        };
    }

    public void changeDeleteVisibility() {
        if (null == treeViewer) {
            return;
        }
        boolean visibility;
        Tree tree = treeViewer.getTree();
        if (null != tree && !tree.isDisposed() && tree.getItemCount() > 0) {
            visibility = true;
        } else {
            visibility = false;
        }
        if (null != deleteButton && !deleteButton.isDisposed()) {
            deleteButton.setEnabled(visibility);
        }
    }

    @Override
    public void createPartControl(Composite parent) {
        Composite compContent = new Composite(parent, SWT.NONE);
        GridLayout baseLayout = new GridLayout(1, false);
        compContent.setLayout(baseLayout);

        GridData gd = new GridData(SWT.FILL, SWT.FILL, true, true);
        compContent.setLayoutData(gd);

        Composite buttonComp = new Composite(compContent, SWT.NONE);
        buttonComp.setLayout(new GridLayout(2, false));

        gd = new GridData();
        gd.horizontalAlignment = SWT.FILL;
        gd.grabExcessHorizontalSpace = true;

        buttonComp.setLayoutData(gd);

        createButton = new Button(buttonComp, SWT.PUSH);
        createButton.setText("Create");
        createButton.setToolTipText("Create Simulator Resource(s)");

        gd = new GridData();
        gd.widthHint = 90;
        createButton.setLayoutData(gd);

        deleteButton = new Button(buttonComp, SWT.PUSH);
        deleteButton.setText("Delete");
        deleteButton.setToolTipText("Delete Simulator Resource(s)");

        gd = new GridData();
        gd.widthHint = 90;
        deleteButton.setLayoutData(gd);

        Group resourceGroup = new Group(compContent, SWT.NONE);
        resourceGroup.setText("Created Resources");

        Color color = Display.getDefault().getSystemColor(SWT.COLOR_WHITE);
        resourceGroup.setBackground(color);

        resourceGroup.setLayout(new GridLayout(1, false));
        gd = new GridData(SWT.FILL, SWT.FILL, true, true);
        resourceGroup.setLayoutData(gd);

        PatternFilter filter = new PatternFilter();
        FilteredTree filteredTree = new FilteredTree(resourceGroup,
                SWT.H_SCROLL | SWT.V_SCROLL | SWT.SINGLE, filter, true);
        treeViewer = filteredTree.getViewer();
        treeViewer.getTree().setLayoutData(
                new GridData(SWT.FILL, SWT.FILL, true, true));
        treeViewer.setContentProvider(new TreeContentProvider());
        treeViewer.setLabelProvider(new TreeLabelProvider());
        treeViewer.setInput(new Object());

        addUIListeners();

        addManagerListeners();

        // If there is at least one resource exist, then enable the delete
        // resource button
        changeDeleteVisibility();
    }

    private void addUIListeners() {

        createButton.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                PlatformUI.getWorkbench().getDisplay().syncExec(new Runnable() {

                    @Override
                    public void run() {
                        CreateResourceWizard createWizard = new CreateResourceWizard();
                        ResourceWizardDialog wizardDialog = new ResourceWizardDialog(
                                PlatformUI.getWorkbench().getDisplay()
                                        .getActiveShell(), createWizard);
                        int open = wizardDialog.open();
                        if (open == WizardDialog.OK) {
                            String configFilePath;
                            int count;
                            configFilePath = createWizard.getConfigFilePath();
                            System.out.println("Resultant config file path is "
                                    + configFilePath);
                            count = createWizard.getResourceCount();
                            if (count > 0) {
                                if (count == 1) {
                                    // Single resource creation
                                    resourceManager
                                            .createResource(configFilePath);
                                } else {
                                    // Multi-resource creation
                                    resourceManager.createResource(
                                            configFilePath, count);
                                }
                            }
                        }
                    }
                });
            }
        });

        deleteButton.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                PlatformUI.getWorkbench().getDisplay().syncExec(new Runnable() {

                    @Override
                    public void run() {
                        DeleteResourceWizard deleteWizard = new DeleteResourceWizard();
                        ResourceWizardDialog wizardDialog = new ResourceWizardDialog(
                                PlatformUI.getWorkbench().getDisplay()
                                        .getActiveShell(), deleteWizard);
                        int open = wizardDialog.open();
                        if (open == WizardDialog.OK) {
                            DeleteCategory deleteCategory = deleteWizard
                                    .getDeleteCategory();
                            if (deleteCategory == DeleteCategory.BY_URI) {
                                String uri = deleteWizard.getDeleteCandidate();
                                if (null != uri) {
                                    boolean dispName = Activator.getDefault()
                                            .getResourceManager()
                                            .isDisplayName(uri);
                                    if (dispName) {
                                        uri = Activator
                                                .getDefault()
                                                .getResourceManager()
                                                .getCompleteUriFromDisplayName(
                                                        uri);
                                    }
                                    resourceManager.deleteResourceByURI(uri);
                                }
                            } else if (deleteCategory == DeleteCategory.BY_TYPE) {
                                resourceManager
                                        .deleteResourceByType(deleteWizard
                                                .getDeleteCandidate());
                            } else if (deleteCategory == DeleteCategory.ALL) {
                                resourceManager.deleteAllResources();
                            }
                        }
                    }
                });
            }
        });

        if (null != treeViewer) {
            final Tree resourceTreeHead = treeViewer.getTree();
            if (null != resourceTreeHead) {
                // Below code adds a listener to the tree for selection changes
                // and notifies the resource manager
                resourceTreeHead.addSelectionListener(new SelectionAdapter() {
                    @Override
                    public void widgetSelected(SelectionEvent e) {
                        TreeItem selectedItem = (TreeItem) e.item;
                        if (null != selectedItem) {
                            String selectedItemText = selectedItem.getText();
                            selectedItemText = resourceManager
                                    .getCompleteUriFromDisplayName(selectedItemText);
                            // Propagate this selection change event to manager
                            resourceManager
                                    .resourceSelectionChanged(selectedItemText);
                        }
                    }
                });
                // Below code creates menu entries and shows them on right
                // clicking a resource
                final Menu menu = new Menu(resourceTreeHead);
                resourceTreeHead.setMenu(menu);
                menu.addMenuListener(new MenuAdapter() {
                    @Override
                    public void menuShown(MenuEvent e) {
                        // Clear existing menu items
                        MenuItem[] items = menu.getItems();
                        for (int index = 0; index < items.length; index++) {
                            items[index].dispose();
                        }
                        final String selectedItem = resourceTreeHead
                                .getSelection()[0].getText();
                        MenuItem startItem = new MenuItem(menu, SWT.NONE);
                        startItem.setText(Constants.START_RESOURCE_AUTOMATION);
                        startItem.addSelectionListener(new SelectionAdapter() {
                            @Override
                            public void widgetSelected(SelectionEvent e) {
                                // Block starting resource level
                                // automation if any attribute level
                                // automation is in progress for the
                                // selected resource
                                boolean started = resourceManager
                                        .isAttributeAutomationStarted(resourceManager
                                                .getCompleteUriFromDisplayName(selectedItem));
                                if (started) {
                                    MessageDialog
                                            .openInformation(
                                                    Display.getDefault()
                                                            .getActiveShell(),
                                                    "Attribute automation is in progress",
                                                    "Attribute level automation for this resource is already in progress!!!\nPlease stop all "
                                                            + "running attribute level automations to start resource level automation.");
                                } else {
                                    boolean status = resourceManager
                                            .startResourceAutomationUIRequest(resourceManager
                                                    .getCompleteUriFromDisplayName(selectedItem));
                                    String statusMsg = status ? "Automation started successfully!!!"
                                            : "Automation request failed!!!";
                                    MessageDialog.openInformation(Display
                                            .getDefault().getActiveShell(),
                                            "Automation Status", statusMsg);
                                }
                            }
                        });

                        MenuItem stopItem = new MenuItem(menu, SWT.NONE);
                        stopItem.setText(Constants.STOP_RESOURCE_AUTOMATION);
                        stopItem.addSelectionListener(new SelectionAdapter() {
                            @Override
                            public void widgetSelected(SelectionEvent e) {
                                boolean status = resourceManager
                                        .stopResourceAutomationUIRequest(resourceManager
                                                .getCompleteUriFromDisplayName(selectedItem));
                                String statusMsg = status ? "Automation stop requested!!!"
                                        : "Automation stop failed.";
                                MessageDialog.openInformation(Display
                                        .getDefault().getActiveShell(),
                                        "Automation Status", statusMsg);
                            }
                        });

                        // Set the initial visibility of menu items
                        boolean status = resourceManager.isResourceAutomationStarted(resourceManager
                                .getCompleteUriFromDisplayName(selectedItem));
                        startItem.setEnabled(!status);
                        stopItem.setEnabled(status);
                    }
                });
            }
        }
    }

    public void addManagerListeners() {
        resourceManager
                .addResourceListChangedUIListener(resourceListChangedListener);
    }

    @Override
    public void setFocus() {
    }

    @Override
    public void dispose() {
        // Unregister the listener
        if (null != resourceListChangedListener) {
            resourceManager
                    .removeResourceListChangedUIListener(resourceListChangedListener);
            resourceManager.resourceSelectionChanged(null);
        }
        super.dispose();
    }
}

class TreeContentProvider implements ITreeContentProvider {

    @Override
    public void dispose() {
    }

    @Override
    public void inputChanged(Viewer arg0, Object arg1, Object arg2) {
    }

    @Override
    public Object[] getChildren(Object parent) {
        return null;
    }

    @Override
    public Object[] getElements(Object parent) {
        List<String> uriList;
        uriList = Activator.getDefault().getResourceManager().getURIList();
        if (null == uriList) {
            uriList = new ArrayList<String>();
        }
        return uriList.toArray();
    }

    @Override
    public Object getParent(Object child) {
        return null;
    }

    @Override
    public boolean hasChildren(Object parent) {
        return false;
    }
}

class TreeLabelProvider extends LabelProvider {
    @Override
    public String getText(Object element) {
        String value = (String) element;
        value = Utility.uriToDisplayName(value);
        return value;
    }

    @Override
    public Image getImage(Object element) {
        ResourceManager resourceManager = Activator.getDefault()
                .getResourceManager();
        return resourceManager.getImage((String) element);
    }
}