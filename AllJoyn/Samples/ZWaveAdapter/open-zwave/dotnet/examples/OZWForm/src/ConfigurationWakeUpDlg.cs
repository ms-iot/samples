//-----------------------------------------------------------------------------
//
//      ConfigurationWakeUpDlg.cs
//
//      <Enter class description>
//
//      Copyright (c) 2010 Mal Lansell <openzwave@lansell.org>
//
//      SOFTWARE NOTICE AND LICENSE
//
//      This file is part of OZWForm.
//
//      OZWForm is free software: you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation, either version 3 of the License, or
//      (at your option) any later version.
//
//      OZWForm is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------------

using System;
using System.Windows.Forms;
using OpenZWaveDotNet;

namespace OZWForm
{
    /// <summary>
    /// Requests all Configuration parameters and shows a dialog window while the request is running
    /// </summary>
	public partial class ConfigurationWakeUpDlg : Form
	{
        static private ZWManager m_manager;
        static private UInt32 m_homeId;
        private ZWNotification m_notification = null;
        static private Byte m_nodeId;

        /// <summary>
        /// Initializes a new instance of the <see cref="ConfigurationWakeUpDlg"/> class.
        /// </summary>
        /// <param name="_manager">The _manager.</param>
        /// <param name="homeId">The home identifier.</param>
        /// <param name="nodeId">The node identifier.</param>
        public ConfigurationWakeUpDlg( ZWManager _manager, UInt32 homeId, Byte nodeId)
		{
            m_manager = _manager;
            m_homeId = homeId;
            m_nodeId = nodeId;

            InitializeComponent();

            // Set the text according to whether the device is listening
            if( m_manager.IsNodeListeningDevice( homeId, nodeId ) )
            {
                label1.Text = "Waiting for configurable parameter info from device...";
            }
            else
            {
                label1.Text = "Waiting for configurable parameter info from device.\r\nPlease ensure device is awake...";
            }
		}

        /// <summary>
        /// The notification handler.
        /// </summary>
        /// <param name="notification">The notification.</param>
        public void NotificationHandler(ZWNotification notification)
        {
            // Handle the notification on a thread that can safely
            // modify the form controls without throwing an exception.
            m_notification = notification;
            Invoke(new MethodInvoker(NotificationHandler));
            m_notification = null;
        }

        /// <summary>
        /// The notification handler.
        /// </summary>
        private void NotificationHandler()
        {
            // Check whether all the queries on this node have completed
            if( m_notification.GetType() == ZWNotification.Type.NodeQueriesComplete )
            {
                if ((m_notification.GetHomeId() == m_homeId) && (m_notification.GetNodeId() == m_nodeId))
                {
                    // Done!
					m_manager.OnNotification -= new ManagedNotificationsHandler(NotificationHandler);
					DialogResult = DialogResult.OK;
				}
            }
        }

        /// <summary>
        /// Handles the FormClosing event of the ConfigurationWakeUpDlg control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="FormClosingEventArgs"/> instance containing the event data.</param>
        private void ConfigurationWakeUpDlg_FormClosing(object sender, FormClosingEventArgs e)
        {
        }

        /// <summary>
        /// Handles the Shown event of the ConfigurationWakeUpDlg control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
		private void ConfigurationWakeUpDlg_Shown(object sender, EventArgs e)
		{
			// Add a handler so that we receive notification of when the node queries are complete.
			m_manager.OnNotification += new ManagedNotificationsHandler(NotificationHandler);

			// Request refreshed config param values.
			m_manager.RequestAllConfigParams(m_homeId, m_nodeId);
		}
 	}
}