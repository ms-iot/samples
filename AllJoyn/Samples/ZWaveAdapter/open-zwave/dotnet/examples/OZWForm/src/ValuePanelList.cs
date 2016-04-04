//-----------------------------------------------------------------------------
//
//      ValuePanelList.cs
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
using OpenZWaveDotNet;

namespace OZWForm
{
    /// <summary>
    /// Value panel containing lists
    /// </summary>
    public class ValuePanelList: ValuePanel
    {
        private System.Windows.Forms.ComboBox ValueListComboBox;
        private System.Windows.Forms.Label ValueListLabel;

        /// <summary>
        /// Initializes a new instance of the <see cref="ValuePanelList"/> class.
        /// </summary>
        /// <param name="valueID">The value identifier.</param>
        public ValuePanelList( ZWValueID valueID ): base( valueID )
        {
            InitializeComponent();

            ValueListLabel.Text = MainForm.Manager.GetValueLabel(valueID);

            if (MainForm.Manager.IsValueReadOnly(valueID))
            {
                ValueListComboBox.Enabled = false;
            }

            String[] items;
            if (MainForm.Manager.GetValueListItems(valueID, out items))
            {
                ValueListComboBox.Items.Clear();
                foreach (String item in items)
                {
                    ValueListComboBox.Items.Add(item);
                }
            }

            String value;
            if (MainForm.Manager.GetValueListSelection(valueID, out value))
            {
                ValueListComboBox.Text = value;
            }

            SendChanges = true;
        }

        /// <summary>
        /// Initializes the component.
        /// </summary>
        private void InitializeComponent()
        {
            this.ValueListLabel = new System.Windows.Forms.Label();
            this.ValueListComboBox = new System.Windows.Forms.ComboBox();
            this.SuspendLayout();
            // 
            // ValueListLabel
            // 
            this.ValueListLabel.AutoSize = true;
            this.ValueListLabel.Location = new System.Drawing.Point(133, 10);
            this.ValueListLabel.Name = "ValueListLabel";
            this.ValueListLabel.Size = new System.Drawing.Size(33, 13);
            this.ValueListLabel.TabIndex = 2;
            this.ValueListLabel.Text = "Label";
            // 
            // ValueListComboBox
            // 
            this.ValueListComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.ValueListComboBox.FormattingEnabled = true;
            this.ValueListComboBox.Location = new System.Drawing.Point(4, 4);
            this.ValueListComboBox.MaxDropDownItems = 100;
            this.ValueListComboBox.Name = "ValueListComboBox";
            this.ValueListComboBox.Size = new System.Drawing.Size(121, 21);
            this.ValueListComboBox.TabIndex = 3;
            this.ValueListComboBox.SelectedIndexChanged += new System.EventHandler(this.ValueListComboBox_SelectedIndexChanged);
            // 
            // ValuePanelList
            // 
            this.AutoSize = true;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.Controls.Add(this.ValueListComboBox);
            this.Controls.Add(this.ValueListLabel);
            this.Name = "ValuePanelList";
            this.Size = new System.Drawing.Size(169, 28);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        /// <summary>
        /// Handles the SelectedIndexChanged event of the ValueListComboBox control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void ValueListComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (SendChanges)
            {
                MainForm.Manager.SetValueListSelection(ValueID, ValueListComboBox.Text);
            }
        }
    }
}
