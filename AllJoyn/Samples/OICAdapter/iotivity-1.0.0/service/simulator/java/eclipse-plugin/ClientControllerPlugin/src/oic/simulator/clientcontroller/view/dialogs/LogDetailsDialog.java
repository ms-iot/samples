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

import java.text.DateFormat;
import java.util.Date;

import org.eclipse.jface.dialogs.Dialog;
import org.eclipse.jface.dialogs.IDialogConstants;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.layout.RowData;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;

/**
 * This class shows detailed information about a log. The dialog will be opened
 * on double-clicking a log entry in the log view.
 */
public class LogDetailsDialog extends Dialog {
    private final String severity;
    private final Date   date;
    private final String message;
    private final Image  severityIcon;

    public LogDetailsDialog(Shell parentShell, String severity,
            Image severityIcon, Date date, String message) {
        super(parentShell);
        this.severity = severity;
        this.severityIcon = severityIcon;
        this.message = message;
        this.date = date;
    }

    @Override
    protected boolean isResizable() {
        return true;
    }

    @Override
    protected void configureShell(Shell shell) {
        super.configureShell(shell);
    }

    @Override
    protected Control createDialogArea(Composite parent) {
        getShell().setText("Logged event details");

        Composite container = (Composite) super.createDialogArea(parent);

        GridData layoutData = new GridData(SWT.FILL, SWT.FILL, true, true);
        container.setLayoutData(layoutData);
        container.setLayout(new GridLayout(2, false));

        GridData gd;

        Label l1 = new Label(container, SWT.NONE);
        l1.setText("Severity:");
        gd = new GridData();
        gd.widthHint = 100;
        l1.setLayoutData(gd);

        Composite y = new Composite(container, SWT.NONE);
        gd = new GridData();
        gd.grabExcessHorizontalSpace = true;
        y.setLayoutData(gd);
        y.setLayout(new RowLayout(SWT.HORIZONTAL));

        Label l2 = new Label(y, SWT.NONE);
        l2.setImage(severityIcon);
        l2.setLayoutData(new RowData());
        Label l3 = new Label(y, SWT.NONE);
        l3.setText(severity);
        l3.setLayoutData(new RowData());

        Label l4 = new Label(container, SWT.NONE);
        l4.setText("Date:");
        gd = new GridData();
        gd.widthHint = 100;
        l4.setLayoutData(gd);

        Label l5 = new Label(container, SWT.NONE);
        DateFormat dateFormat = DateFormat.getDateTimeInstance(
                DateFormat.SHORT, DateFormat.SHORT);
        l5.setText(dateFormat.format(date));
        gd = new GridData();
        gd.grabExcessHorizontalSpace = true;
        l5.setLayoutData(gd);

        new Label(container, SWT.NONE); // separator

        Label l6 = new Label(container, SWT.NONE);
        l6.setText("Message details");
        gd = new GridData();
        gd.horizontalSpan = 2;
        l6.setLayoutData(gd);

        Text text = new Text(container, SWT.MULTI | SWT.READ_ONLY
                | SWT.H_SCROLL | SWT.V_SCROLL | SWT.BORDER);
        if (message != null) {
            text.setText(message);
        } else {
            text.setText("No description available");
        }
        gd = new GridData(SWT.FILL, SWT.FILL, true, true);
        gd.horizontalSpan = 2;
        gd.heightHint = 350;
        gd.widthHint = 500;
        text.setLayoutData(gd);

        return container;
    }

    @Override
    protected Button createButton(Composite parent, int id, String label,
            boolean defaultButton) {
        if (id == IDialogConstants.CANCEL_ID) {
            return null;
        }
        return super.createButton(parent, id, label, defaultButton);
    }
}
