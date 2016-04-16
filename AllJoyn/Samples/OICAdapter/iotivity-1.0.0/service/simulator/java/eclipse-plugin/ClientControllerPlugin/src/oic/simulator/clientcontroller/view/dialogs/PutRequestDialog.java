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

import oic.simulator.clientcontroller.remoteresource.PutPostAttributeModel;

import org.eclipse.jface.dialogs.IDialogConstants;
import org.eclipse.jface.dialogs.TitleAreaDialog;
import org.eclipse.jface.viewers.CellEditor;
import org.eclipse.jface.viewers.EditingSupport;
import org.eclipse.jface.viewers.IStructuredContentProvider;
import org.eclipse.jface.viewers.StyledCellLabelProvider;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.jface.viewers.TableViewerColumn;
import org.eclipse.jface.viewers.TextCellEditor;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.viewers.ViewerCell;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Table;

/**
 * This dialog is used for generating a PUT request.
 */
public class PutRequestDialog extends TitleAreaDialog {

    private TableViewer                 attTblViewer;

    private final String[]              attTblHeaders  = { "Name", "Value" };
    private final Integer[]             attTblColWidth = { 200, 200 };

    private List<PutPostAttributeModel> modelList      = null;

    public PutRequestDialog(Shell parentShell,
            List<PutPostAttributeModel> modelList) {
        super(parentShell);
        this.modelList = modelList;
    }

    @Override
    public void create() {
        super.create();
        setTitle("Generate PUT Request");
        setMessage("Dialog which takes input and generates a put request.");
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
            model.setAttValue(String.valueOf(value));
            viewer.update(element, null);
        }
    }

    public List<PutPostAttributeModel> getUpdatedModel() {
        return modelList;
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
            label = "PUT";
        }
        return super.createButton(parent, id, label, defaultButton);
    }
}
