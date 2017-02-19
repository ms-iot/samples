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

import org.eclipse.core.runtime.FileLocator;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.wizard.Wizard;

/**
 * This class creates a UI wizard for create resource operation.
 */
public class CreateResourceWizard extends Wizard {

    private CreateResourcePage page;

    public CreateResourceWizard() {
        setWindowTitle("Create resources");
        IPath path = new Path("/icons/oic_logo_64x64.png");
        URL find = FileLocator.find(Activator.getDefault().getBundle(), path,
                null);
        setDefaultPageImageDescriptor(ImageDescriptor.createFromURL(find));
    }

    @Override
    public void addPages() {
        page = new CreateResourcePage();
        addPage(page);
    }

    public String getConfigFilePath() {
        if (null == page) {
            return null;
        }
        return page.getConfigFilePath();
    }

    public int getResourceCount() {
        if (null == page) {
            return 0;
        }
        return page.getResourceCount();
    }

    @Override
    public boolean performFinish() {
        return true;
    }
}