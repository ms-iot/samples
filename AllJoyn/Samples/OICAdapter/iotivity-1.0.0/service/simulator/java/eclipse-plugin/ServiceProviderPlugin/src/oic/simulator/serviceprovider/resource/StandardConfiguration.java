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

package oic.simulator.serviceprovider.resource;

import java.io.File;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.Date;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;

import oic.simulator.serviceprovider.Activator;
import oic.simulator.serviceprovider.utils.Constants;

import org.eclipse.core.runtime.FileLocator;
import org.oic.simulator.ILogger.Level;

/**
 * Class which loads and maintains the standard RAML configuration file list.
 */
public class StandardConfiguration {

    // A map of filename of standard resources as the key and the complete
    // location of the file(including the filename) as the value.
    Map<String, String> stdConfigFiles;

    public StandardConfiguration() {
        stdConfigFiles = new HashMap<String, String>();
        populateStandardConfigurationList();
    }

    private void populateStandardConfigurationList() {
        Enumeration<URL> fileList = Activator.getDefault().getBundle()
                .findEntries(Constants.CONFIG_DIRECTORY_PATH, "*", true);
        if (null == fileList) {
            Activator
                    .getDefault()
                    .getLogManager()
                    .log(Level.ERROR.ordinal(), new Date(),
                            "No configuration files exist.");
            return;
        }
        URL url;
        URL resolvedURL;
        URI resolvedURI;
        File file;
        String relPath;
        String absPath;
        while (fileList.hasMoreElements()) {
            url = (URL) fileList.nextElement();
            relPath = url.getPath();
            System.out.println(url.getPath());
            try {
                resolvedURL = FileLocator.toFileURL(url);
                if (relPath.toLowerCase().endsWith(
                        Constants.RAML_FILE_EXTENSION)) {
                    resolvedURI = new URI(resolvedURL.getProtocol(),
                            resolvedURL.getPath(), null);
                    file = new File(resolvedURI);
                    absPath = file.getAbsolutePath();
                    stdConfigFiles.put(relPath, absPath);
                    System.out.println("File path:" + absPath);
                }
            } catch (URISyntaxException | IOException e) {
                Activator.getDefault().getLogManager()
                        .log(Level.ERROR.ordinal(), new Date(), e.getMessage());
            }
        }
    }

    public Map<String, String> getStandardResourceConfigurationList() {
        return stdConfigFiles;
    }

    public void setStandardResourceConfigurationList(
            Map<String, String> stdConfigFiles) {
        this.stdConfigFiles = stdConfigFiles;
    }

    public String getFilePath(String fileName) {
        return stdConfigFiles.get(fileName);
    }
}