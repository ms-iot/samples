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

import java.net.URL;
import java.util.Set;

import oic.simulator.clientcontroller.Activator;

import org.eclipse.core.runtime.FileLocator;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.wizard.Wizard;

/**
 * This class creates a UI wizard for find resource operation.
 */
public class FindResourceWizard extends Wizard {
    private FindResourcePage page;

    public FindResourceWizard() {
        setWindowTitle("Find resources");
        IPath path = new Path("/icons/oic_logo_64x64.png");
        URL find = FileLocator.find(Activator.getDefault().getBundle(), path,
                null);
        setDefaultPageImageDescriptor(ImageDescriptor.createFromURL(find));
    }

    public Set<String> getSearchTypes() {
        if (null == page) {
            return null;
        }
        return page.getSearchTypes();
    }

    @Override
    public void addPages() {
        page = new FindResourcePage();
        addPage(page);
    }

    @Override
    public boolean performFinish() {
        return true;
    }
}
