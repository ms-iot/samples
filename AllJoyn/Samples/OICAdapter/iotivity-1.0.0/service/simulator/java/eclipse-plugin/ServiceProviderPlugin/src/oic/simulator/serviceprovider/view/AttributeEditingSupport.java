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

import oic.simulator.serviceprovider.Activator;
import oic.simulator.serviceprovider.manager.ResourceManager;
import oic.simulator.serviceprovider.resource.AutomationSettingHelper;
import oic.simulator.serviceprovider.resource.LocalResourceAttribute;
import oic.simulator.serviceprovider.resource.SimulatorResource;
import oic.simulator.serviceprovider.utils.Utility;
import oic.simulator.serviceprovider.view.dialogs.AutomationSettingDialog;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.CellEditor;
import org.eclipse.jface.viewers.CheckboxCellEditor;
import org.eclipse.jface.viewers.ComboBoxCellEditor;
import org.eclipse.jface.viewers.EditingSupport;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.jface.window.Window;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.CCombo;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.MessageBox;
import org.oic.simulator.serviceprovider.AutomationType;

/**
 * This class provides editing support to the resources attributes table in the
 * attributes view.
 */
public class AttributeEditingSupport {

    private AttributeValueEditor attValueEditor;
    private AutomationEditor     automationEditor;

    public AttributeValueEditor createAttributeValueEditor(TableViewer viewer) {
        attValueEditor = new AttributeValueEditor(viewer);
        return attValueEditor;
    }

    public AutomationEditor createAutomationEditor(TableViewer viewer) {
        automationEditor = new AutomationEditor(viewer);
        return automationEditor;
    }

    class AttributeValueEditor extends EditingSupport {

        private final TableViewer      viewer;
        private LocalResourceAttribute attributeInSelection;
        private CCombo                 comboBox;

        public AttributeValueEditor(TableViewer viewer) {
            super(viewer);
            this.viewer = viewer;
        }

        @Override
        protected boolean canEdit(Object arg0) {
            return true;
        }

        @Override
        protected CellEditor getCellEditor(Object element) {
            attributeInSelection = (LocalResourceAttribute) element;

            // CellEditor is not required as the automation is in progress.
            if (attributeInSelection.isAutomationInProgress()) {
                return null;
            }

            String values[] = null;
            List<String> valueSet = attributeInSelection.getAttValues();
            values = convertListToStringArray(valueSet);

            ComboBoxCellEditor comboEditor = new ComboBoxCellEditor(
                    viewer.getTable(), values, SWT.READ_ONLY);
            comboBox = (CCombo) comboEditor.getControl();
            comboBox.addModifyListener(new ModifyListener() {

                @Override
                public void modifyText(ModifyEvent event) {
                    String oldValue = String.valueOf(attributeInSelection
                            .getAttributeValue());
                    String newValue = comboBox.getText();
                    if (!oldValue.equals(newValue)) {
                        attributeInSelection.setAttributeValue(newValue);
                        MessageBox dialog = new MessageBox(viewer.getTable()
                                .getShell(), SWT.ICON_QUESTION | SWT.OK
                                | SWT.CANCEL);
                        dialog.setText("Confirm action");
                        dialog.setMessage("Do you want to modify the value?");
                        int retval = dialog.open();
                        if (retval != SWT.OK) {
                            attributeInSelection.setAttributeValue(oldValue);
                        } else {
                            ResourceManager resourceManager;
                            resourceManager = Activator.getDefault()
                                    .getResourceManager();
                            SimulatorResource resource = resourceManager
                                    .getCurrentResourceInSelection();
                            resourceManager.attributeValueUpdated(resource,
                                    attributeInSelection.getAttributeName(),
                                    newValue);
                        }
                        viewer.update(attributeInSelection, null);
                        comboBox.setVisible(false);
                    }
                }
            });
            return comboEditor;
        }

        @Override
        protected Object getValue(Object element) {
            int indexOfItem = 0;
            LocalResourceAttribute att = (LocalResourceAttribute) element;
            String valueString = String.valueOf(att.getAttributeValue());
            List<String> valueSet = att.getAttValues();
            if (null != valueSet) {
                indexOfItem = valueSet.indexOf(valueString);
            }
            if (indexOfItem == -1) {
                indexOfItem = 0;
            }
            return indexOfItem;
        }

        @Override
        protected void setValue(Object element, Object value) {
            Object valueObj = attributeInSelection.getAttributeValue();
            if (null == valueObj)
                return;
            String attValue = String.valueOf(valueObj);
            ((LocalResourceAttribute) element).setAttributeValue(attValue);
            viewer.update(element, null);
        }

        public String[] convertListToStringArray(List<String> valueList) {
            String[] strArr;
            if (null != valueList && valueList.size() > 0) {
                strArr = valueList.toArray(new String[1]);
            } else {
                strArr = new String[1];
            }
            return strArr;
        }
    }

    class AutomationEditor extends EditingSupport {

        private final TableViewer viewer;

        public AutomationEditor(TableViewer viewer) {
            super(viewer);
            this.viewer = viewer;
        }

        @Override
        protected boolean canEdit(Object arg0) {
            return true;
        }

        @Override
        protected CellEditor getCellEditor(Object element) {
            // CellEditor is not required as the automation is in progress.
            ResourceManager resourceManager = Activator.getDefault()
                    .getResourceManager();
            SimulatorResource resource = resourceManager
                    .getCurrentResourceInSelection();
            if (null != resource && resource.isResourceAutomationInProgress()) {
                return null;
            }
            return new CheckboxCellEditor(null, SWT.CHECK | SWT.READ_ONLY);
        }

        @Override
        protected Object getValue(Object element) {
            LocalResourceAttribute att = (LocalResourceAttribute) element;
            return att.isAutomationInProgress();
        }

        @Override
        protected void setValue(Object element, Object value) {
            ResourceManager resourceManager = Activator.getDefault()
                    .getResourceManager();
            // As automation depends on the current resource in selection, its
            // presence is being checked.
            SimulatorResource resource = resourceManager
                    .getCurrentResourceInSelection();
            if (null == resource) {
                return;
            }

            LocalResourceAttribute att = (LocalResourceAttribute) element;
            boolean checked = (Boolean) value;
            if (checked) {
                // Start the automation
                // Fetch the settings data
                List<AutomationSettingHelper> automationSettings;
                automationSettings = AutomationSettingHelper
                        .getAutomationSettings(att);

                // Open the settings dialog
                AutomationSettingDialog dialog = new AutomationSettingDialog(
                        viewer.getTable().getShell(), automationSettings);
                dialog.create();
                if (dialog.open() == Window.OK) {
                    String automationType = dialog.getAutomationType();
                    String updateFreq = dialog.getUpdateFrequency();

                    AutomationType autoType = AutomationType
                            .valueOf(automationType);
                    int updFreq = Utility
                            .getUpdateIntervalFromString(updateFreq);
                    int autoId = resourceManager.startAutomation(resource, att,
                            autoType, updFreq);
                    if (autoId == -1) {
                        MessageDialog.openInformation(Display.getDefault()
                                .getActiveShell(), "Automation Status",
                                "Automation start failed!!");
                    } else {
                        viewer.update(element, null);
                    }
                }
            } else {
                // Stop the automation
                resourceManager.stopAutomation(resource, att,
                        att.getAutomationId());
                MessageDialog.openInformation(Display.getDefault()
                        .getActiveShell(), "Automation Status",
                        "Automation stopped.");
                viewer.update(element, null);
            }
        }
    }
}
