//-----------------------------------------------------------------------------
//
//      ValuePanelShort.cs
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
    /// Value panel to contain shorts
    /// </summary>
    public class ValuePanelShort: ValuePanel
    {
        private System.Windows.Forms.NumericUpDown ValueShortNumericUpDown;
        private System.Windows.Forms.Label ValueShortLabel;
        private System.Windows.Forms.Button ValueShortButtonSet;

        /// <summary>
        /// Initializes a new instance of the <see cref="ValuePanelShort"/> class.
        /// </summary>
        /// <param name="valueID">The value identifier.</param>
        public ValuePanelShort( ZWValueID valueID ): base( valueID )
        {
            InitializeComponent();

            ValueShortLabel.Text = MainForm.Manager.GetValueLabel(valueID);

            if (MainForm.Manager.IsValueReadOnly(valueID))
            {
                ValueShortNumericUpDown.Enabled = false;
                ValueShortButtonSet.Visible = false;
            }

            Int16 value;
            if (MainForm.Manager.GetValueAsShort(valueID, out value))
            {
                ValueShortNumericUpDown.Value = Convert.ToDecimal(value);
            }

            SendChanges = true;
        }

        /// <summary>
        /// Initializes the component.
        /// </summary>
        private void InitializeComponent()
        {
            this.ValueShortNumericUpDown = new System.Windows.Forms.NumericUpDown();
            this.ValueShortButtonSet = new System.Windows.Forms.Button();
            this.ValueShortLabel = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.ValueShortNumericUpDown)).BeginInit();
            this.SuspendLayout();
            // 
            // ValueShortNumericUpDown
            // 
            this.ValueShortNumericUpDown.Location = new System.Drawing.Point(3, 6);
            this.ValueShortNumericUpDown.Maximum = new decimal(new int[] {
            32767,
            0,
            0,
            0});
            this.ValueShortNumericUpDown.Minimum = new decimal(new int[] {
            32768,
            0,
            0,
            -2147483648});
            this.ValueShortNumericUpDown.Name = "ValueShortNumericUpDown";
            this.ValueShortNumericUpDown.Size = new System.Drawing.Size(59, 20);
            this.ValueShortNumericUpDown.TabIndex = 0;
            // 
            // ValueShortButtonSet
            // 
            this.ValueShortButtonSet.Location = new System.Drawing.Point(68, 6);
            this.ValueShortButtonSet.Name = "ValueShortButtonSet";
            this.ValueShortButtonSet.Size = new System.Drawing.Size(59, 20);
            this.ValueShortButtonSet.TabIndex = 1;
            this.ValueShortButtonSet.Text = "Set";
            this.ValueShortButtonSet.UseVisualStyleBackColor = true;
            this.ValueShortButtonSet.Click += new System.EventHandler(this.ValueShortButtonSet_Click);
            // 
            // ValueShortLabel
            // 
            this.ValueShortLabel.AutoSize = true;
            this.ValueShortLabel.Location = new System.Drawing.Point(133, 10);
            this.ValueShortLabel.Name = "ValueShortLabel";
            this.ValueShortLabel.Size = new System.Drawing.Size(33, 13);
            this.ValueShortLabel.TabIndex = 2;
            this.ValueShortLabel.Text = "Label";
            // 
            // ValuePanelShort
            // 
            this.AutoSize = true;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.Controls.Add(this.ValueShortLabel);
            this.Controls.Add(this.ValueShortButtonSet);
            this.Controls.Add(this.ValueShortNumericUpDown);
            this.Name = "ValuePanelShort";
            this.Size = new System.Drawing.Size(169, 29);
            ((System.ComponentModel.ISupportInitialize)(this.ValueShortNumericUpDown)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        /// <summary>
        /// Handles the Click event of the ValueShortButtonSet control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EventArgs"/> instance containing the event data.</param>
        private void ValueShortButtonSet_Click(object sender, EventArgs e)
        {
            if (SendChanges)
            {
                Int16 value = Convert.ToInt16(ValueShortNumericUpDown.Value);
                MainForm.Manager.SetValue(ValueID, value);
            }
        }
    }
}
