//-----------------------------------------------------------------------------
//
//      Node.cs
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
using System.Collections.Generic;
using OpenZWaveDotNet;

namespace OZWForm
{
    /// <summary>
    /// Container for Nodes
    /// </summary>
    public class Node
    {
        private Byte m_id = 0;
        /// <summary>
        /// Gets or sets the identifier.
        /// </summary>
        /// <value>
        /// The identifier.
        /// </value>
        public Byte ID
        {
            get { return m_id; }
            set { m_id = value; }
        }

        private UInt32 m_homeId = 0;
        /// <summary>
        /// Gets or sets the home identifier.
        /// </summary>
        /// <value>
        /// The home identifier.
        /// </value>
        public UInt32 HomeID
        {
            get { return m_homeId; }
            set { m_homeId = value; }
        }

        private String m_name = "";
        /// <summary>
        /// Gets or sets the name.
        /// </summary>
        /// <value>
        /// The name.
        /// </value>
        public String Name
        {
            get { return m_name; }
            set { m_name = value; }
        }

        private String m_location = "";
        /// <summary>
        /// Gets or sets the location.
        /// </summary>
        /// <value>
        /// The location.
        /// </value>
        public String Location
        {
            get { return m_location; }
            set { m_location = value; }
        }

        private String m_label = "";
        /// <summary>
        /// Gets or sets the label.
        /// </summary>
        /// <value>
        /// The label.
        /// </value>
        public String Label
        {
            get { return m_label; }
            set { m_label = value; }
        }

        private String m_manufacturer = "";
        /// <summary>
        /// Gets or sets the manufacturer.
        /// </summary>
        /// <value>
        /// The manufacturer.
        /// </value>
        public String Manufacturer
        {
            get { return m_manufacturer; }
            set { m_manufacturer = value; }
        }

        private String m_product = "";
        /// <summary>
        /// Gets or sets the product.
        /// </summary>
        /// <value>
        /// The product.
        /// </value>
        public String Product
        {
            get { return m_product; }
            set { m_product = value; }
        }

        private List<ZWValueID> m_values = new List<ZWValueID>();
        /// <summary>
        /// Gets the values.
        /// </summary>
        /// <value>
        /// The values.
        /// </value>
        public List<ZWValueID> Values
        {
            get { return m_values; }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="Node"/> class.
        /// </summary>
        public Node()
        {
        }

        /// <summary>
        /// Adds the value.
        /// </summary>
        /// <param name="valueID">The value identifier.</param>
        public void AddValue(ZWValueID valueID)
        {
            m_values.Add(valueID);
        }

        /// <summary>
        /// Removes the value.
        /// </summary>
        /// <param name="valueID">The value identifier.</param>
        public void RemoveValue(ZWValueID valueID)
        {
            m_values.Remove(valueID);
        }
    }
}
