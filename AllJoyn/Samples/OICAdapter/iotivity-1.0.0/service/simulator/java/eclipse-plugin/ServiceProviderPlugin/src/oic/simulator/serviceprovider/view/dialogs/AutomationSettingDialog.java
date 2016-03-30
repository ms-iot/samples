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

package oic.simulator.serviceprovider.view.dialogs;

import java.util.Iterator;
import java.util.List;

import oic.simulator.serviceprovider.resource.AutomationSettingHelper;
import oic.simulator.serviceprovider.utils.Constants;

import org.eclipse.jface.dialogs.TitleAreaDialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.CCombo;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;

/**
 * This class manages and shows the automation settings dialog from the
 * attribute view.
 */
public class AutomationSettingDialog extends TitleAreaDialog {

    private CCombo                        autoTypeCmb;
    private CCombo                        updateFreqCmb;

    private String                        automationType;
    private String                        updateFrequencyInMillis;
    private List<AutomationSettingHelper> automationSettings;

    public AutomationSettingDialog(Shell parentShell,
            List<AutomationSettingHelper> automationSettings) {
        super(parentShell);
        this.automationSettings = automationSettings;
    }

    @Override
    public void create() {
        super.create();
        setTitle("Automation Settings");
        setMessage("Fill the automation settings for the attribute");
    }

    @Override
    protected Control createDialogArea(Composite parent) {
        Composite compLayout = (Composite) super.createDialogArea(parent);
        Composite container = new Composite(compLayout, SWT.NONE);
        container.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
        GridLayout layout = new GridLayout(2, false);
        layout.verticalSpacing = 10;
        layout.marginTop = 10;
        container.setLayout(layout);

        GridData gd;

        Label autoTypeLbl = new Label(container, SWT.NONE);
        autoTypeLbl.setText("Automation Type");

        autoTypeCmb = new CCombo(container, SWT.READ_ONLY | SWT.BORDER);
        gd = new GridData();
        gd.horizontalAlignment = SWT.FILL;
        gd.grabExcessHorizontalSpace = true;
        autoTypeCmb.setLayoutData(gd);

        Label updateFreqLbl = new Label(container, SWT.NONE);
        updateFreqLbl.setText("Update Frequency(ms)");

        updateFreqCmb = new CCombo(container, SWT.READ_ONLY | SWT.BORDER);
        gd = new GridData();
        gd.horizontalAlignment = SWT.FILL;
        gd.grabExcessHorizontalSpace = true;
        updateFreqCmb.setLayoutData(gd);

        populateSettingsData();

        addUIListeners();

        setInitialSettings();

        return compLayout;
    }

    public void populateSettingsData() {
        Iterator<AutomationSettingHelper> settingItr = automationSettings
                .iterator();
        AutomationSettingHelper setting;
        String settingId;
        String value;
        List<String> allowedValues;
        Iterator<String> itr;
        while (settingItr.hasNext()) {
            setting = settingItr.next();
            settingId = setting.getSettingID();
            value = setting.getSettingValue();
            allowedValues = setting.getAllowedValues();
            if (settingId.equals(Constants.AUTOMATION_TYPE)) {
                itr = allowedValues.iterator();
                while (itr.hasNext()) {
                    autoTypeCmb.add(itr.next());
                }
                // Select the default value
                autoTypeCmb.select(autoTypeCmb.indexOf(value));
            } else if (settingId.equals(Constants.UPDATE_INTERVAL_IN_MS)) {
                itr = allowedValues.iterator();
                while (itr.hasNext()) {
                    updateFreqCmb.add(itr.next());
                }
                // Select the default value
                updateFreqCmb.select(updateFreqCmb.indexOf(value));
            }
        }
    }

    public void addUIListeners() {
        autoTypeCmb.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                automationType = autoTypeCmb.getText();
            }
        });

        updateFreqCmb.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                updateFrequencyInMillis = updateFreqCmb.getText();
            }
        });
    }

    public void setInitialSettings() {
        automationType = autoTypeCmb.getText();
        updateFrequencyInMillis = updateFreqCmb.getText();
    }

    public String getAutomationType() {
        return automationType;
    }

    public String getUpdateFrequency() {
        return updateFrequencyInMillis;
    }

    @Override
    protected boolean isResizable() {
        return true;
    }

    @Override
    public boolean isHelpAvailable() {
        return false;
    }
}
