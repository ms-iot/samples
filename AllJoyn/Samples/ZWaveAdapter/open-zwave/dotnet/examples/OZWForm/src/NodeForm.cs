//-----------------------------------------------------------------------------
//
//      NodeForm.cs
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
    /// Shows a form containing all the details and configuration parameters of a node
    /// </summary>
    public partial class NodeForm : Form
    {
        private Node m_node;

        /// <summary>
        /// Initializes a new instance of the <see cref="NodeForm"/> class.
        /// </summary>
        /// <param name="node">The node.</param>
        public NodeForm( Node node )
        {
            m_node = node;
            InitializeComponent();

			// load all possible configuration parameters
			for (byte i = 0; i <= 10; i++)
			{
				MainForm.Manager.RequestConfigParam(node.HomeID, node.ID, i);
			}

			if (node.Manufacturer != "")
            {
                this.Text = "Node " + node.ID.ToString() + ": " + node.Manufacturer + " " + node.Product;
            }
            else
            {
                this.Text = "Node " + node.ID.ToString() + ": " + node.Label;
            }

            foreach (ZWValueID valueID in node.Values)
            {
                Control control = null;
                switch (valueID.GetType())
                {
                    case ZWValueID.ValueType.Bool:
                    {
                        control = new ValuePanelBool(valueID);
                        break;
                    }
                    case ZWValueID.ValueType.Button:
                    {
                        control = new ValuePanelButton(valueID);
                        break;
                    }
                    case ZWValueID.ValueType.Byte:
                    {
                        control = new ValuePanelByte(valueID);
                        break;
                    }
                    case ZWValueID.ValueType.Decimal:
                    {
                        control = new ValuePanelDecimal(valueID);
                        break;
                    }
                    case ZWValueID.ValueType.Int:
                    {
                        control = new ValuePanelInt(valueID);
                        break;
                    }
                    case ZWValueID.ValueType.List:
                    {
                        control = new ValuePanelList(valueID);
                        break;
                    }
                    case ZWValueID.ValueType.Short:
                    {
                        control = new ValuePanelShort(valueID);
                        break;
                    }
                    case ZWValueID.ValueType.String:
                    {
                        control = new ValuePanelString(valueID);
                        break;
                    }
                }

                if (control != null)
                {
                    NodeLayoutPanel.Controls.Add(control);
                }
            }

        }
	}
}
