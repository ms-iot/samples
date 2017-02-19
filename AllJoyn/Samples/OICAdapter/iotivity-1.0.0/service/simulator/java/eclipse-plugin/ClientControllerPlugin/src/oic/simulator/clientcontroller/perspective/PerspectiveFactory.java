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

package oic.simulator.clientcontroller.perspective;

import oic.simulator.clientcontroller.view.AttributeView;
import oic.simulator.clientcontroller.view.LogView;
import oic.simulator.clientcontroller.view.MetaPropertiesView;
import oic.simulator.clientcontroller.view.MultiResourceOrchestrationView;
import oic.simulator.clientcontroller.view.ResourceManagerView;

import org.eclipse.ui.IPageLayout;
import org.eclipse.ui.IPerspectiveFactory;

/**
 * This class creates a new eclipse perspective for client controller and
 * positions the different views inside.
 */
public class PerspectiveFactory implements IPerspectiveFactory {

    public static final String PERSPECTIVE_ID = "oic.simulator.clientcontroller.perspective";
    private IPageLayout        factory;

    @Override
    public void createInitialLayout(IPageLayout factory) {
        this.factory = factory;
        factory.setEditorAreaVisible(false);
        addViews();
        factory.setFixed(false);
    }

    private void addViews() {
        factory.addView(ResourceManagerView.VIEW_ID, IPageLayout.LEFT, 0.3f,
                factory.getEditorArea());
        factory.addView(MetaPropertiesView.VIEW_ID, IPageLayout.BOTTOM, 0.65f,
                ResourceManagerView.VIEW_ID);
        factory.addView(AttributeView.VIEW_ID, IPageLayout.LEFT, 0.7f,
                factory.getEditorArea());
        factory.addView(LogView.VIEW_ID, IPageLayout.BOTTOM, 0.65f,
                AttributeView.VIEW_ID);
        factory.addView(MultiResourceOrchestrationView.VIEW_ID,
                IPageLayout.RIGHT, 0.6f, AttributeView.VIEW_ID);
    }
}
