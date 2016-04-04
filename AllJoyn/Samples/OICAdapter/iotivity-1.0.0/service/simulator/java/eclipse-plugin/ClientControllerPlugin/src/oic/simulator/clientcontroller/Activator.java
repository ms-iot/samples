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

package oic.simulator.clientcontroller;

import oic.simulator.clientcontroller.manager.ImageManager;
import oic.simulator.clientcontroller.manager.LogManager;
import oic.simulator.clientcontroller.manager.ResourceManager;

import org.eclipse.ui.plugin.AbstractUIPlugin;
import org.osgi.framework.BundleContext;

/**
 * The activator class controls the plug-in life cycle.
 */
public class Activator extends AbstractUIPlugin {

    // The plug-in ID
    public static final String     PLUGIN_ID = "ClientControllerPlugin";

    // The shared instance
    private static Activator       plugin;

    private static ResourceManager resourceManager;

    private static LogManager      logManager;

    private static ImageManager    imageManager;

    static {
        System.loadLibrary("SimulatorManager");
    }

    public Activator() {
    }

    public void start(BundleContext context) throws Exception {
        super.start(context);
        plugin = this;
        setResourceManager(new ResourceManager());
        setLogManager(new LogManager());
        imageManager = ImageManager.getInstance();
    }

    public void stop(BundleContext context) throws Exception {
        plugin = null;
        // Stopping Resource Manager
        if (null != resourceManager) {
            resourceManager.shutdown();
            resourceManager = null;
        }
        // Stopping Log Manager
        if (null != logManager) {
            logManager.shutdown();
            logManager = null;
        }
        super.stop(context);
    }

    public static Activator getDefault() {
        return plugin;
    }

    public ResourceManager getResourceManager() {
        return resourceManager;
    }

    private static void setResourceManager(ResourceManager manager) {
        Activator.resourceManager = manager;
    }

    public LogManager getLogManager() {
        return logManager;
    }

    private static void setLogManager(LogManager logManager) {
        Activator.logManager = logManager;
    }

    public ImageManager getImageManager() {
        return imageManager;
    }
}
