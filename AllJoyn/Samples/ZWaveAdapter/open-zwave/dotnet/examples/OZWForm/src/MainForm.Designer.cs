namespace OZWForm
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.NodeGridView = new System.Windows.Forms.DataGridView();
            this.NodeContextMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.PowerOnToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.PowerOffToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator4 = new System.Windows.Forms.ToolStripSeparator();
            this.requestNodeNeighborUpdateToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.assignReturnRouteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.deleteReturnRouteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator5 = new System.Windows.Forms.ToolStripSeparator();
            this.hasNodeFailedToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.markNodeAsFailedToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.replaceFailedNodeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator6 = new System.Windows.Forms.ToolStripSeparator();
            this.propertiesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.MenuBar = new System.Windows.Forms.MenuStrip();
            this.FileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.SaveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.controllerToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createNewPrmaryControllerToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.transferPrimaryRoleToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.addControllerToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.addDeviceToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.removeControllerToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.removeDeviceToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.receiveConfigurationToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.requestNetworkUpdateToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator7 = new System.Windows.Forms.ToolStripSeparator();
            this.resetControllersoftToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.softToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.eraseAllToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
            this.addSecureDeviceToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            ((System.ComponentModel.ISupportInitialize)(this.NodeGridView)).BeginInit();
            this.NodeContextMenuStrip.SuspendLayout();
            this.MenuBar.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // NodeGridView
            // 
            this.NodeGridView.AllowUserToAddRows = false;
            this.NodeGridView.AllowUserToDeleteRows = false;
            this.NodeGridView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.NodeGridView.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.NodeGridView.ContextMenuStrip = this.NodeContextMenuStrip;
            this.NodeGridView.Location = new System.Drawing.Point(13, 37);
            this.NodeGridView.MultiSelect = false;
            this.NodeGridView.Name = "NodeGridView";
            this.NodeGridView.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.CellSelect;
            this.NodeGridView.Size = new System.Drawing.Size(609, 381);
            this.NodeGridView.TabIndex = 0;
            this.NodeGridView.CellMouseDown += new System.Windows.Forms.DataGridViewCellMouseEventHandler(this.NodeGridView_CellMouseDown);
            this.NodeGridView.CellParsing += new System.Windows.Forms.DataGridViewCellParsingEventHandler(this.NodeGridView_CellParsing);
            // 
            // NodeContextMenuStrip
            // 
            this.NodeContextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.PowerOnToolStripMenuItem,
            this.PowerOffToolStripMenuItem,
            this.toolStripSeparator4,
            this.requestNodeNeighborUpdateToolStripMenuItem,
            this.assignReturnRouteToolStripMenuItem,
            this.deleteReturnRouteToolStripMenuItem,
            this.toolStripSeparator5,
            this.hasNodeFailedToolStripMenuItem,
            this.markNodeAsFailedToolStripMenuItem,
            this.replaceFailedNodeToolStripMenuItem,
            this.toolStripSeparator6,
            this.propertiesToolStripMenuItem});
            this.NodeContextMenuStrip.Name = "NodeContextMenuStrip";
            this.NodeContextMenuStrip.Size = new System.Drawing.Size(243, 220);
            // 
            // PowerOnToolStripMenuItem
            // 
            this.PowerOnToolStripMenuItem.Name = "PowerOnToolStripMenuItem";
            this.PowerOnToolStripMenuItem.Size = new System.Drawing.Size(242, 22);
            this.PowerOnToolStripMenuItem.Text = "Power On";
            this.PowerOnToolStripMenuItem.Click += new System.EventHandler(this.PowerOnToolStripMenuItem_Click);
            // 
            // PowerOffToolStripMenuItem
            // 
            this.PowerOffToolStripMenuItem.Name = "PowerOffToolStripMenuItem";
            this.PowerOffToolStripMenuItem.Size = new System.Drawing.Size(242, 22);
            this.PowerOffToolStripMenuItem.Text = "Power Off";
            this.PowerOffToolStripMenuItem.Click += new System.EventHandler(this.PowerOffToolStripMenuItem_Click);
            // 
            // toolStripSeparator4
            // 
            this.toolStripSeparator4.Name = "toolStripSeparator4";
            this.toolStripSeparator4.Size = new System.Drawing.Size(239, 6);
            // 
            // requestNodeNeighborUpdateToolStripMenuItem
            // 
            this.requestNodeNeighborUpdateToolStripMenuItem.Name = "requestNodeNeighborUpdateToolStripMenuItem";
            this.requestNodeNeighborUpdateToolStripMenuItem.Size = new System.Drawing.Size(242, 22);
            this.requestNodeNeighborUpdateToolStripMenuItem.Text = "Request Node Neighbor Update";
            this.requestNodeNeighborUpdateToolStripMenuItem.Click += new System.EventHandler(this.requestNodeNeighborUpdateToolStripMenuItem_Click);
            // 
            // assignReturnRouteToolStripMenuItem
            // 
            this.assignReturnRouteToolStripMenuItem.Name = "assignReturnRouteToolStripMenuItem";
            this.assignReturnRouteToolStripMenuItem.Size = new System.Drawing.Size(242, 22);
            this.assignReturnRouteToolStripMenuItem.Text = "Assign Return Route";
            this.assignReturnRouteToolStripMenuItem.Click += new System.EventHandler(this.assignReturnRouteToolStripMenuItem_Click);
            // 
            // deleteReturnRouteToolStripMenuItem
            // 
            this.deleteReturnRouteToolStripMenuItem.Name = "deleteReturnRouteToolStripMenuItem";
            this.deleteReturnRouteToolStripMenuItem.Size = new System.Drawing.Size(242, 22);
            this.deleteReturnRouteToolStripMenuItem.Text = "Delete All Return Routes";
            this.deleteReturnRouteToolStripMenuItem.Click += new System.EventHandler(this.deleteReturnRouteToolStripMenuItem_Click);
            // 
            // toolStripSeparator5
            // 
            this.toolStripSeparator5.Name = "toolStripSeparator5";
            this.toolStripSeparator5.Size = new System.Drawing.Size(239, 6);
            // 
            // hasNodeFailedToolStripMenuItem
            // 
            this.hasNodeFailedToolStripMenuItem.Name = "hasNodeFailedToolStripMenuItem";
            this.hasNodeFailedToolStripMenuItem.Size = new System.Drawing.Size(242, 22);
            this.hasNodeFailedToolStripMenuItem.Text = "Has Node Failed";
            this.hasNodeFailedToolStripMenuItem.Click += new System.EventHandler(this.hasNodeFailedToolStripMenuItem_Click);
            // 
            // markNodeAsFailedToolStripMenuItem
            // 
            this.markNodeAsFailedToolStripMenuItem.Name = "markNodeAsFailedToolStripMenuItem";
            this.markNodeAsFailedToolStripMenuItem.Size = new System.Drawing.Size(242, 22);
            this.markNodeAsFailedToolStripMenuItem.Text = "Remove Failed Node";
            this.markNodeAsFailedToolStripMenuItem.Click += new System.EventHandler(this.markNodeAsFailedToolStripMenuItem_Click);
            // 
            // replaceFailedNodeToolStripMenuItem
            // 
            this.replaceFailedNodeToolStripMenuItem.Name = "replaceFailedNodeToolStripMenuItem";
            this.replaceFailedNodeToolStripMenuItem.Size = new System.Drawing.Size(242, 22);
            this.replaceFailedNodeToolStripMenuItem.Text = "Replace Failed Node";
            this.replaceFailedNodeToolStripMenuItem.Click += new System.EventHandler(this.replaceFailedNodeToolStripMenuItem_Click);
            // 
            // toolStripSeparator6
            // 
            this.toolStripSeparator6.Name = "toolStripSeparator6";
            this.toolStripSeparator6.Size = new System.Drawing.Size(239, 6);
            // 
            // propertiesToolStripMenuItem
            // 
            this.propertiesToolStripMenuItem.Name = "propertiesToolStripMenuItem";
            this.propertiesToolStripMenuItem.Size = new System.Drawing.Size(242, 22);
            this.propertiesToolStripMenuItem.Text = "Properties";
            this.propertiesToolStripMenuItem.Click += new System.EventHandler(this.propertiesToolStripMenuItem_Click);
            // 
            // MenuBar
            // 
            this.MenuBar.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.FileToolStripMenuItem,
            this.controllerToolStripMenuItem});
            this.MenuBar.Location = new System.Drawing.Point(0, 0);
            this.MenuBar.Name = "MenuBar";
            this.MenuBar.Size = new System.Drawing.Size(634, 24);
            this.MenuBar.TabIndex = 1;
            this.MenuBar.Text = "menuStrip1";
            // 
            // FileToolStripMenuItem
            // 
            this.FileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.SaveToolStripMenuItem});
            this.FileToolStripMenuItem.Name = "FileToolStripMenuItem";
            this.FileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.FileToolStripMenuItem.Text = "&File";
            // 
            // SaveToolStripMenuItem
            // 
            this.SaveToolStripMenuItem.Name = "SaveToolStripMenuItem";
            this.SaveToolStripMenuItem.Size = new System.Drawing.Size(98, 22);
            this.SaveToolStripMenuItem.Text = "&Save";
            this.SaveToolStripMenuItem.Click += new System.EventHandler(this.SaveToolStripMenuItem_Click);
            // 
            // controllerToolStripMenuItem
            // 
            this.controllerToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.createNewPrmaryControllerToolStripMenuItem,
            this.transferPrimaryRoleToolStripMenuItem,
            this.addControllerToolStripMenuItem,
            this.addDeviceToolStripMenuItem,
            this.addSecureDeviceToolStripMenuItem,
            this.toolStripSeparator1,
            this.removeControllerToolStripMenuItem,
            this.removeDeviceToolStripMenuItem,
            this.toolStripSeparator2,
            this.receiveConfigurationToolStripMenuItem,
            this.toolStripSeparator3,
            this.requestNetworkUpdateToolStripMenuItem,
            this.toolStripSeparator7,
            this.resetControllersoftToolStripMenuItem});
            this.controllerToolStripMenuItem.Name = "controllerToolStripMenuItem";
            this.controllerToolStripMenuItem.Size = new System.Drawing.Size(72, 20);
            this.controllerToolStripMenuItem.Text = "Controller";
            // 
            // createNewPrmaryControllerToolStripMenuItem
            // 
            this.createNewPrmaryControllerToolStripMenuItem.Name = "createNewPrmaryControllerToolStripMenuItem";
            this.createNewPrmaryControllerToolStripMenuItem.Size = new System.Drawing.Size(232, 22);
            this.createNewPrmaryControllerToolStripMenuItem.Text = "Create New Prmary Controller";
            this.createNewPrmaryControllerToolStripMenuItem.Click += new System.EventHandler(this.createNewPrmaryControllerToolStripMenuItem_Click);
            // 
            // transferPrimaryRoleToolStripMenuItem
            // 
            this.transferPrimaryRoleToolStripMenuItem.Name = "transferPrimaryRoleToolStripMenuItem";
            this.transferPrimaryRoleToolStripMenuItem.Size = new System.Drawing.Size(232, 22);
            this.transferPrimaryRoleToolStripMenuItem.Text = "Transfer Primary Role";
            this.transferPrimaryRoleToolStripMenuItem.Click += new System.EventHandler(this.transferPrimaryRoleToolStripMenuItem_Click);
            // 
            // addControllerToolStripMenuItem
            // 
            this.addControllerToolStripMenuItem.Name = "addControllerToolStripMenuItem";
            this.addControllerToolStripMenuItem.Size = new System.Drawing.Size(232, 22);
            // 
            // addDeviceToolStripMenuItem
            // 
            this.addDeviceToolStripMenuItem.Name = "addDeviceToolStripMenuItem";
            this.addDeviceToolStripMenuItem.Size = new System.Drawing.Size(232, 22);
            this.addDeviceToolStripMenuItem.Text = "Add Device";
            this.addDeviceToolStripMenuItem.Click += new System.EventHandler(this.addDeviceToolStripMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(229, 6);
            // 
            // removeControllerToolStripMenuItem
            // 
            this.removeControllerToolStripMenuItem.Name = "removeControllerToolStripMenuItem";
            this.removeControllerToolStripMenuItem.Size = new System.Drawing.Size(232, 22);
            // 
            // removeDeviceToolStripMenuItem
            // 
            this.removeDeviceToolStripMenuItem.Name = "removeDeviceToolStripMenuItem";
            this.removeDeviceToolStripMenuItem.Size = new System.Drawing.Size(232, 22);
            this.removeDeviceToolStripMenuItem.Text = "Remove Device";
            this.removeDeviceToolStripMenuItem.Click += new System.EventHandler(this.removeDeviceToolStripMenuItem_Click);
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(229, 6);
            // 
            // receiveConfigurationToolStripMenuItem
            // 
            this.receiveConfigurationToolStripMenuItem.Name = "receiveConfigurationToolStripMenuItem";
            this.receiveConfigurationToolStripMenuItem.Size = new System.Drawing.Size(232, 22);
            this.receiveConfigurationToolStripMenuItem.Text = "Receive Configuration";
            this.receiveConfigurationToolStripMenuItem.Click += new System.EventHandler(this.receiveConfigurationToolStripMenuItem_Click);
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            this.toolStripSeparator3.Size = new System.Drawing.Size(229, 6);
            // 
            // requestNetworkUpdateToolStripMenuItem
            // 
            this.requestNetworkUpdateToolStripMenuItem.Name = "requestNetworkUpdateToolStripMenuItem";
            this.requestNetworkUpdateToolStripMenuItem.Size = new System.Drawing.Size(232, 22);
            this.requestNetworkUpdateToolStripMenuItem.Text = "Request Network Update";
            this.requestNetworkUpdateToolStripMenuItem.Click += new System.EventHandler(this.requestNetworkUpdateToolStripMenuItem_Click);
            // 
            // toolStripSeparator7
            // 
            this.toolStripSeparator7.Name = "toolStripSeparator7";
            this.toolStripSeparator7.Size = new System.Drawing.Size(229, 6);
            // 
            // resetControllersoftToolStripMenuItem
            // 
            this.resetControllersoftToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.softToolStripMenuItem,
            this.eraseAllToolStripMenuItem});
            this.resetControllersoftToolStripMenuItem.Name = "resetControllersoftToolStripMenuItem";
            this.resetControllersoftToolStripMenuItem.Size = new System.Drawing.Size(232, 22);
            this.resetControllersoftToolStripMenuItem.Text = "Reset Controller";
            // 
            // softToolStripMenuItem
            // 
            this.softToolStripMenuItem.Name = "softToolStripMenuItem";
            this.softToolStripMenuItem.Size = new System.Drawing.Size(118, 22);
            this.softToolStripMenuItem.Text = "Soft";
            this.softToolStripMenuItem.Click += new System.EventHandler(this.softToolStripMenuItem_Click);
            // 
            // eraseAllToolStripMenuItem
            // 
            this.eraseAllToolStripMenuItem.Name = "eraseAllToolStripMenuItem";
            this.eraseAllToolStripMenuItem.Size = new System.Drawing.Size(118, 22);
            this.eraseAllToolStripMenuItem.Text = "Erase All";
            this.eraseAllToolStripMenuItem.Click += new System.EventHandler(this.eraseAllToolStripMenuItem_Click);
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1});
            this.statusStrip1.Location = new System.Drawing.Point(0, 430);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(634, 22);
            this.statusStrip1.TabIndex = 2;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // toolStripStatusLabel1
            // 
            this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            this.toolStripStatusLabel1.Size = new System.Drawing.Size(70, 17);
            this.toolStripStatusLabel1.Text = "Initializing...";
            // 
            // addSecureDeviceToolStripMenuItem
            // 
            this.addSecureDeviceToolStripMenuItem.Name = "addSecureDeviceToolStripMenuItem";
            this.addSecureDeviceToolStripMenuItem.Size = new System.Drawing.Size(232, 22);
            this.addSecureDeviceToolStripMenuItem.Text = "Add Secure Device";
            this.addSecureDeviceToolStripMenuItem.Click += new System.EventHandler(this.addSecureDeviceToolStripMenuItem_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(634, 452);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.NodeGridView);
            this.Controls.Add(this.MenuBar);
            this.MainMenuStrip = this.MenuBar;
            this.Name = "MainForm";
            this.Text = "OpenZWave Test";
            ((System.ComponentModel.ISupportInitialize)(this.NodeGridView)).EndInit();
            this.NodeContextMenuStrip.ResumeLayout(false);
            this.MenuBar.ResumeLayout(false);
            this.MenuBar.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.DataGridView NodeGridView;
        private System.Windows.Forms.MenuStrip MenuBar;
        private System.Windows.Forms.ToolStripMenuItem FileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem SaveToolStripMenuItem;
        private System.Windows.Forms.ContextMenuStrip NodeContextMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem requestNodeNeighborUpdateToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem controllerToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem createNewPrmaryControllerToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem addControllerToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem addDeviceToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem removeControllerToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem removeDeviceToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem transferPrimaryRoleToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem receiveConfigurationToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
        private System.Windows.Forms.ToolStripMenuItem PowerOnToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem PowerOffToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator4;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator5;
        private System.Windows.Forms.ToolStripMenuItem hasNodeFailedToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem markNodeAsFailedToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem replaceFailedNodeToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator6;
        private System.Windows.Forms.ToolStripMenuItem propertiesToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem requestNetworkUpdateToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem assignReturnRouteToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem deleteReturnRouteToolStripMenuItem;
		private System.Windows.Forms.StatusStrip statusStrip1;
		private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator7;
		private System.Windows.Forms.ToolStripMenuItem resetControllersoftToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem softToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem eraseAllToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem addSecureDeviceToolStripMenuItem;
    }
}

