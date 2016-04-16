package org.iotivity.bundle.hue;

import java.util.HashMap;

import org.iotivity.resourcecontainer.bundle.api.BundleResource;

/**
 * This class maps a Philips Hue light to OIC light resource
 * 
 * @author iotivity
 */
public class HueLightResource extends BundleResource {
	private HueConnector m_hueConnector;

	public HueLightResource() {
		initAttributes();
		m_resourceType = "oic.r.light.control";

	}

	public HueLightResource(HueConnector hueConnector, String name, String uri,
			String resourceType, String address) {
		this();
		this.m_hueConnector = hueConnector;
		m_name = name;
		m_uri = uri;
		m_resourceType = resourceType;
		m_address = address;
	}

	protected void initAttributes() {
		m_attributes.put("on-off", "true");
		m_attributes.put("color", "0");
		m_attributes.put("dim", "0");
	}

	public void handleSetAttributeRequest(String key, String value) {
		System.out.println("Set attribute called - key: " + key + ", value: "
				+ value + " transmitting now.");

		if ("on-off".equals(value)) {
			m_hueConnector.transmit(m_address + "/state", "{\"on\":" + value
					+ "}");
		}

		if ("dim".equals(value)) {
			m_hueConnector.transmit(m_address + "/state", "{\"bri\":" + value
					+ "}");
		}

		if ("color".equals(value)) {
			m_hueConnector.transmit(m_address + "/state", "{\"hue\":" + value
					+ "}");
		}
		this.setAttribute(key, value);
	}

	public String handleGetAttributeRequest(String key) {
		// map key to hue address
		// read from Hue gateway, parse resource representation and return
		// attribute
		// m_hueConnector.read(m_address);
		return this.getAttribute(key);
	}

	@Override
	public String toString() {
		return "HueLightResource [m_hueConnector=" + m_hueConnector
				+ ", m_name=" + m_name + ", m_uri=" + m_uri
				+ ", m_resourceType=" + m_resourceType + ", m_address="
				+ m_address + ", m_attributes=" + m_attributes + "]";
	}

}
