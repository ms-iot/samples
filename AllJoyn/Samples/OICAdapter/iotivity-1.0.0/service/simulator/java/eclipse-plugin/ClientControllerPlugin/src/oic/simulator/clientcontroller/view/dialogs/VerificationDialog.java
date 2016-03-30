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

import java.util.ArrayList;
import java.util.Map;
import java.util.Set;

import oic.simulator.clientcontroller.Activator;
import oic.simulator.clientcontroller.manager.ResourceManager;
import oic.simulator.clientcontroller.utils.Constants;

import org.eclipse.jface.dialogs.IDialogConstants;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.dialogs.TrayDialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Shell;

/**
 * Dialog for starting and stopping the automatic verifications.
 */
public class VerificationDialog extends TrayDialog {
    private Map<String, Boolean> automationStatus;

    public VerificationDialog(Shell shell, Map<String, Boolean> automationStatus) {
        super(shell);
        this.automationStatus = automationStatus;
    }

    @Override
    protected void configureShell(Shell shell) {
        super.configureShell(shell);
    }

    @Override
    protected Control createDialogArea(Composite parent) {
        Composite composite = (Composite) super.createDialogArea(parent);
        createAutomationGroup(composite);
        getShell().setText("Automation Settings");
        return composite;
    }

    /**
     * Dynamically creates a check-box list for enabling/disabling different
     * types of automation
     */
    private void createAutomationGroup(Composite parent) {
        Group group = new Group(parent, SWT.NONE);
        group.setLayout(new GridLayout());
        GridData gd = new GridData(GridData.FILL_HORIZONTAL);
        gd.widthHint = 275;
        group.setLayoutData(gd);
        group.setText("Automation Levels");

        Set<String> keySet = automationStatus.keySet();
        if (null == keySet) {
            return;
        }
        ArrayList<String> list = new ArrayList<String>(
                automationStatus.keySet());
        for (final String str : list) {
            final Button checkbox = new Button(group, SWT.CHECK);
            checkbox.setText(str);
            checkbox.setSelection(automationStatus.get(str));
            checkbox.addSelectionListener(new SelectionAdapter() {

                @Override
                public void widgetSelected(SelectionEvent e) {
                    boolean checked = checkbox.getSelection();
                    boolean answer = MessageDialog.openQuestion(Display
                            .getDefault().getActiveShell(), "Verification",
                            "Do you want to "
                                    + (checked ? "enable" : "disable")
                                    + " the verification?");
                    if (!answer) {
                        checkbox.setSelection(!checked);
                        checked = !checked;
                    } else {
                        ResourceManager resourceManager = Activator
                                .getDefault().getResourceManager();
                        String reqTypeTxt = checkbox.getText();
                        int reqType;
                        if (reqTypeTxt.equals("Get")) {
                            reqType = Constants.GET_AUTOMATION_INDEX;
                        } else if (reqTypeTxt.equals("Put")) {
                            reqType = Constants.PUT_AUTOMATION_INDEX;
                        } else {// if(reqTypeTxt.equals("Post")) {
                            reqType = Constants.POST_AUTOMATION_INDEX;
                        }
                        if (checked) {
                            resourceManager.startAutomationRequest(reqType,
                                    resourceManager
                                            .getCurrentResourceInSelection());
                        } else {
                            resourceManager.stopAutomationRequest(reqType,
                                    resourceManager
                                            .getCurrentResourceInSelection());
                        }
                    }
                    automationStatus.put(str, checked);
                }
            });
        }
    }

    @Override
    protected Button createButton(Composite parent, int id, String label,
            boolean defaultButton) {
        if (id == IDialogConstants.CANCEL_ID) {
            return null;
        }
        return super.createButton(parent, id, label, defaultButton);
    }

    @Override
    public boolean isHelpAvailable() {
        return false;
    }
}
