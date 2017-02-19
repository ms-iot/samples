package org.iotivity.service;

/**
 * Thrown when an operation that has base-layer dependency is failed.
 *
 */
public class RcsPlatformException extends RcsException {

    private static final long serialVersionUID = -6093438347973754721L;

    private final int mReasonCode;

    public RcsPlatformException(String message, int reasonCode) {
        super(message);

        mReasonCode = reasonCode;
    }

    public int getReasonCode() {
        return mReasonCode;
    }
}
