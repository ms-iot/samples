//-----------------------------------------------------------------------------
//
//      ValuePanelDecimal.cs
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
    /// Value panel for containing decimal values
    /// </summary>
    public class ValuePanelDecimal: ValuePanel
    {
        private System.Windows.Forms.TextBox ValueDecimalTextBox;
        private System.Windows.Forms.Button ValueDecimalButtonSet;
        private System.Windows.Forms.Label ValueDecimalLabel;

        /// <summary>
        /// Initializes a new instance of the <see cref="ValuePanelDecimal"/> class.
        /// </summary>
        /// <param name="valueID">The value identifier.</param>
        public ValuePanelDecimal( ZWValueID valueID ): base( valueID )
        {
            InitializeComponent();

            ValueDecimalLabel.Text = MainForm.Manager.GetValueLabel(valueID);

            if (MainForm.Manager.IsValueReadOnly(valueID))
            {
                ValueDecimalTextBox.Enabled = false;
				ValueDecimalButtonSet.Enabled = false;
            }

            Decimal value;
            if (MainForm.Manager.GetValueAsDecimal(valueID, out value))
            {
                ValueDecimalTextBox.Text = value.ToString();
            }

            SendChanges = true;
        }

        /// <summary>
        /// Initializes the component.
        /// </summary>
        private void InitializeComponent()
        {
            this.ValueDecimalLabel = new System.Windows.Forms.Label();
            this.ValueDecimalTextBox = new System.Windows.Forms.TextBox();
            this.ValueDecimalButtonSet = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // ValueDecimalLabel
            // 
            this.ValueDecimalLabel.AutoSize = true;
            this.ValueDecimalLabel.Location = new System.Drawing.Point(152, 7);
            this.ValueDecimalLabel.Name = "ValueDecimalLabel";
            this.ValueDecimalLabel.Size = new System.Drawing.Size(33, 13);
            this.ValueDecimalLabel.TabIndex = 2;
            this.ValueDecimalLabel.Text = "Label";
            // 
            // ValueDecimalTextBox
            // 
            this.ValueDecimalTextBox.Location = new System.Drawing.Point(4, 4);
            this.ValueDecimalTextBox.MaxLength = 255;
            this.ValueDecimalTextBox.Name = "ValueDecimalTextBox";
            this.ValueDecimalTextBox.Size = new System.Drawing.Size(77, 20);
            this.ValueDecimalTextBox.TabIndex = 3;
            // 
            // ValueDecimalButtonSet
            // 
            this.ValueDecimalButtonSet.Location = new System.Drawing.Point(87, 3);
            this.ValueDecimalButtonSet.Name = "ValueDecimalButtonSet";
            this.ValueDecimalButtonSet.Size = new System.Drawing.Size(59, 20);
            this.ValueDecimalButtonSet.TabIndex = 4;
            this.ValueDecimalButtonSet.Text = "Set";
            this.ValueDecimalButtonSet.UseVisualStyleBackColor = true;
            this.ValueDecimalButtonSet.Click += new System.EventHandler(this.ValueDecimalButtonSet_Click);
            // 
            // ValuePanelDecimal
            // 
            this.AutoSize = true;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.Controls.Add(this.ValueDecimalButtonSet);
            this.Controls.Add(this.ValueDecimalTextBox);
            this.Controls.Add(this.ValueDecimalLabel);
            this.Name = "ValuePanelDecimal";
            this.Size = new System.Drawing.Size(188, 27);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        /// <summary>
        /// Handles the Click event of the ValueDecimalButtonSet control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void ValueDecimalButtonSet_Click(object sender, EventArgs e)
        {
            if (SendChanges)
            {
                float value = Convert.ToSingle(ValueDecimalTextBox.Text);
                MainForm.Manager.SetValue(ValueID, value);
            }
        }
    }
}
