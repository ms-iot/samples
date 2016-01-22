//-----------------------------------------------------------------------------
//
//      ControllerCommandDlg.cs
//
//      Executes a controller command and show a dialog window while the 
//      controller command is running.
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
    /// Executes a controller command and show a dialog window while the controller command is running
    /// </summary>
    public partial class ControllerCommandDlg : Form
    {
        private static ZWManager m_manager;
        private static ControllerCommandDlg m_dlg;
        private static UInt32 m_homeId;

        private static ZWControllerCommand m_op;
        private static Byte m_nodeId;
        private static DialogResult result;

        private MainForm m_mainDlg;

        public MainForm MainDlg
        {
            get { return m_mainDlg; }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ControllerCommandDlg"/> class.
        /// </summary>
        /// <param name="_mainDlg">The main form.</param>
        /// <param name="_manager">The manager.</param>
        /// <param name="homeId">The home identifier.</param>
        /// <param name="_op">The Controller Command.</param>
        /// <param name="nodeId">The node identifier.</param>
        public ControllerCommandDlg(MainForm _mainDlg, ZWManager _manager, UInt32 homeId, ZWControllerCommand _op,
            Byte nodeId)
        {
            m_mainDlg = _mainDlg;
            m_manager = _manager;
            m_homeId = homeId;
            m_op = _op;
            m_nodeId = nodeId;
            m_dlg = this;

            InitializeComponent();

            m_manager.OnNotification += new ManagedNotificationsHandler(NotificationHandler);
            switch (m_op)
            {
                case ZWControllerCommand.RequestNodeNeighborUpdate:
                {
                    this.Text = "Node Neighbor Update";
                    this.label1.Text = "Request that a node update its list of neighbors.";

                    if (!m_manager.RequestNodeNeighborUpdate(m_homeId, m_nodeId))
                    {
                        MyControllerStateChangedHandler(ZWControllerState.Failed);
                    }
                    break;
                }
                case ZWControllerCommand.AddDevice:
                {
                    this.Text = "Add Device";
                    this.label1.Text =
                        "Press the program button on the Z-Wave device to add it to the network.\nFor security reasons, the PC Z-Wave Controller must be close to the device being added.";

                    if (!m_manager.AddNode(m_homeId, m_mainDlg.SecurityEnabled))
                    {
                        MyControllerStateChangedHandler(ZWControllerState.Failed);
                    }
                    break;
                }
                case ZWControllerCommand.CreateNewPrimary:
                {
                    this.Text = "Create New Primary Controller";
                    this.label1.Text =
                        "Put the target controller into receive configuration mode.\nThe PC Z-Wave Controller must be within 2m of the controller that is being made the primary.";

                    if (!m_manager.CreateNewPrimary(m_homeId))
                    {
                        MyControllerStateChangedHandler(ZWControllerState.Failed);
                    }
                    break;
                }
                case ZWControllerCommand.ReceiveConfiguration:
                {
                    this.Text = "Receive Configuration";
                    this.label1.Text =
                        "Transfering the network configuration\nfrom another controller.\n\nPlease bring the other controller within 2m of the PC controller and set it to send its network configuration.";

                    if (!m_manager.ReceiveConfiguration(m_homeId))
                    {
                        MyControllerStateChangedHandler(ZWControllerState.Failed);
                    }
                    break;
                }
                case ZWControllerCommand.RemoveDevice:
                {
                    this.Text = "Remove Device";
                    this.label1.Text =
                        "Press the program button on the Z-Wave device to remove it from the network.\nFor security reasons, the PC Z-Wave Controller must be close to the device being removed.";

                    if (!m_manager.RemoveNode(m_homeId))
                    {
                        MyControllerStateChangedHandler(ZWControllerState.Failed);
                    }
                    break;
                }
                case ZWControllerCommand.TransferPrimaryRole:
                {
                    this.Text = "Transfer Primary Role";
                    this.label1.Text =
                        "Transfering the primary role\nto another controller.\n\nPlease bring the new controller within 2m of the PC controller and set it to receive the network configuration.";

                    if (!m_manager.TransferPrimaryRole(m_homeId))
                    {
                        MyControllerStateChangedHandler(ZWControllerState.Failed);
                    }
                    break;
                }
                case ZWControllerCommand.HasNodeFailed:
                {
                    this.ButtonCancel.Enabled = false;
                    this.Text = "Has Node Failed";
                    this.label1.Text = "Testing whether the node has failed.\nThis command cannot be cancelled.";

                    if (!m_manager.HasNodeFailed(m_homeId, m_nodeId))
                    {
                        MyControllerStateChangedHandler(ZWControllerState.Failed);
                    }
                    break;
                }
                case ZWControllerCommand.RemoveFailedNode:
                {
                    this.ButtonCancel.Enabled = false;
                    this.Text = "Remove Failed Node";
                    this.label1.Text =
                        "Removing the failed node from the controller's list.\nThis command cannot be cancelled.";

                    if (!m_manager.RemoveFailedNode(m_homeId, m_nodeId))
                    {
                        MyControllerStateChangedHandler(ZWControllerState.Failed);
                    }
                    break;
                }
                case ZWControllerCommand.ReplaceFailedNode:
                {
                    this.ButtonCancel.Enabled = false;
                    this.Text = "Replacing Failed Node";
                    this.label1.Text = "Testing the failed node.\nThis command cannot be cancelled.";

                    if (!m_manager.ReplaceFailedNode(m_homeId, m_nodeId))
                    {
                        MyControllerStateChangedHandler(ZWControllerState.Failed);
                    }
                    break;
                }
                case ZWControllerCommand.RequestNetworkUpdate:
                {
                    this.ButtonCancel.Enabled = false;
                    this.Text = "Requesting Network Update";
                    this.label1.Text = "Requesting the Network Update.";

                    if (!m_manager.RequestNetworkUpdate(m_homeId, m_nodeId))
                    {
                        MyControllerStateChangedHandler(ZWControllerState.Failed);
                    }
                    break;
                }
                default:
                {
                    m_manager.OnNotification -= NotificationHandler;
                    break;
                }
            }
        }

        /// <summary>
        /// Handles Notifications.
        /// </summary>
        /// <param name="notification">The notification.</param>
        public static void NotificationHandler(ZWNotification notification)
        {
            switch (notification.GetType())
            {
                case ZWNotification.Type.ControllerCommand:
                {
                    MyControllerStateChangedHandler(((ZWControllerState) notification.GetEvent()));
                    break;
                }
            }
        }

        /// <summary>
        /// Handles controller state changes.
        /// </summary>
        /// <param name="state">The state.</param>
        public static void MyControllerStateChangedHandler(ZWControllerState state)
        {
            // Handle the controller state notifications here.
            bool complete = false;
            String dlgText = "";
            bool buttonEnabled = true;

            switch (state)
            {
                case ZWControllerState.Waiting:
                {
                    // Display a message to tell the user to press the include button on the controller
                    if (m_op == ZWControllerCommand.ReplaceFailedNode)
                    {
                        dlgText =
                            "Press the program button on the replacement Z-Wave device to add it to the network.\nFor security reasons, the PC Z-Wave Controller must be close to the device being added.\nThis command cannot be cancelled.";
                    }
                    break;
                }
                case ZWControllerState.InProgress:
                {
                    // Tell the user that the controller has been found and the adding process is in progress.
                    dlgText = "Please wait...";
                    buttonEnabled = false;
                    break;
                }
                case ZWControllerState.Completed:
                {
                    // Tell the user that the controller has been successfully added.
                    // The command is now complete
                    dlgText = "Command Completed OK.";
                    complete = true;
                    result = DialogResult.OK;
                    break;
                }
                case ZWControllerState.Failed:
                {
                    // Tell the user that the controller addition process has failed.
                    // The command is now complete
                    dlgText = "Command Failed.";
                    complete = true;
                    result = DialogResult.Abort;
                    break;
                }
                case ZWControllerState.NodeOK:
                {
                    dlgText = "Node has not failed.";
                    complete = true;
                    result = DialogResult.No;
                    break;
                }
                case ZWControllerState.NodeFailed:
                {
                    dlgText = "Node has failed.";
                    complete = true;
                    result = DialogResult.Yes;
                    break;
                }
                case ZWControllerState.Cancel:
                {
                    dlgText = "Command was cancelled.";
                    complete = true;
                    result = DialogResult.Cancel;
                    break;
                }
                case ZWControllerState.Error:
                {
                    dlgText = "An error occurred while processing the controller command.";
                    complete = true;
                    result = DialogResult.Cancel;
                    break;
                }
            }

            if (dlgText != "")
            {
                m_dlg.SetDialogText(dlgText);
            }

            m_dlg.SetButtonEnabled(buttonEnabled);

            if (complete)
            {
                m_dlg.SetButtonText("OK");

                // Remove the event handler
                m_manager.OnNotification -= NotificationHandler;
            }
        }

        /// <summary>
        /// Sets the dialog text.
        /// </summary>
        /// <param name="text">The text.</param>
        private void SetDialogText(String text)
        {
            if (m_dlg.InvokeRequired)
            {
                Invoke(new MethodInvoker(delegate() { SetDialogText(text); }));
            }
            else
            {
                m_dlg.label1.Text = text;
            }
        }

        /// <summary>
        /// Sets the button text.
        /// </summary>
        /// <param name="text">The text.</param>
        private void SetButtonText(String text)
        {
            if (m_dlg.InvokeRequired)
            {
                Invoke(new MethodInvoker(delegate() { SetButtonText(text); }));
            }
            else
            {
                m_dlg.ButtonCancel.Text = text;
            }
        }

        /// <summary>
        /// Sets the button enabled.
        /// </summary>
        /// <param name="enabled">if set to <c>true</c> [enabled].</param>
        private void SetButtonEnabled(bool enabled)
        {
            if (m_dlg.InvokeRequired)
            {
                Invoke(new MethodInvoker(delegate() { SetButtonEnabled(enabled); }));
            }
            else
            {
                m_dlg.ButtonCancel.Enabled = enabled;
            }
        }

        /// <summary>
        /// Handles the Click event of the ButtonCancel control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void ButtonCancel_Click(object sender, EventArgs e)
        {
            if (ButtonCancel.Text != "OK")
            {
                // Remove the event handler
                m_manager.OnNotification -= NotificationHandler;

                // Cancel the operation
                m_manager.CancelControllerCommand(m_homeId);
            }

            // Close the dialog
            Close();
            m_dlg.DialogResult = result;
        }
    }
}