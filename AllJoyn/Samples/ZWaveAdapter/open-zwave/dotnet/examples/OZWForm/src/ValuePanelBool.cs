//-----------------------------------------------------------------------------
//
//      ValuePanelBool.cs
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
    /// Valuepanel for containing boolean values
    /// </summary>
    public class ValuePanelBool: ValuePanel
    {
        private System.Windows.Forms.CheckBox ValueCheckBox;

        /// <summary>
        /// Initializes a new instance of the <see cref="ValuePanelBool"/> class.
        /// </summary>
        /// <param name="valueID">The value identifier.</param>
        public ValuePanelBool( ZWValueID valueID ): base( valueID )
        {
            InitializeComponent();

            if (MainForm.Manager.IsValueReadOnly(valueID))
            {
                ValueCheckBox.Enabled = false;
            }

            ValueCheckBox.Text = MainForm.Manager.GetValueLabel(valueID);
            
            bool state;
            if (MainForm.Manager.GetValueAsBool(valueID, out state))
            {
                ValueCheckBox.Checked = state;
            }

            SendChanges = true;
        }

        /// <summary>
        /// Initializes the component.
        /// </summary>
        private void InitializeComponent()
        {
            this.ValueCheckBox = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // ValueCheckBox
            // 
            this.ValueCheckBox.AutoSize = true;
            this.ValueCheckBox.Location = new System.Drawing.Point(4, 4);
            this.ValueCheckBox.Name = "ValueCheckBox";
            this.ValueCheckBox.Size = new System.Drawing.Size(52, 17);
            this.ValueCheckBox.TabIndex = 0;
            this.ValueCheckBox.Text = "Label";
            this.ValueCheckBox.UseVisualStyleBackColor = true;
            this.ValueCheckBox.CheckedChanged += new System.EventHandler(this.ValueCheckBox_CheckedChanged);
            // 
            // ValuePanelBool
            // 
            this.AutoSize = true;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.Controls.Add(this.ValueCheckBox);
            this.Name = "ValuePanelBool";
            this.Size = new System.Drawing.Size(59, 24);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        /// <summary>
        /// Handles the CheckedChanged event of the ValueCheckBox control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void ValueCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (SendChanges)
            {
                MainForm.Manager.SetValue(ValueID, ValueCheckBox.Checked);
            }
        }
    }
}
