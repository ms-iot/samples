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

import java.util.HashSet;
import java.util.Set;

import oic.simulator.clientcontroller.utils.Constants;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.CCombo;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

/**
 * This class shows UI for finding resources.
 */
public class FindResourcePage extends WizardPage {

    private Button      stdResTypeRbtn;
    private CCombo      resourceTypeCmb;
    private Button      cusResTypeRbtn;
    private Text        resTypeTxt;
    private Label       stdRTypeLbl;
    private Label       cusRTypeLbl;

    private Set<String> typesToSearch;

    private String      dummyRType;

    protected FindResourcePage() {
        super("Find Resource");
    }

    @Override
    public void createControl(Composite parent) {
        setPageComplete(false);
        setTitle(Constants.FIND_PAGE_TITLE);
        setMessage(Constants.FIND_PAGE_MESSAGE);

        Composite compContent = new Composite(parent, SWT.NONE);
        GridLayout gridLayout = new GridLayout();
        compContent.setLayout(gridLayout);
        GridData gd = new GridData(SWT.FILL, SWT.FILL, true, true);
        compContent.setLayoutData(gd);

        Group configGroup = new Group(compContent, SWT.NONE);
        gridLayout = new GridLayout(1, false);
        gridLayout.verticalSpacing = 10;
        gridLayout.marginTop = 5;
        configGroup.setLayout(gridLayout);
        gd = new GridData();
        gd.horizontalAlignment = SWT.FILL;
        gd.grabExcessHorizontalSpace = true;
        configGroup.setLayoutData(gd);
        configGroup.setText("Resource Type");

        stdResTypeRbtn = new Button(configGroup, SWT.RADIO);
        stdResTypeRbtn.setText("Standard OIC Resources");

        Composite stdConfigComp = new Composite(configGroup, SWT.NONE);
        stdConfigComp.setLayout(new GridLayout(2, false));
        gd = new GridData();
        gd.horizontalAlignment = SWT.FILL;
        gd.grabExcessHorizontalSpace = true;
        stdConfigComp.setLayoutData(gd);

        stdRTypeLbl = new Label(stdConfigComp, SWT.NONE);
        stdRTypeLbl.setText("ResourceType:");
        stdRTypeLbl.setEnabled(false);

        resourceTypeCmb = new CCombo(stdConfigComp, SWT.READ_ONLY | SWT.BORDER);
        gd = new GridData();
        gd.widthHint = 150;
        resourceTypeCmb.setLayoutData(gd);
        resourceTypeCmb.setEnabled(false);

        cusResTypeRbtn = new Button(configGroup, SWT.RADIO);
        cusResTypeRbtn.setText("Custom Resources");

        Composite cusConfigComp = new Composite(configGroup, SWT.NONE);
        cusConfigComp.setLayout(new GridLayout(2, false));
        gd = new GridData();
        gd.horizontalAlignment = SWT.FILL;
        gd.grabExcessHorizontalSpace = true;
        cusConfigComp.setLayoutData(gd);

        cusRTypeLbl = new Label(cusConfigComp, SWT.NONE);
        cusRTypeLbl.setText("Enter ResourceType:");
        cusRTypeLbl.setEnabled(false);

        resTypeTxt = new Text(cusConfigComp, SWT.BORDER);
        gd = new GridData();
        gd.minimumWidth = 200;
        gd.horizontalAlignment = SWT.FILL;
        gd.grabExcessHorizontalSpace = true;
        resTypeTxt.setLayoutData(gd);
        resTypeTxt.setEnabled(false);

        populateDataInUI();

        addUIListeners();

        setControl(compContent);
    }

    private void populateDataInUI() {
        // Populate Standard resource-types in Combo
        populateResourceTypeCombo();
    }

    private void populateResourceTypeCombo() {
        /*
         * List<String> configList; configList =
         * Activator.getDefault().getManager().getResourceConfigurationList();
         * if(null != configList) { Iterator<String> itr =
         * configList.iterator(); while(itr.hasNext()) {
         * resourceTypeCmb.add(itr.next()); } }
         */

        // TODO: Temporarily adding a resourceType for testing
       // resourceTypeCmb.add("oic.r.light");
        resourceTypeCmb.add("sample.light");

        // By default, selecting the first item in the resourceType combo
        if (resourceTypeCmb.getItemCount() > 0) {
            resourceTypeCmb.select(0);
            // TODO: Get the RAML configuration file path of the selected
            // resource
            // configFilePath =
            // Activator.getManager().getConfigFilePath(resourceTypeCmb.getItem(0));
        }
    }

    private void addUIListeners() {
        stdResTypeRbtn.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                // Clear the existing items from the search list
                if (null != typesToSearch)
                    typesToSearch.clear();

                // Set the configFilePath to the first item in the combo
                if (resourceTypeCmb.getItemCount() > 0) {
                    resourceTypeCmb.select(0);
                    addSearchType(resourceTypeCmb.getText());
                }

                setPageComplete(isSelectionDone());

                // Change the visibility of widgets
                changeVisibility(true);
            }
        });

        cusResTypeRbtn.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                // Clear the existing items from the search list
                if (null != typesToSearch)
                    typesToSearch.clear();

                addSearchType(resTypeTxt.getText());

                setPageComplete(isSelectionDone());

                // Change the visibility of widgets
                changeVisibility(false);

                resTypeTxt.setFocus();
            }
        });

        resourceTypeCmb.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                int index = resourceTypeCmb.getSelectionIndex();
                if (index < 0) {
                    return;
                }
                String resourceType = resourceTypeCmb.getItem(index);
                addSearchType(resourceType);
                setPageComplete(isSelectionDone());
            }
        });

        resTypeTxt.addModifyListener(new ModifyListener() {
            @Override
            public void modifyText(ModifyEvent e) {
                String resourceType = resTypeTxt.getText();
                if (null != dummyRType) {
                    removeSearchType(dummyRType);
                }
                dummyRType = resourceType;
                addSearchType(resourceType);
                setPageComplete(isSelectionDone());
            }
        });
    }

    private void changeVisibility(boolean standard) {
        stdRTypeLbl.setEnabled(standard);
        resourceTypeCmb.setEnabled(standard);
        cusRTypeLbl.setEnabled(!standard);
        resTypeTxt.setEnabled(!standard);
    }

    private boolean isSelectionDone() {
        if (null == typesToSearch || typesToSearch.size() < 1) {
            return false;
        }
        return true;
    }

    private void addSearchType(String resourceType) {
        if (null == resourceType)
            return;
        resourceType = resourceType.trim();
        if (resourceType.length() < 1) {
            return;
        }
        if (null == typesToSearch) {
            typesToSearch = new HashSet<String>();
        }
        typesToSearch.add(resourceType);
    }

    private void removeSearchType(String resourceType) {
        if (null == resourceType || null == typesToSearch)
            return;
        resourceType = resourceType.trim();
        if (resourceType.length() < 1) {
            return;
        }
        typesToSearch.remove(resourceType);
    }

    public Set<String> getSearchTypes() {
        return typesToSearch;
    }
}
