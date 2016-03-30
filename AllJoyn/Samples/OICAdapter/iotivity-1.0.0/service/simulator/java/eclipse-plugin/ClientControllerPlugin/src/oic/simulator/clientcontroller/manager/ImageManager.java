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

package oic.simulator.clientcontroller.manager;

import java.net.URL;

import oic.simulator.clientcontroller.Activator;
import oic.simulator.clientcontroller.utils.Constants;

import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.resource.ImageRegistry;
import org.eclipse.swt.graphics.Image;
import org.osgi.framework.Bundle;

/**
 * Class which loads the icons/images into the image registry, and has methods
 * to handle the image related requests from other UI modules.
 */
public class ImageManager {

    private static ImageManager imageManager;

    public static ImageManager getInstance() {
        if (null == imageManager) {
            imageManager = new ImageManager();
        }
        return imageManager;
    }

    static {
        ImageRegistry r = Activator.getDefault().getImageRegistry();
        Bundle bundle = Activator.getDefault().getBundle();

        r.put(Constants.CHECKED, ImageDescriptor.createFromURL(bundle
                .getEntry("icons/checked.gif")));
        r.put(Constants.UNCHECKED, ImageDescriptor.createFromURL(bundle
                .getEntry("icons/unchecked.gif")));

        // Resource icons based on the resource type
        r.put(Constants.OIC_R_LIGHT, ImageDescriptor.createFromURL(bundle
                .getEntry("/icons/light_16x16.png")));

        // Log View related icons
        r.put(Constants.DEBUG_LOG, ImageDescriptor.createFromURL(bundle
                .getEntry("/icons/debug_log.gif")));
        r.put(Constants.INFO_LOG, ImageDescriptor.createFromURL(bundle
                .getEntry("/icons/info_log.gif")));
        r.put(Constants.WARNING_LOG, ImageDescriptor.createFromURL(bundle
                .getEntry("/icons/warning_log.gif")));
        r.put(Constants.ERROR_LOG, ImageDescriptor.createFromURL(bundle
                .getEntry("/icons/error_log.gif")));
        r.put(Constants.UNKNOWN_LOG, ImageDescriptor.createFromURL(bundle
                .getEntry("/icons/unknown_log.gif")));
    }

    public static Image getImage(String imagePath) {
        if (null == imagePath || imagePath.length() < 1) {
            return null;
        }
        URL imageURL = Activator.getDefault().getBundle().getEntry(imagePath);
        ImageDescriptor descriptor = ImageDescriptor.createFromURL(imageURL);
        return descriptor.createImage();
    }
}
