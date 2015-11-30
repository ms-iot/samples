//-----------------------------------------------------------------------------
//
//      ValuePanelInt.cs
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
    /// Value panel containing int values
    /// </summary>
    public class ValuePanelInt: ValuePanel
    {
        private System.Windows.Forms.NumericUpDown ValueIntNumericUpDown;
        private System.Windows.Forms.Label ValueIntLabel;
        private System.Windows.Forms.Button ValueIntButtonSet;

        /// <summary>
        /// Initializes a new instance of the <see cref="ValuePanelInt"/> class.
        /// </summary>
        /// <param name="valueID">The value identifier.</param>
        public ValuePanelInt( ZWValueID valueID ): base( valueID )
        {
            InitializeComponent();

            ValueIntLabel.Text = MainForm.Manager.GetValueLabel(valueID);

            if (MainForm.Manager.IsValueReadOnly(valueID))
            {
                ValueIntNumericUpDown.Enabled = false;
                ValueIntButtonSet.Visible = false;
            }

            Int32 value;
            if (MainForm.Manager.GetValueAsInt(valueID, out value))
            {
                ValueIntNumericUpDown.Value = Convert.ToDecimal(value);
            }

            SendChanges = true;
        }

        /// <summary>
        /// Initializes the component.
        /// </summary>
        private void InitializeComponent()
        {
            this.ValueIntNumericUpDown = new System.Windows.Forms.NumericUpDown();
            this.ValueIntButtonSet = new System.Windows.Forms.Button();
            this.ValueIntLabel = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.ValueIntNumericUpDown)).BeginInit();
            this.SuspendLayout();
            // 
            // ValueIntNumericUpDown
            // 
            this.ValueIntNumericUpDown.Location = new System.Drawing.Point(3, 6);
            this.ValueIntNumericUpDown.Maximum = new decimal(new int[] {
            2147483647,
            0,
            0,
            0});
            this.ValueIntNumericUpDown.Minimum = new decimal(new int[] {
            -2147483648,
            0,
            0,
            -2147483648});
            this.ValueIntNumericUpDown.Name = "ValueIntNumericUpDown";
            this.ValueIntNumericUpDown.Size = new System.Drawing.Size(59, 20);
            this.ValueIntNumericUpDown.TabIndex = 0;
            this.ValueIntNumericUpDown.ThousandsSeparator = true;
            // 
            // ValueIntButtonSet
            // 
            this.ValueIntButtonSet.Location = new System.Drawing.Point(68, 6);
            this.ValueIntButtonSet.Name = "ValueIntButtonSet";
            this.ValueIntButtonSet.Size = new System.Drawing.Size(59, 20);
            this.ValueIntButtonSet.TabIndex = 1;
            this.ValueIntButtonSet.Text = "Set";
            this.ValueIntButtonSet.UseVisualStyleBackColor = true;
            this.ValueIntButtonSet.Click += new System.EventHandler(this.ValueIntButtonSet_Click);
            // 
            // ValueIntLabel
            // 
            this.ValueIntLabel.AutoSize = true;
            this.ValueIntLabel.Location = new System.Drawing.Point(133, 10);
            this.ValueIntLabel.Name = "ValueIntLabel";
            this.ValueIntLabel.Size = new System.Drawing.Size(33, 13);
            this.ValueIntLabel.TabIndex = 2;
            this.ValueIntLabel.Text = "Label";
            // 
            // ValuePanelInt
            // 
            this.AutoSize = true;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.Controls.Add(this.ValueIntLabel);
            this.Controls.Add(this.ValueIntButtonSet);
            this.Controls.Add(this.ValueIntNumericUpDown);
            this.Name = "ValuePanelInt";
            this.Size = new System.Drawing.Size(169, 29);
            ((System.ComponentModel.ISupportInitialize)(this.ValueIntNumericUpDown)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        /// <summary>
        /// Handles the Click event of the ValueIntButtonSet control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void ValueIntButtonSet_Click(object sender, EventArgs e)
        {
            if (SendChanges)
            {
                Int32 value = Convert.ToInt32(ValueIntNumericUpDown.Value);
                MainForm.Manager.SetValue(ValueID, value);
            }
        }
    }
}
