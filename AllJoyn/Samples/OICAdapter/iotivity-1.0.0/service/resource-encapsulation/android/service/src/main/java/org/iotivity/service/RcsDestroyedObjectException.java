package org.iotivity.service;

/**
 * Thrown when trying to access a destroyed resource object.
 *
 */
public class RcsDestroyedObjectException extends RcsException {

    private static final long serialVersionUID = -4107062696447237658L;

    public RcsDestroyedObjectException(String message) {
        super(message);
    }

}
