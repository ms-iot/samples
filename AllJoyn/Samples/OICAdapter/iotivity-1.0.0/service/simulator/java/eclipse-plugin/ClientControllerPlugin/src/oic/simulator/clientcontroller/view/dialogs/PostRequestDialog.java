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

package oic.simulator.clientcontroller.view.dialogs;

import java.util.List;

import oic.simulator.clientcontroller.Activator;
import oic.simulator.clientcontroller.manager.ResourceManager;
import oic.simulator.clientcontroller.remoteresource.PutPostAttributeModel;
import oic.simulator.clientcontroller.utils.Constants;

import org.eclipse.jface.dialogs.IDialogConstants;
import org.eclipse.jface.dialogs.TitleAreaDialog;
import org.eclipse.jface.viewers.CellEditor;
import org.eclipse.jface.viewers.CheckboxCellEditor;
import org.eclipse.jface.viewers.ColumnLabelProvider;
import org.eclipse.jface.viewers.EditingSupport;
import org.eclipse.jface.viewers.IStructuredContentProvider;
import org.eclipse.jface.viewers.StyledCellLabelProvider;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.jface.viewers.TableViewerColumn;
import org.eclipse.jface.viewers.TextCellEditor;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.viewers.ViewerCell;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Table;

/**
 * This dialog is used for generating a POST request.
 */
public class PostRequestDialog extends TitleAreaDialog {

    private TableViewer                 attTblViewer;

    private final String[]              attTblHeaders  = { "Name", "Value",
            "Select"                                  };
    private final Integer[]             attTblColWidth = { 200, 200, 50 };

    private ResourceManager             resourceManager;

    private List<PutPostAttributeModel> modelList      = null;

    public PostRequestDialog(Shell parentShell,
            List<PutPostAttributeModel> modelList) {
        super(parentShell);
        this.modelList = modelList;
        resourceManager = Activator.getDefault().getResourceManager();
    }

    @Override
    public void create() {
        super.create();
        setTitle("Generate POST Request");
        setMessage("Dialog which takes input and generates a post request.");
    }

    @Override
    protected Control createDialogArea(Composite parent) {
        Composite compLayout = (Composite) super.createDialogArea(parent);
        Composite container = new Composite(compLayout, SWT.NONE);
        container.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
        GridLayout layout = new GridLayout(1, false);
        layout.verticalSpacing = 10;
        layout.marginTop = 10;
        container.setLayout(layout);

        createTableViewer(container);

        attTblViewer.setInput(modelList.toArray());

        return compLayout;
    }

    private void createTableViewer(Composite parent) {
        attTblViewer = new TableViewer(parent, SWT.SINGLE | SWT.H_SCROLL
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

        // attributeEditor = new AttributeEditingSupport();

        TableViewerColumn attName = new TableViewerColumn(tableViewer, SWT.NONE);
        attName.getColumn().setWidth(attTblColWidth[0]);
        attName.getColumn().setText(attTblHeaders[0]);
        attName.setLabelProvider(new StyledCellLabelProvider() {
            @Override
            public void update(ViewerCell cell) {
                Object element = cell.getElement();
                if (element instanceof PutPostAttributeModel) {
                    PutPostAttributeModel entry = (PutPostAttributeModel) element;
                    cell.setText(entry.getAttName());
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
                if (element instanceof PutPostAttributeModel) {
                    PutPostAttributeModel entry = (PutPostAttributeModel) element;
                    cell.setText(entry.getAttValue());
                }
            }
        });
        attValue.setEditingSupport(new AttributeValueEditor(attTblViewer));

        TableViewerColumn updateColumn = new TableViewerColumn(tableViewer,
                SWT.NONE);
        updateColumn.getColumn().setWidth(attTblColWidth[2]);
        updateColumn.getColumn().setText(attTblHeaders[2]);
        updateColumn.setLabelProvider(new ColumnLabelProvider() {
            @Override
            public String getText(Object element) {
                return "";
            }

            @Override
            public Image getImage(Object element) {
                PutPostAttributeModel model = (PutPostAttributeModel) element;
                if (model.isModified()) {
                    return Activator.getDefault().getImageRegistry()
                            .get(Constants.CHECKED);
                }
                return Activator.getDefault().getImageRegistry()
                        .get(Constants.UNCHECKED);
            }
        });
        updateColumn.setEditingSupport(new UpdateEditor(attTblViewer));
    }

    @Override
    protected boolean isResizable() {
        return true;
    }

    @Override
    public boolean isHelpAvailable() {
        return false;
    }

    @Override
    protected Button createButton(Composite parent, int id, String label,
            boolean defaultButton) {
        if (id == IDialogConstants.OK_ID) {
            label = "POST";
        }
        return super.createButton(parent, id, label, defaultButton);
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

    class AttributeValueEditor extends EditingSupport {
        private final TableViewer viewer;
        private final CellEditor  editor;

        public AttributeValueEditor(TableViewer viewer) {
            super(viewer);
            this.viewer = viewer;
            editor = new TextCellEditor(viewer.getTable());
        }

        @Override
        protected boolean canEdit(Object arg0) {
            return true;
        }

        @Override
        protected CellEditor getCellEditor(Object element) {
            return editor;
        }

        @Override
        protected Object getValue(Object element) {
            PutPostAttributeModel model = (PutPostAttributeModel) element;
            return model.getAttValue();
        }

        @Override
        protected void setValue(Object element, Object value) {
            PutPostAttributeModel model = (PutPostAttributeModel) element;
            // Compare the actual value and the new value
            // If there is a change, then its corresponding check box should be
            // checked.
            String newValue = String.valueOf(value);
            String actualValue = resourceManager.getAttributeValue(
                    resourceManager.getCurrentResourceInSelection(),
                    model.getAttName());
            if (newValue.equals(actualValue)) {
                model.setModified(false);
            } else {
                model.setModified(true);
            }
            model.setAttValue(newValue);
            viewer.update(element, null);
        }
    }

    class UpdateEditor extends EditingSupport {

        private final TableViewer viewer;

        public UpdateEditor(TableViewer viewer) {
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
            PutPostAttributeModel model = (PutPostAttributeModel) element;
            return model.isModified();
        }

        @Override
        protected void setValue(Object element, Object value) {
            PutPostAttributeModel model = (PutPostAttributeModel) element;
            boolean status = (boolean) value;
            model.setModified(status);
            viewer.update(element, null);
        }
    }
}
