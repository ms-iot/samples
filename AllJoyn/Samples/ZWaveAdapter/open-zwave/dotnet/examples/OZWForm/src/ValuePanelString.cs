//-----------------------------------------------------------------------------
//
//      ValuePanelString.cs
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
    /// Value panel to contain strings
    /// </summary>
    public class ValuePanelString: ValuePanel
    {
        private System.Windows.Forms.TextBox ValueStringTextBox;
        private System.Windows.Forms.Label ValueStringLabel;

        /// <summary>
        /// Initializes a new instance of the <see cref="ValuePanelString"/> class.
        /// </summary>
        /// <param name="valueID">The value identifier.</param>
        public ValuePanelString( ZWValueID valueID ): base( valueID )
        {
            InitializeComponent();

            ValueStringLabel.Text = MainForm.Manager.GetValueLabel(valueID);

            if (MainForm.Manager.IsValueReadOnly(valueID))
            {
                ValueStringTextBox.Enabled = false;
            }

            String value;
            if (MainForm.Manager.GetValueAsString(valueID, out value))
            {
                ValueStringTextBox.Text = value;
            }

            SendChanges = true;
        }

        /// <summary>
        /// Initializes the component.
        /// </summary>
        private void InitializeComponent()
        {
            this.ValueStringLabel = new System.Windows.Forms.Label();
            this.ValueStringTextBox = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // ValueStringLabel
            // 
            this.ValueStringLabel.AutoSize = true;
            this.ValueStringLabel.Location = new System.Drawing.Point(133, 10);
            this.ValueStringLabel.Name = "ValueStringLabel";
            this.ValueStringLabel.Size = new System.Drawing.Size(33, 13);
            this.ValueStringLabel.TabIndex = 2;
            this.ValueStringLabel.Text = "Label";
            // 
            // ValueStringTextBox
            // 
            this.ValueStringTextBox.Location = new System.Drawing.Point(4, 4);
            this.ValueStringTextBox.MaxLength = 255;
            this.ValueStringTextBox.Name = "ValueStringTextBox";
            this.ValueStringTextBox.Size = new System.Drawing.Size(123, 20);
            this.ValueStringTextBox.TabIndex = 3;
            // 
            // ValuePanelString
            // 
            this.AutoSize = true;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.Controls.Add(this.ValueStringTextBox);
            this.Controls.Add(this.ValueStringLabel);
            this.Name = "ValuePanelString";
            this.Size = new System.Drawing.Size(169, 27);
            this.ResumeLayout(false);
            this.PerformLayout();

        }
    }
}
