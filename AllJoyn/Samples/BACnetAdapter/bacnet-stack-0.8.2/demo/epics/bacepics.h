/**************************************************************************
*
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/

#ifndef BACEPICS_H_
#define BACEPICS_H_

/** @file epics/bacepics.h  Header for tool to generate EPICS-usable output. */


/** @defgroup BACEPICS Tool to generate EPICS-usable output.
 * @ingroup Demos
 * This tool will generate a list of Objects and their properties for use in
 * an EPICS file.
 * You will still need to provide the front part of the EPICS (see file
 * demo/server/epics_vts3.tpi) which cannot be easily determined by observation,
 * but this tool communicates with the test device and does the grunt work of
 * creating the list of Objects and the supported properties for
 * each of those Objects.
 *
 * Usage:
 *  bacepics [-v] [-p sport] [-t target_mac] device-instance
 *    -v: show values instead of '?'
 *    -p: Use sport for "my" port, instead of 0xBAC0 (BACnet/IP only)
 *        Allows you to communicate with a localhost target.
 *    -t: declare target's MAC instead of using Who-Is to bind to
 *        device-instance. Format is "C0:A8:00:18:BA:C0" (as usual)
 * 
 * Examples:
 * 	./bacepics -v 1234
 *    where the device instance to be addressed is 1234
 *    and the optional -v prints values out rather than the '?' that
 *    the EPICS format for VTS3 wants.
 * 	./bacepics -p 0xBAC1 -t "7F:0:0:1:BA:C0" 4194303
 *    communicates with the BACnet device on localhost (127.0.0.1), using
 *    port 47809 as "my" source port so it doesn't conflict with
 *    the device's port 47808.
 *
 *
 * The tool follows an optimal approach which will use efficient communication
 * means if available or else fall back to simple-minded methods.
 * Starting with the Device Object, the tool will
 * - Try to fetch ALL the Properties with RPM
 *   - If RPM is not supported, will use coded properties in demo/object folder
 *   - If response is too big to fit (without segmentation), then will fetch
 *     ALL again with array index of 0, which should result mostly in errors
 *     but will provide the list of supported properties.
 *     - If that succeeds, build the list of properties to be accessed. <br>
 *   - If no RPM or failed to get ALL properties from the target device, then 
 *     fetch the coded Required and Optional properties from the demo/object 
 *     folder for this object type, and use this to build the list of 
 *     properties to be accessed.
 * - If the Fetch All succeeded, print the values for each property
 * - Otherwise, for each property in the list for this object,
 * 	 - Request the single property value with ReadProperty (RP)
 *   - From the response, print the property's value
 * The Device Object will have fetched the Object List property and built a list
 * of objects from that; use it now to cycle through each other Object and
 * repeat the above process to get and print out their property values.
 */

/** The allowed States of the bacepics State Machine.
 * Important to distinguish the request from the response phases as well
 * as which approach will get all the properties for us.
 * @ingroup BACEPICS
 */
typedef enum {
        /** Initial state to establish a binding with the target device. */
    INITIAL_BINDING,
        /** Get selected device information and put out the heading information. */
    GET_HEADING_INFO, GET_HEADING_RESPONSE, PRINT_HEADING,
        /** Getting ALL properties and values at once with RPM. */
    GET_ALL_REQUEST, GET_ALL_RESPONSE,
        /** Getting ALL properties with array index = 0, just to get the list. */
    GET_LIST_OF_ALL_REQUEST, GET_LIST_OF_ALL_RESPONSE,
        /** Processing the properties individually with ReadProperty. */
    GET_PROPERTY_REQUEST, GET_PROPERTY_RESPONSE,
        /** Done with this Object; move onto the next. */
    NEXT_OBJECT
} EPICS_STATES;


#endif /* BACEPICS_H_ */
