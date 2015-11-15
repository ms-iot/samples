//-----------------------------------------------------------------------------
//
//      ValuePanel.cs
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

using System.Windows.Forms;
using OpenZWaveDotNet;

namespace OZWForm
{
    /// <summary>
    /// User control panel to show values
    /// </summary>
    public partial class ValuePanel : UserControl
    {
        private ZWValueID m_valueID;
        /// <summary>
        /// Gets the value identifier.
        /// </summary>
        /// <value>
        /// The value identifier.
        /// </value>
        public ZWValueID ValueID
        {
            get { return m_valueID; }
        }

        private bool m_sendChanges = false;
        /// <summary>
        /// Gets or sets a value indicating whether [send changes].
        /// </summary>
        /// <value>
        ///   <c>true</c> if [send changes]; otherwise, <c>false</c>.
        /// </value>
        public bool SendChanges
        {
            get { return m_sendChanges; }
            set { m_sendChanges = value; }
        }

        /// <summary>
        /// Prevents a default instance of the <see cref="ValuePanel"/> class from being created.
        /// </summary>
        private ValuePanel()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ValuePanel"/> class.
        /// </summary>
        /// <param name="valueID">The value identifier.</param>
        public ValuePanel( ZWValueID valueID )
        {
            m_valueID = valueID;
            InitializeComponent();
        }
    }
}
