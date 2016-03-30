package org.iotivity.service.server;

import org.iotivity.service.RcsException;

/**
 * Thrown when trying to access a unlocked {@link RcsLockedAttributes}.
 *
 */
public class RcsUnlockedException extends RcsException {

    private static final long serialVersionUID = 4292243643497860992L;

    public RcsUnlockedException(String message) {
        super(message);
    }

}
