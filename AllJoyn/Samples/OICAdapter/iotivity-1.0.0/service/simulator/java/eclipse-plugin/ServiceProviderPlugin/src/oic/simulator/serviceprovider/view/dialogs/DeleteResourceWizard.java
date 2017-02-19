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

import java.net.URL;

import oic.simulator.serviceprovider.Activator;
import oic.simulator.serviceprovider.resource.DeleteCategory;

import org.eclipse.core.runtime.FileLocator;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.PlatformUI;

/**
 * This class creates a UI wizard for delete resource operation.
 */
public class DeleteResourceWizard extends Wizard {

    private DeleteResourcePage page;

    public DeleteResourceWizard() {
        setWindowTitle("Delete resources");
        IPath path = new Path("/icons/oic_logo_64x64.png");
        URL find = FileLocator.find(Activator.getDefault().getBundle(), path,
                null);
        setDefaultPageImageDescriptor(ImageDescriptor.createFromURL(find));
    }

    @Override
    public void addPages() {
        page = new DeleteResourcePage();
        addPage(page);
    }

    @Override
    public boolean performFinish() {
        if (null == page) {
            return false;
        }
        // Check the existence of the resource if the user has entered the uri
        if (page.getDeleteCategory() == DeleteCategory.BY_URI) {
            // Check whether the uri is in full form or short form
            // If it is in short form, expand it to its full form.
            String uri = page.getDeleteCandidate();
            boolean dispName = Activator.getDefault().getResourceManager()
                    .isDisplayName(uri);
            if (dispName) {
                uri = Activator.getDefault().getResourceManager()
                        .getCompleteUriFromDisplayName(uri);
            }
            boolean exist = Activator.getDefault().getResourceManager()
                    .isResourceExist(uri);
            if (!exist) {
                Shell activeShell = PlatformUI.getWorkbench().getDisplay()
                        .getActiveShell();
                MessageDialog dialog = new MessageDialog(activeShell,
                        "Resource Not Found", null,
                        "No resource exist with the given URI.",
                        MessageDialog.INFORMATION, new String[] { "OK" }, 0);
                dialog.open();
                page.setFocusToTextBox();
                return false;
            }
        }
        return true;
    }

    public DeleteCategory getDeleteCategory() {
        if (null == page) {
            return DeleteCategory.NONE;
        }
        return page.getDeleteCategory();
    }

    public String getDeleteCandidate() {
        if (null == page) {
            return null;
        }
        return page.getDeleteCandidate();
    }
}