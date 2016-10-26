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
import java.util.Collections;
import java.util.Map;

import oic.simulator.clientcontroller.manager.LogManager;

import org.eclipse.jface.dialogs.TrayDialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Shell;

/**
 * This class shows a dialog for filtering logs based on severity levels.
 */
public class FilterDialog extends TrayDialog {
    private Map<Integer, Boolean> severities;

    public FilterDialog(Shell shell, Map<Integer, Boolean> severities) {
        super(shell);
        this.severities = severities;
    }

    @Override
    protected void configureShell(Shell shell) {
        super.configureShell(shell);
    }

    @Override
    protected Control createDialogArea(Composite parent) {
        Composite composite = (Composite) super.createDialogArea(parent);
        createSeverityGroup(composite);
        getShell().setText("Filter details");
        return composite;
    }

    /**
     * Dynamically creates a check-box list for severity levels for user to
     * choose from
     */
    private void createSeverityGroup(Composite parent) {
        Group group = new Group(parent, SWT.NONE);
        group.setLayout(new GridLayout());
        GridData gd = new GridData(GridData.FILL_HORIZONTAL);
        gd.widthHint = 275;
        group.setLayoutData(gd);
        group.setText("Severity Levels");

        ArrayList<Integer> arrayList = new ArrayList<Integer>(
                severities.keySet());
        Collections.sort(arrayList);
        for (final Integer i : arrayList) {
            final Button checkbox = new Button(group, SWT.CHECK);
            checkbox.setText(LogManager.getSeverityName(i));
            checkbox.setSelection(severities.get(i));
            checkbox.addSelectionListener(new SelectionAdapter() {

                @Override
                public void widgetSelected(SelectionEvent e) {
                    severities.put(i, checkbox.getSelection());
                }
            });
        }
    }

    @Override
    public boolean isHelpAvailable() {
        return false;
    }
}