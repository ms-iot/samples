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

import oic.simulator.serviceprovider.Activator;
import oic.simulator.serviceprovider.resource.DeleteCategory;
import oic.simulator.serviceprovider.utils.Constants;

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
import org.eclipse.swt.widgets.Text;

/**
 * This class shows UI for deleting resources.
 */
public class DeleteResourcePage extends WizardPage {

    private Button         allRbtn;
    private Button         byTypeRbtn;
    private Button         byUriRbtn;

    private CCombo         resourceTypeCmb;
    private Text           resourceUriTxt;

    private DeleteCategory deleteCategory;

    // It will hold either the resource type or resource uri
    private String         deleteCandidate;

    protected DeleteResourcePage() {
        super("Delete Resource");
    }

    @Override
    public void createControl(Composite parent) {
        setPageComplete(false);
        setTitle(Constants.DELETE_PAGE_TITLE);
        setMessage(Constants.DELETE_PAGE_MESSAGE);

        Composite compContent = new Composite(parent, SWT.NONE);
        compContent.setLayout(new GridLayout(1, false));
        GridData gd = new GridData(SWT.FILL, SWT.FILL, true, true);
        compContent.setLayoutData(gd);

        Group group = new Group(compContent, SWT.NONE);
        group.setText("Select Category");
        GridLayout gridLayout = new GridLayout(2, false);
        gridLayout.verticalSpacing = 15;
        gridLayout.marginTop = 10;
        gridLayout.marginLeft = 10;
        group.setLayout(gridLayout);
        gd = new GridData(SWT.FILL, SWT.FILL, true, true);
        group.setLayoutData(gd);

        allRbtn = new Button(group, SWT.RADIO);
        allRbtn.setText("All resources");
        gd = new GridData();
        gd.horizontalSpan = 2;
        gd.widthHint = 200;
        allRbtn.setLayoutData(gd);

        byTypeRbtn = new Button(group, SWT.RADIO);
        byTypeRbtn.setText("All (By resource type)");
        gd = new GridData();
        gd.widthHint = 200;
        byTypeRbtn.setLayoutData(gd);

        resourceTypeCmb = new CCombo(group, SWT.READ_ONLY | SWT.BORDER);
        gd = new GridData();
        gd.widthHint = 200;
        resourceTypeCmb.setLayoutData(gd);

        byUriRbtn = new Button(group, SWT.RADIO);
        byUriRbtn.setText("By Resource URI");
        gd = new GridData();
        gd.widthHint = 200;
        byUriRbtn.setLayoutData(gd);

        resourceUriTxt = new Text(group, SWT.BORDER);
        gd = new GridData();
        gd.widthHint = 300;
        resourceUriTxt.setLayoutData(gd);

        // Setting the initial visibility of controls
        allRbtn.setSelection(false);
        byTypeRbtn.setSelection(false);
        byUriRbtn.setSelection(false);

        resourceTypeCmb.setEnabled(false);
        resourceUriTxt.setEnabled(false);

        deleteCategory = DeleteCategory.NONE;

        populateDataInUI();

        addUIListeners();

        setControl(compContent);
    }

    private void populateDataInUI() {
        // Populate Resourcetype in Combo
        populateResourceTypeCombo();
    }

    private void populateResourceTypeCombo() {
        List<String> resourceTypeList;
        resourceTypeList = Activator.getDefault().getResourceManager()
                .getResourceTypeList();
        if (null != resourceTypeList) {
            Iterator<String> itr = resourceTypeList.iterator();
            while (itr.hasNext()) {
                resourceTypeCmb.add(itr.next());
            }
        }

        // By default, select the first item in the combo
        if (resourceTypeCmb.getItemCount() > 0) {
            resourceTypeCmb.select(0);
            deleteCandidate = resourceTypeCmb.getText();
        }
    }

    public void addUIListeners() {
        allRbtn.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                // Update the visibility of controls
                resourceTypeCmb.setEnabled(false);
                resourceUriTxt.setEnabled(false);

                deleteCategory = DeleteCategory.ALL;
                deleteCandidate = null;
                setPageComplete(isSelectionDone());
            }
        });

        byTypeRbtn.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                // Update the visibility of controls
                resourceTypeCmb.setEnabled(true);
                resourceUriTxt.setEnabled(false);

                deleteCategory = DeleteCategory.BY_TYPE;
                setPageComplete(isSelectionDone());
            }
        });

        byUriRbtn.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                // Update the visibility of controls
                resourceUriTxt.setEnabled(true);
                resourceUriTxt.setFocus();
                resourceTypeCmb.setEnabled(false);

                deleteCategory = DeleteCategory.BY_URI;
                setPageComplete(isSelectionDone());
            }
        });

        resourceTypeCmb.addModifyListener(new ModifyListener() {
            @Override
            public void modifyText(ModifyEvent arg0) {
                setPageComplete(isSelectionDone());
            }
        });

        resourceUriTxt.addModifyListener(new ModifyListener() {
            @Override
            public void modifyText(ModifyEvent arg0) {
                setPageComplete(isSelectionDone());
            }
        });
    }

    public boolean isSelectionDone() {
        boolean done = false;
        if (deleteCategory == DeleteCategory.ALL) {
            done = true;
        } else if (deleteCategory == DeleteCategory.BY_TYPE) {
            int selectedItemIndex = resourceTypeCmb.getSelectionIndex();
            if (selectedItemIndex >= 0) {
                deleteCandidate = resourceTypeCmb.getItem(selectedItemIndex);
                if (null != deleteCandidate && deleteCandidate.length() > 0) {
                    done = true;
                }
            }
        } else if (deleteCategory == DeleteCategory.BY_URI) {
            deleteCandidate = resourceUriTxt.getText();
            if (null != deleteCandidate && deleteCandidate.length() > 0) {
                done = true;
            }
        }
        return done;
    }

    public DeleteCategory getDeleteCategory() {
        return deleteCategory;
    }

    public String getDeleteCandidate() {
        return deleteCandidate;
    }

    public void setFocusToTextBox() {
        resourceUriTxt.setFocus();
    }
}