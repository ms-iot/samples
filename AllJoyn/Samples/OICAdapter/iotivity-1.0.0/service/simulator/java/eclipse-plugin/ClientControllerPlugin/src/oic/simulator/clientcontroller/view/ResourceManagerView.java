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
import java.util.Set;

import oic.simulator.clientcontroller.Activator;
import oic.simulator.clientcontroller.listener.IFindResourceUIListener;
import oic.simulator.clientcontroller.manager.ResourceManager;
import oic.simulator.clientcontroller.remoteresource.RemoteResource;
import oic.simulator.clientcontroller.utils.Constants;
import oic.simulator.clientcontroller.view.dialogs.FindResourceWizard;
import oic.simulator.clientcontroller.view.dialogs.LoadRAMLDialog;
import oic.simulator.clientcontroller.view.dialogs.ResourceWizardDialog;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.ITreeContentProvider;
import org.eclipse.jface.viewers.LabelProvider;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.TreeViewer;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.window.Window;
import org.eclipse.jface.wizard.WizardDialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.CTabFolder;
import org.eclipse.swt.custom.CTabItem;
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
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.dialogs.FilteredTree;
import org.eclipse.ui.dialogs.PatternFilter;
import org.eclipse.ui.part.ViewPart;

/**
 * This class manages and shows the resource manager view in the perspective.
 */
public class ResourceManagerView extends ViewPart {

    public static final String      VIEW_ID = "oic.simulator.clientcontroller.view.resourcemanager";

    private Button                  findResButton;
    private Button                  refreshButton;

    private TreeViewer              treeViewer;
    private TreeViewer              favTreeViewer;

    private CTabFolder              folder;
    private CTabItem                foundResTab;
    private CTabItem                favResTab;

    private ResourceManager         resourceManager;

    private IFindResourceUIListener findListener;

    private Boolean                 foundResource;

    private MessageDialog           findDialog;

    private MessageDialog           refreshDialog;

    private Thread                  sleepThreadHandle;

    public ResourceManagerView() {
        resourceManager = Activator.getDefault().getResourceManager();

        findListener = new IFindResourceUIListener() {

            @Override
            public void onNewResourceFound(final RemoteResource resource) {
                System.out.println("View: onNewResourceFound");
                if (null == resource) {
                    return;
                }
                // Changing the status of the find operation.
                setFoundResource(true);

                // Interrupt the sleep thread.
                if (null != sleepThreadHandle && sleepThreadHandle.isAlive()) {
                    sleepThreadHandle.interrupt();
                }

                // Update the tree viewer
                Display.getDefault().asyncExec(new Runnable() {
                    @Override
                    public void run() {
                        if (!treeViewer.getControl().isDisposed()) {
                            treeViewer.refresh();
                        }

                        if (!favTreeViewer.getControl().isDisposed()) {
                            favTreeViewer.refresh();
                        }

                        // Close the find dialog
                        if (null != findDialog) {
                            boolean status = findDialog.close();
                            System.out
                                    .println("dialog close status: " + status);
                        }

                        // Close the refresh dialog
                        if (null != refreshDialog) {
                            boolean status = refreshDialog.close();
                            System.out
                                    .println("dialog close status: " + status);
                        }
                    }
                });
            }
        };
    }

    @Override
    public void createPartControl(Composite parent) {
        Composite compContent = new Composite(parent, SWT.NONE);
        compContent.setLayout(new GridLayout());
        GridData gd = new GridData(SWT.FILL, SWT.FILL, true, true);
        compContent.setLayoutData(gd);

        Composite buttonComp = new Composite(compContent, SWT.NONE);
        buttonComp.setLayout(new GridLayout(2, false));

        gd = new GridData();
        gd.horizontalAlignment = SWT.FILL;
        gd.grabExcessHorizontalSpace = true;

        buttonComp.setLayoutData(gd);

        findResButton = new Button(buttonComp, SWT.PUSH);
        findResButton.setText("Find Resources");
        findResButton.setToolTipText("Find OIC resources");

        gd = new GridData();
        gd.widthHint = 130;
        findResButton.setLayoutData(gd);

        refreshButton = new Button(buttonComp, SWT.PUSH);
        refreshButton.setText("Refresh");
        refreshButton.setToolTipText("Restart the search once again");

        gd = new GridData();
        gd.widthHint = 90;
        refreshButton.setLayoutData(gd);

        // Create a Tab Folder.
        folder = new CTabFolder(compContent, SWT.BORDER);
        gd = new GridData(SWT.FILL, SWT.FILL, true, true);
        folder.setLayoutData(gd);
        folder.setSimple(false);
        folder.setUnselectedCloseVisible(false);
        folder.setUnselectedImageVisible(false);
        folder.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                /*
                 * CTabItem selectedTab = folder.getSelection(); if(selectedTab
                 * == foundResTab) { System.out.println("Found resources tab");
                 * } else { System.out.println("Favorite resources tab"); }
                 */
                // Tab is switched.
                treeViewer.setSelection(null);
                favTreeViewer.setSelection(null);
                resourceManager.resourceSelectionChanged(null);
            }
        });

        createFoundResourcesArea();

        createFavoriteResourcesArea();

        folder.setSelection(foundResTab);

        findDialog = new MessageDialog(Display.getDefault().getActiveShell(),
                "Finding Servers", null,
                "Finding the requested servers\nPlease wait...",
                MessageDialog.INFORMATION, new String[] { "Cancel" }, 0);
        // findDialog.setBlockOnOpen(false);

        refreshDialog = new MessageDialog(
                Display.getDefault().getActiveShell(),
                "Finding Servers",
                null,
                "Refreshing the search and finding the requested servers once again\nPlease wait...",
                MessageDialog.INFORMATION, new String[] { "Cancel" }, 0);
        // refreshDialog.setBlockOnOpen(false);

        addUIListeners();

        addManagerListeners();

        // Setting the initial visibility of refresh based on the last known
        // search operation.
        Set<String> prevSearchTypes = resourceManager.getLastKnownSearchTypes();
        if (null == prevSearchTypes || prevSearchTypes.size() < 1) {
            refreshButton.setEnabled(false);
        } else {
            refreshButton.setEnabled(true);
        }
    }

    private void createFoundResourcesArea() {
        foundResTab = new CTabItem(folder, SWT.NULL);
        foundResTab.setText("Found Resources");

        // Create a group to show all the discovered resources.
        // Adding the group to the folder.
        Group resourceGroup = new Group(folder, SWT.NONE);
        // resourceGroup.setText("Discovered Resources");

        Color color = Display.getDefault().getSystemColor(SWT.COLOR_WHITE);
        resourceGroup.setBackground(color);

        resourceGroup.setLayout(new GridLayout(1, false));
        GridData gd = new GridData(SWT.FILL, SWT.FILL, true, true);
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

        addMenuToFoundResources();

        foundResTab.setControl(resourceGroup);
    }

    private void addMenuToFoundResources() {

        if (null != treeViewer) {
            final Tree resourceTreeHead = treeViewer.getTree();
            if (null != resourceTreeHead) {
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
                        MenuItem uploadRAMLItem = new MenuItem(menu, SWT.NONE);
                        uploadRAMLItem.setText("Upload RAML Configuration");
                        uploadRAMLItem
                                .addSelectionListener(new SelectionAdapter() {
                                    @Override
                                    public void widgetSelected(SelectionEvent e) {
                                        // Open the RAML configuration dialog if
                                        // RAML file is not yet uploaded for the
                                        // currently selected resource
                                        RemoteResource resource = resourceManager
                                                .getCurrentResourceInSelection();
                                        if (null == resource) {
                                            return;
                                        }
                                        if (!resource.isConfigUploaded()) {
                                            // Open the dialog in a separate
                                            // UI thread.
                                            PlatformUI.getWorkbench()
                                                    .getDisplay()
                                                    .syncExec(new Thread() {
                                                        @Override
                                                        public void run() {
                                                            LoadRAMLDialog ramlDialog = new LoadRAMLDialog(
                                                                    Display.getDefault()
                                                                            .getActiveShell());
                                                            if (ramlDialog
                                                                    .open() != Window.OK) {
                                                                return;
                                                            }
                                                            String configFilePath = ramlDialog
                                                                    .getConfigFilePath();
                                                            if (null == configFilePath
                                                                    || configFilePath
                                                                            .length() < 1) {
                                                                MessageDialog
                                                                        .openInformation(
                                                                                Display.getDefault()
                                                                                        .getActiveShell(),
                                                                                "Invalid RAML Config path",
                                                                                "Configuration file path is invalid.");
                                                                return;
                                                            }
                                                            resourceManager
                                                                    .setConfigFilePath(
                                                                            resourceManager
                                                                                    .getCurrentResourceInSelection(),
                                                                            configFilePath);
                                                        }
                                                    });
                                        } else {
                                            MessageDialog
                                                    .openInformation(Display
                                                            .getDefault()
                                                            .getActiveShell(),
                                                            "Already Uploaded",
                                                            "Configuration file for the selected resource is already uploaded");
                                        }
                                    }
                                });

                        RemoteResource resource = resourceManager
                                .getCurrentResourceInSelection();
                        if (null == resource) {
                            return;
                        }
                        String menuText = !resource.isFavorite() ? "Add to favorites"
                                : "Remove from favorites";
                        MenuItem addToFavMenuItem = new MenuItem(menu, SWT.NONE);
                        addToFavMenuItem.setText(menuText);
                        addToFavMenuItem
                                .addSelectionListener(new SelectionAdapter() {
                                    @Override
                                    public void widgetSelected(SelectionEvent e) {
                                        RemoteResource resource = (RemoteResource) ((IStructuredSelection) treeViewer
                                                .getSelection())
                                                .getFirstElement();
                                        if (null == resource) {
                                            return;
                                        }
                                        System.out.println("Selected resource:"
                                                + resource.getResourceURI());
                                        if (!resource.isFavorite()) {
                                            resourceManager
                                                    .addResourcetoFavorites(resource);
                                        } else {
                                            resourceManager
                                                    .removeResourceFromFavorites(resource);
                                            resourceManager
                                                    .removeResourceURIFromFavorites(resource);
                                        }
                                        favTreeViewer.refresh();
                                    }
                                });
                    }
                });
            }
        }
    }

    private void createFavoriteResourcesArea() {
        favResTab = new CTabItem(folder, SWT.NULL);
        favResTab.setText("Favorite Resources");

        // Create a group to show all the discovered resources.
        // Adding the group to the folder.
        Group resourceGroup = new Group(folder, SWT.NONE);
        // resourceGroup.setText("Discovered Resources");

        Color color = Display.getDefault().getSystemColor(SWT.COLOR_WHITE);
        resourceGroup.setBackground(color);

        resourceGroup.setLayout(new GridLayout(1, false));
        GridData gd = new GridData(SWT.FILL, SWT.FILL, true, true);
        resourceGroup.setLayoutData(gd);

        PatternFilter filter = new PatternFilter();
        FilteredTree filteredTree = new FilteredTree(resourceGroup,
                SWT.H_SCROLL | SWT.V_SCROLL | SWT.SINGLE, filter, true);
        favTreeViewer = filteredTree.getViewer();
        favTreeViewer.getTree().setLayoutData(
                new GridData(SWT.FILL, SWT.FILL, true, true));
        favTreeViewer.setContentProvider(new FavTreeContentProvider());
        favTreeViewer.setLabelProvider(new TreeLabelProvider());
        favTreeViewer.setInput(new Object());

        favResTab.setControl(resourceGroup);

        addMenuToFavResources();
    }

    private void addMenuToFavResources() {
        if (null != favTreeViewer) {
            final Tree resourceTreeHead = favTreeViewer.getTree();
            if (null != resourceTreeHead) {
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
                        MenuItem addToFavMenuItem = new MenuItem(menu, SWT.NONE);
                        addToFavMenuItem.setText("Remove from favorites");
                        addToFavMenuItem
                                .addSelectionListener(new SelectionAdapter() {
                                    @Override
                                    public void widgetSelected(SelectionEvent e) {
                                        RemoteResource resource = (RemoteResource) ((IStructuredSelection) favTreeViewer
                                                .getSelection())
                                                .getFirstElement();
                                        if (null == resource) {
                                            return;
                                        }
                                        resourceManager
                                                .removeResourceFromFavorites(resource);
                                        resourceManager
                                                .removeResourceURIFromFavorites(resource);
                                        favTreeViewer.refresh();
                                    }
                                });
                    }
                });
            }
        }
    }

    private void addUIListeners() {
        findResButton.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                PlatformUI.getWorkbench().getDisplay().syncExec(new Runnable() {

                    @Override
                    public void run() {
                        FindResourceWizard findWizard = new FindResourceWizard();
                        ResourceWizardDialog wizardDialog = new ResourceWizardDialog(
                                PlatformUI.getWorkbench().getDisplay()
                                        .getActiveShell(), findWizard);
                        int open = wizardDialog.open();
                        if (open == WizardDialog.OK) {
                            // Setting initial value on starting the find
                            // operation.
                            setFoundResource(false);

                            Set<String> searchTypes = findWizard
                                    .getSearchTypes();
                            if (null != searchTypes) {
                                System.out.println(searchTypes);
                                // Call native method to clear existing
                                // resources of
                                // the given search types.
                                resourceManager.deleteResources(searchTypes);

                                // Update the tree
                                treeViewer.refresh();
                                favTreeViewer.refresh();

                                // Call native method to find Resources
                                boolean result = resourceManager
                                        .findResourceRequest(searchTypes);
                                if (result) {
                                    searchUIOperation(false);
                                } else {
                                    MessageDialog
                                            .openError(Display.getDefault()
                                                    .getActiveShell(),
                                                    "Find Resource status",
                                                    "Operation failed due to some problems in core layer.");
                                }

                                // Store this information for refresh
                                // functionality
                                resourceManager
                                        .setLastKnownSearchTypes(searchTypes);

                                // Change the refresh visibility
                                refreshButton.setEnabled(true);
                            }
                        }
                    }
                });
            }
        });

        refreshButton.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                Set<String> searchTypes = resourceManager
                        .getLastKnownSearchTypes();
                if (null == searchTypes) {
                    return;
                }
                setFoundResource(false);

                // Call native method to clear existing resources of the given
                // search types.
                resourceManager.deleteResources(searchTypes);

                // Update the tree
                treeViewer.refresh();
                favTreeViewer.refresh();

                // Call native method to find Resources
                boolean result = resourceManager
                        .findResourceRequest(searchTypes);
                if (result) {
                    searchUIOperation(true);
                } else {
                    MessageDialog
                            .openError(Display.getDefault().getActiveShell(),
                                    "Find Resource status",
                                    "Operation failed due to some problems in core layer.");
                }
            }
        });

        // Below code adds a listener to the tree for selection changes
        // and notifies the resource manager
        ISelectionChangedListener treeSelectionListener = new ISelectionChangedListener() {

            @Override
            public void selectionChanged(SelectionChangedEvent e) {
                if (e.getSelection().isEmpty()) {
                    return;
                }
                if (e.getSelection() instanceof IStructuredSelection) {
                    IStructuredSelection selection = (IStructuredSelection) e
                            .getSelection();
                    RemoteResource resource = (RemoteResource) selection
                            .getFirstElement();
                    if (null == resource) {
                        return;
                    }
                    System.out.println("Selected resource: "
                            + resource.getResourceURI());
                    resourceManager.resourceSelectionChanged(resource);
                }
            }
        };

        treeViewer.addSelectionChangedListener(treeSelectionListener);
        favTreeViewer.addSelectionChangedListener(treeSelectionListener);
    }

    // If refresh is true, then Refresh Dialog else Find Dialog will be shown.
    private void searchUIOperation(boolean refresh) {
        final MessageDialog targetDialog;
        if (refresh) {
            targetDialog = refreshDialog;
        } else {
            targetDialog = findDialog;
        }
        // Open the dialog in a new thread.
        PlatformUI.getWorkbench().getDisplay().syncExec(new Thread() {

            @Override
            public void run() {
                if (isFoundResource()) {
                    setFoundResource(false);
                    return;
                }

                PlatformUI.getWorkbench().getDisplay().asyncExec(new Thread() {
                    @Override
                    public void run() {
                        targetDialog.open(); // This method returns once the
                        // cancel button is pressed.

                        // Interrupt the sleep thread.
                        if (null != sleepThreadHandle
                                && sleepThreadHandle.isAlive()) {
                            sleepThreadHandle.interrupt();
                        }

                        // Set the status of find.
                        setFoundResource(false);
                    }
                });

                // Thread for find time-out.
                sleepThreadHandle = new Thread() {
                    Thread child;

                    public void run() {
                        try {
                            Thread.sleep(Constants.FIND_RESOURCES_TIMEOUT * 1000);
                        } catch (InterruptedException e) {
                            System.out.println("Interrupted during sleep.");
                            return;
                        }

                        child = new Thread() {
                            @Override
                            public void run() {
                                if (null != targetDialog) {
                                    targetDialog.close();

                                    // Check if any new resources are
                                    // discovered.
                                    // Is no new resources, then display a
                                    // message box.
                                    if (!isFoundResource()) {
                                        MessageDialog
                                                .openInformation(
                                                        Display.getDefault()
                                                                .getActiveShell(),
                                                        "No servers found",
                                                        "No servers are available as of now.\n"
                                                                + "Please check the servers' status and press"
                                                                + "'Refresh' button to restart the search.");
                                    } else {
                                        // Resetting the status to false for
                                        // ensuring safety.
                                        setFoundResource(false);
                                    }
                                }
                            }
                        };

                        PlatformUI.getWorkbench().getDisplay().syncExec(child);
                    }
                };
                sleepThreadHandle.start();
            }
        });
    }

    private void addManagerListeners() {
        resourceManager.addFindresourceUIListener(findListener);
    }

    @Override
    public void dispose() {
        // Unregister the listener
        if (null != findListener) {
            resourceManager.removeFindresourceUIListener(findListener);
            resourceManager.resourceSelectionChanged(null);
        }
        super.dispose();
    }

    @Override
    public void setFocus() {
        // TODO Auto-generated method stub

    }

    public synchronized void setFoundResource(boolean value) {
        foundResource = value;
    }

    public synchronized boolean isFoundResource() {
        return foundResource;
    }
}

class TreeContentProvider implements ITreeContentProvider {

    @Override
    public void dispose() {
    }

    @Override
    public void inputChanged(Viewer viewer, Object oldInput, Object newInput) {

    }

    @Override
    public Object[] getChildren(Object parent) {
        return null;
    }

    @Override
    public Object[] getElements(Object parent) {
        System.out.println("Inside getElements()");
        List<RemoteResource> resourceList = Activator.getDefault()
                .getResourceManager().getResourceList();
        return resourceList.toArray();
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

class FavTreeContentProvider implements ITreeContentProvider {

    @Override
    public void dispose() {
    }

    @Override
    public void inputChanged(Viewer viewer, Object oldInput, Object newInput) {

    }

    @Override
    public Object[] getChildren(Object parent) {
        return null;
    }

    @Override
    public Object[] getElements(Object parent) {
        System.out.println("Inside getElements()");
        List<RemoteResource> resourceList = Activator.getDefault()
                .getResourceManager().getFavResourceList();
        return resourceList.toArray();
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
        RemoteResource resource = (RemoteResource) element;
        return resource.getResourceURI();
    }

    @Override
    public Image getImage(Object element) {
        RemoteResource resource = (RemoteResource) element;
        ResourceManager resourceManager = Activator.getDefault()
                .getResourceManager();
        return resourceManager.getImage(resource.getResourceURI());
    }
}