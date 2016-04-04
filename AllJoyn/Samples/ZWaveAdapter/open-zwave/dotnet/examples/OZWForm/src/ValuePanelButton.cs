//-----------------------------------------------------------------------------
//
//      ValuePanelButton.cs
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

using OpenZWaveDotNet;

namespace OZWForm
{
    /// <summary>
    /// Value panel for containing buttons
    /// </summary>
    public class ValuePanelButton: ValuePanel
    {
        private System.Windows.Forms.Button ValueButtonButton;

        /// <summary>
        /// Initializes a new instance of the <see cref="ValuePanelButton"/> class.
        /// </summary>
        /// <param name="valueID">The value identifier.</param>
        public ValuePanelButton( ZWValueID valueID ): base( valueID )
        {
            InitializeComponent();

            ValueButtonButton.Text = MainForm.Manager.GetValueLabel(valueID);

            if (MainForm.Manager.IsValueReadOnly(valueID))
            {
                ValueButtonButton.Enabled = false;
            }

            SendChanges = true;
        }

        /// <summary>
        /// Initializes the component.
        /// </summary>
        private void InitializeComponent()
        {
            this.ValueButtonButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // ValueButtonButton
            // 
            this.ValueButtonButton.AutoSize = true;
            this.ValueButtonButton.Location = new System.Drawing.Point(3, 6);
            this.ValueButtonButton.Name = "ValueButtonButton";
            this.ValueButtonButton.Size = new System.Drawing.Size(59, 23);
            this.ValueButtonButton.TabIndex = 1;
            this.ValueButtonButton.Text = "Label";
            this.ValueButtonButton.UseVisualStyleBackColor = true;
            this.ValueButtonButton.KeyUp += new System.Windows.Forms.KeyEventHandler(this.ValueButtonButton_KeyUp);
            this.ValueButtonButton.KeyDown += new System.Windows.Forms.KeyEventHandler(this.ValueButtonButton_KeyDown);
            // 
            // ValuePanelButton
            // 
            this.AutoSize = true;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.Controls.Add(this.ValueButtonButton);
            this.Name = "ValuePanelButton";
            this.Size = new System.Drawing.Size(65, 32);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        /// <summary>
        /// Handles the KeyDown event of the ValueButtonButton control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.KeyEventArgs"/> instance containing the event data.</param>
        private void ValueButtonButton_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            if (SendChanges)
            {
                MainForm.Manager.PressButton(ValueID);
            }
        }

        /// <summary>
        /// Handles the KeyUp event of the ValueButtonButton control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.KeyEventArgs"/> instance containing the event data.</param>
        private void ValueButtonButton_KeyUp(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            if (SendChanges)
            {
                MainForm.Manager.ReleaseButton(ValueID);
            }
        }
    }
}
