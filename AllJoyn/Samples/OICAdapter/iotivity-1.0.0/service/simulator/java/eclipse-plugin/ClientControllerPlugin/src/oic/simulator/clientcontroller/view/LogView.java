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

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.HashMap;
import java.util.List;

import oic.simulator.clientcontroller.Activator;
import oic.simulator.clientcontroller.listener.ILogUIListener;
import oic.simulator.clientcontroller.manager.LogManager;
import oic.simulator.clientcontroller.utils.Constants;
import oic.simulator.clientcontroller.view.dialogs.FilterDialog;
import oic.simulator.clientcontroller.view.dialogs.LogDetailsDialog;
import oic.simulator.logger.LogContentProvider;
import oic.simulator.logger.LogEntry;
import oic.simulator.logger.LogLabelProvider;

import org.eclipse.jface.action.Action;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.action.IContributionItem;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.IToolBarManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.jface.action.Separator;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.viewers.DoubleClickEvent;
import org.eclipse.jface.viewers.IDoubleClickListener;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.jface.viewers.TreeViewer;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.viewers.ViewerComparator;
import org.eclipse.jface.window.Window;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.KeyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Text;
import org.eclipse.swt.widgets.Tree;
import org.eclipse.swt.widgets.TreeColumn;
import org.eclipse.swt.widgets.TreeItem;
import org.eclipse.ui.IActionBars;
import org.eclipse.ui.IWorkbenchActionConstants;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.dialogs.FilteredTree;
import org.eclipse.ui.dialogs.PatternFilter;
import org.eclipse.ui.part.ViewPart;
import org.oic.simulator.ILogger.Level;

/**
 * This class manages and shows the log view in the perspective.
 */
public class LogView extends ViewPart {

    public static final String        VIEW_ID              = "oic.simulator.clientcontroller.view.log";

    private LogManager                logManager;
    private ILogUIListener            logListener;

    private LogContentProvider        treeContentProvider;

    private FilteredTree              tree;
    private TreeColumn                severityColumn;
    private TreeColumn                dateColumn;
    private TreeColumn                messageColumn;

    private IAction                   exportLogAction;
    private IAction                   clearLogAction;
    private IAction                   deleteLogAction;
    private IAction                   scrollLockAction;
    private IAction                   logDetailsAction;
    private IAction                   filterAction;
    private IAction                   activateViewAction;
    private IAction                   showTextFilter;
    private IContributionItem         groupByAction;

    private HashMap<Integer, Boolean> shownSeverities      = new HashMap<Integer, Boolean>();

    private boolean                   activateOnChange     = false;

    private boolean                   hideTextFilter       = false;

    private boolean                   scrollLockDisabled;

    public static final int           ORDER_BY_TIME        = 0;
    public static final int           ORDER_BY_SEVERITY    = 1;
    public static final int           ORDER_BY_MESSAGE     = 2;

    int                               sortCandidate        = ORDER_BY_TIME;

    SortAction                        sortByTimeAction     = new SortAction(
                                                                   "Order by Time",
                                                                   ORDER_BY_TIME);
    SortAction                        sortBySeverityAction = new SortAction(
                                                                   "Order by Severity",
                                                                   ORDER_BY_SEVERITY);
    SortAction                        sortByMessageAction  = new SortAction(
                                                                   "Order by Message",
                                                                   ORDER_BY_MESSAGE);

    private ViewerComparator          dateComparator;
    private ViewerComparator          severityComparator;
    private ViewerComparator          messageComparator;

    private TreeColumn                sortColumn           = null;
    private static int                DOWN                 = 1;
    private static int                UP                   = -1;
    private int                       sortDirection        = DOWN;

    public LogView() {

        logListener = new ILogUIListener() {

            @Override
            public void logChanged(final List<LogEntry> entry) {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {
                        TreeViewer viewer = tree.getViewer();
                        if (viewer.getControl().isDisposed()) {
                            return;
                        }
                        viewer.setInput(entry);
                        updateTree(false);
                    }
                });
            }

            @Override
            public void logAdded(final LogEntry added) {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {
                        TreeViewer viewer = tree.getViewer();
                        if (viewer.getControl().isDisposed()) {
                            return;
                        }
                        LogContentProvider provider = (LogContentProvider) viewer
                                .getContentProvider();
                        provider.addLog(added);
                        tree.getViewer().add(viewer.getInput(), added);
                        @SuppressWarnings("unchecked")
                        List<LogEntry> input = (List<LogEntry>) viewer
                                .getInput();
                        if (input.size() > Constants.LOG_SIZE) {
                            viewer.remove(viewer.getInput(), 0);
                        }
                        updateTree(true);
                    }
                });
            }

            private void updateTree(boolean needscroll) {
                if (activateOnChange) {
                    IWorkbenchPage page = Activator.getDefault().getWorkbench()
                            .getActiveWorkbenchWindow().getActivePage();
                    if (page != null) {
                        page.bringToTop(LogView.this);
                    }
                }
                if (scrollLockDisabled && needscroll) {
                    Tree tree2 = tree.getViewer().getTree();
                    if (tree2.getItemCount() > 0) {
                        TreeItem item = tree2.getItem(tree2.getItemCount() - 1);
                        tree2.setTopItem(item);
                        deleteLogAction.setEnabled(true);
                    }
                }
            }
        };

        logManager = Activator.getDefault().getLogManager();

        // Initially state of scroll lock
        scrollLockDisabled = true;
    }

    @Override
    public void createPartControl(Composite parent) {
        PatternFilter filter = new PatternFilter() {

            DateFormat dateFormat = new SimpleDateFormat("HH:mm:ss.SSS");

            @Override
            protected boolean isLeafMatch(Viewer viewer, Object element) {
                if (element instanceof LogEntry) {
                    LogEntry logEntry = (LogEntry) element;
                    String severity = LogManager.getSeverityName(logEntry
                            .getSeverity());
                    String date = dateFormat.format(logEntry.getDate());
                    String message = logEntry.getMessage();
                    return wordMatches(severity) || wordMatches(date)
                            || wordMatches(message);
                }
                return false;
            }
        };
        filter.setIncludeLeadingWildcard(true);
        tree = new FilteredTree(parent, SWT.SINGLE | SWT.H_SCROLL
                | SWT.V_SCROLL | SWT.FULL_SELECTION, filter, true);

        setupFilteredTree();

        createColumnComparators();

        createActions();

        setDefaultShownSeverities();

        IActionBars actionBars = getViewSite().getActionBars();
        IToolBarManager toolBarManager = actionBars.getToolBarManager();
        toolBarManager.add(exportLogAction);
        toolBarManager.add(clearLogAction);
        toolBarManager.add(deleteLogAction);
        toolBarManager.add(scrollLockAction);
        toolBarManager
                .add(new Separator(IWorkbenchActionConstants.MB_ADDITIONS));

        IMenuManager mgr = actionBars.getMenuManager();
        mgr.add(groupByAction);
        mgr.add(new Separator());
        mgr.add(filterAction);
        mgr.add(new Separator());
        mgr.add(activateViewAction);
        mgr.add(showTextFilter);

        addManagerListeners();

        if (sortCandidate == ORDER_BY_TIME) {
            sortByTimeAction.run();
        } else if (sortCandidate == ORDER_BY_SEVERITY) {
            sortBySeverityAction.run();
        } else { // order_selected == ORDER_BY_NONE
            sortByMessageAction.run();
        }

    }

    private void setupFilteredTree() {
        tree.setLayoutData(new GridData(GridData.FILL_BOTH));
        final Tree innerTree = tree.getViewer().getTree();
        innerTree.setLinesVisible(true);

        severityColumn = new TreeColumn(innerTree, SWT.LEFT);
        severityColumn.setText("Severity");
        severityColumn.setWidth(110);
        severityColumn.addSelectionListener(new SelectionAdapter() {

            @Override
            public void widgetSelected(SelectionEvent e) {
                sortBySeverityAction.run();
            }
        });
        dateColumn = new TreeColumn(innerTree, SWT.LEFT);
        dateColumn.setText("Time");
        dateColumn.setWidth(110);
        dateColumn.addSelectionListener(new SelectionAdapter() {

            @Override
            public void widgetSelected(SelectionEvent e) {
                sortByTimeAction.run();
            }
        });
        messageColumn = new TreeColumn(innerTree, SWT.LEFT);
        messageColumn.setText("Message");
        messageColumn.setWidth(180);
        messageColumn.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                sortByMessageAction.run();
            }
        });

        innerTree.setHeaderVisible(true);

        treeContentProvider = new LogContentProvider();

        tree.getViewer().setContentProvider(treeContentProvider);
        tree.getViewer().setLabelProvider(new LogLabelProvider());

        tree.getViewer().setInput(logManager.getLogEntries());

        tree.getViewer().addSelectionChangedListener(
                new ISelectionChangedListener() {

                    @Override
                    public void selectionChanged(SelectionChangedEvent event) {
                        deleteLogAction.setEnabled(!tree.getViewer()
                                .getSelection().isEmpty());
                        logDetailsAction.setEnabled(!tree.getViewer()
                                .getSelection().isEmpty());
                    }
                });

        tree.getViewer().getTree().addKeyListener(new KeyListener() {

            @Override
            public void keyReleased(KeyEvent e) {
            }

            @Override
            public void keyPressed(KeyEvent e) {
                if (e.character == (char) 127) { // If delete key is pressed
                    if (deleteLogAction.isEnabled()) {
                        deleteLogAction.run();
                    }
                }
            }
        });

        tree.getViewer().addDoubleClickListener(new IDoubleClickListener() {

            @Override
            public void doubleClick(DoubleClickEvent event) {
                logDetailsAction.run();
            }
        });
    }

    private void createColumnComparators() {
        dateComparator = new ViewerComparator() {

            @Override
            public int compare(Viewer viewer, Object e1, Object e2) {
                LogEntry l1 = (LogEntry) e1;
                LogEntry l2 = (LogEntry) e2;
                return l1.getDate().compareTo(l2.getDate()) * sortDirection;
            }
        };

        severityComparator = new ViewerComparator() {

            @Override
            public int compare(Viewer viewer, Object e1, Object e2) {
                LogEntry l1 = (LogEntry) e1;
                LogEntry l2 = (LogEntry) e2;
                if (l1.getSeverity() < l2.getSeverity()) {
                    return -1 * sortDirection;
                }
                if (l1.getSeverity() > l2.getSeverity()) {
                    return 1 * sortDirection;
                }
                return 0;
            }
        };

        messageComparator = new ViewerComparator() {

            @Override
            public int compare(Viewer viewer, Object e1, Object e2) {
                LogEntry l1 = (LogEntry) e1;
                LogEntry l2 = (LogEntry) e2;
                return l1.getMessage().compareTo(l2.getMessage())
                        * sortDirection;
            }
        };

    }

    private void setDefaultShownSeverities() {
        shownSeverities.put(Level.INFO.ordinal(), true);
        shownSeverities.put(Level.DEBUG.ordinal(), true);
        shownSeverities.put(Level.WARNING.ordinal(), true);
        shownSeverities.put(Level.ERROR.ordinal(), true);
    }

    private void addManagerListeners() {
        logManager.addLogListener(logListener);
        logManager.applyFilter(shownSeverities);
    }

    private void createActions() {
        exportLogAction = createExportLogAction();
        clearLogAction = createClearLogAction();
        deleteLogAction = createDeleteLogAction();
        scrollLockAction = createScrollLockAction();
        logDetailsAction = createLogDetailsAction();

        filterAction = createFilterAction();
        activateViewAction = createActivateViewAction();
        showTextFilter = createShowTextFilter();
        groupByAction = createGroupByAction();
    }

    private IAction createExportLogAction() {
        Action action = new Action("Export log") {
            @Override
            public void run() {
                FileDialog fd = new FileDialog(Display.getDefault()
                        .getActiveShell(), SWT.SAVE);
                fd.setOverwrite(true);
                fd.setFileName("OIC_Simulator_ServerLog.log");
                fd.setFilterExtensions(Constants.SAVE_LOG_FILTER_EXTENSIONS);
                String name = fd.open();
                List<LogEntry> logEntries = logManager.getLogEntries();
                StringBuilder sb = new StringBuilder();
                for (LogEntry entry : logEntries) {
                    sb.append(entry.toString());
                }
                String data = sb.toString();
                BufferedWriter out = null;
                try {
                    out = new BufferedWriter(new FileWriter(name));
                    out.write(data);
                } catch (IOException e) {
                    e.printStackTrace();
                    MessageDialog.openError(
                            Display.getDefault().getActiveShell(),
                            "Export error",
                            "Could not export log. IO exception: "
                                    + e.getMessage());
                } finally {
                    try {
                        if (null != out) {
                            out.close();
                        }
                    } catch (IOException e) {
                        System.out.println("Error occurred during close.");
                    }
                }
            }
        };
        action.setToolTipText("Export log");

        action.setImageDescriptor(ImageDescriptor.createFromFile(
                this.getClass(), "/icons/export_log_e.gif"));
        action.setDisabledImageDescriptor(ImageDescriptor.createFromFile(
                this.getClass(), "/icons/export_log_d.gif"));
        action.setEnabled(true);

        return action;
    }

    private IAction createClearLogAction() {
        Action action = new Action("Clear log") {

            @Override
            public void run() {
                logManager.clearLog();
            }
        };
        action.setToolTipText("Clear log");

        action.setImageDescriptor(ImageDescriptor.createFromFile(
                this.getClass(), "/icons/clear_e.gif"));
        action.setDisabledImageDescriptor(ImageDescriptor.createFromFile(
                this.getClass(), "/icons/clear_d.gif"));
        action.setEnabled(true);
        return action;
    }

    private IAction createDeleteLogAction() {
        Action action = new Action("Delete log entry") {

            @Override
            @SuppressWarnings("unchecked")
            public void run() {
                IStructuredSelection selection = (IStructuredSelection) tree
                        .getViewer().getSelection();
                List<LogEntry> entries = (List<LogEntry>) tree.getViewer()
                        .getInput();
                LogEntry selectedEntry = (LogEntry) selection.getFirstElement();
                if (null != selectedEntry) {
                    LogEntry toBeShownEntry = null;
                    for (LogEntry entry : entries) {
                        if (entry.equals(selectedEntry)) {
                            int size = entries.size();
                            int index = entries.indexOf(selectedEntry);
                            if (index + 1 < size) {
                                toBeShownEntry = entries.get(index + 1);
                            } else if (index > 0) {
                                toBeShownEntry = entries.get(index - 1);
                            }
                            break;
                        }
                    }
                    logManager.removeEntry(selectedEntry);
                    if (null != toBeShownEntry) {
                        tree.getViewer().setSelection(
                                new StructuredSelection(toBeShownEntry));
                    }
                }
            }
        };
        action.setToolTipText("Delete log entry");
        action.setImageDescriptor(ImageDescriptor.createFromFile(
                this.getClass(), "/icons/delete_e.gif"));
        action.setDisabledImageDescriptor(ImageDescriptor.createFromFile(
                this.getClass(), "/icons/delete_d.gif"));
        action.setEnabled(false);
        return action;
    }

    private IAction createScrollLockAction() {
        Action action = new Action("Scroll lock") {

            @Override
            public void run() {
                scrollLockDisabled = !this.isChecked();
            };

            @Override
            public int getStyle() {
                return IAction.AS_CHECK_BOX;
            }
        };
        action.setToolTipText("Scroll lock");
        action.setImageDescriptor(ImageDescriptor.createFromFile(
                this.getClass(), "/icons/lock_e.gif"));
        action.setDisabledImageDescriptor(ImageDescriptor.createFromFile(
                this.getClass(), "/icons/lock_d.gif"));
        action.setEnabled(true);
        return action;
    }

    private IAction createLogDetailsAction() {
        Action action = new Action("Details...") {

            @Override
            public void run() {
                Display.getDefault().asyncExec(new Runnable() {

                    @Override
                    public void run() {
                        LogEntry x = (LogEntry) ((IStructuredSelection) tree
                                .getViewer().getSelection()).getFirstElement();

                        new LogDetailsDialog(Display.getDefault()
                                .getActiveShell(), LogManager.getSeverityName(x
                                .getSeverity()), LogManager.getSeverityIcon(x
                                .getSeverity()), x.getDate(), x.getMessage())
                                .open();
                    }
                });
            }
        };
        action.setToolTipText("Details...");
        action.setImageDescriptor(ImageDescriptor.createFromFile(
                this.getClass(), "/icons/log_details_e.gif"));
        action.setDisabledImageDescriptor(ImageDescriptor.createFromFile(
                this.getClass(), "/icons/log_details_e.gif"));
        action.setEnabled(false);
        return action;
    }

    private IAction createFilterAction() {
        Action action = new Action("Filters ...") {

            @Override
            public void run() {
                FilterDialog fd = new FilterDialog(Display.getDefault()
                        .getActiveShell(), shownSeverities);
                if (fd.open() == Window.OK) {
                    logManager.applyFilter(shownSeverities);
                }
                tree.getViewer().refresh();
            }
        };
        action.setToolTipText("Filters ...");

        action.setImageDescriptor(ImageDescriptor.createFromFile(
                this.getClass(), "/icons/filter_e.gif"));
        action.setDisabledImageDescriptor(ImageDescriptor.createFromFile(
                this.getClass(), "/icons/filter_d.gif"));
        action.setEnabled(true);
        return action;
    }

    private IAction createActivateViewAction() {
        Action action = new Action("Activate view on new events",
                IAction.AS_CHECK_BOX) {

            @Override
            public void run() {
                activateOnChange = this.isChecked();
            }
        };
        action.setChecked(activateOnChange);
        action.setToolTipText("Activate view on new events");

        action.setImageDescriptor(ImageDescriptor.createFromFile(
                this.getClass(), "/icons/prop_e.gif"));
        action.setDisabledImageDescriptor(ImageDescriptor.createFromFile(
                this.getClass(), "/icons/prop_d.gif"));
        action.setEnabled(true);
        return action;
    }

    private IAction createShowTextFilter() {
        Action action = new Action("Show text filter", IAction.AS_CHECK_BOX) {

            @Override
            public void run() {
                Text filterControl = tree.getFilterControl();
                Composite filterComposite = filterControl.getParent();
                GridData gd = (GridData) filterComposite.getLayoutData();
                boolean visible = isChecked();
                gd.exclude = !visible;
                filterComposite.setVisible(visible);
                filterControl.setText("");
                if (visible) {
                    filterControl.selectAll();
                    setFocus();
                }
                tree.layout(false);
                hideTextFilter = !visible;
            }
        };
        action.setToolTipText("Show text filter");

        action.setImageDescriptor(ImageDescriptor.createFromFile(
                this.getClass(), "/icons/tree_mode_e.gif"));
        action.setDisabledImageDescriptor(ImageDescriptor.createFromFile(
                this.getClass(), "/icons/tree_mode_d.gif"));
        action.setEnabled(true);
        action.setChecked(!hideTextFilter);
        if (hideTextFilter) {
            action.run();
        }
        return action;
    }

    private IContributionItem createGroupByAction() {
        IMenuManager manager = new MenuManager("Order by");
        manager.add(sortByTimeAction);
        manager.add(sortBySeverityAction);
        manager.add(sortByMessageAction);
        return manager;
    }

    class SortAction extends Action {

        private final int sortBy;

        public SortAction(String text, int sortBy) {
            super(text, IAction.AS_RADIO_BUTTON);
            this.sortBy = sortBy;

            if (sortCandidate == sortBy) {
                setChecked(true);
            }
        }

        @Override
        public void run() {
            sortBySeverityAction.setChecked(false);
            sortByTimeAction.setChecked(false);
            sortCandidate = sortBy;
            setChecked(true);

            ViewerComparator comparator;
            TreeColumn column;
            if (sortBy == ORDER_BY_SEVERITY) {
                comparator = severityComparator;
                column = severityColumn;
            } else if (sortBy == ORDER_BY_TIME) {
                comparator = dateComparator;
                column = dateColumn;
            } else { // Order by message
                comparator = messageComparator;
                column = messageColumn;
            }
            TreeViewer viewer = tree.getViewer();
            viewer.setComparator(comparator);
            viewer.getTree().setSortColumn(column);
            if (column.equals(sortColumn)) { // reverse sorting order
                sortDirection = viewer.getTree().getSortDirection() == SWT.UP ? DOWN
                        : UP;
                viewer.getTree().setSortDirection(
                        sortDirection == UP ? SWT.UP : SWT.DOWN);
                viewer.refresh();
            } else { // set this column as the one to sort by
                sortDirection = DOWN;
                viewer.getTree().setSortDirection(SWT.DOWN);
            }
            sortColumn = column;
            refresh();
        }
    }

    private void refresh() {
        tree.getViewer().refresh();
    }

    @Override
    public void setFocus() {
        tree.setFocus();
    }

    @Override
    public void dispose() {
        logManager.removeLogListener(logListener);
        super.dispose();
    }
}