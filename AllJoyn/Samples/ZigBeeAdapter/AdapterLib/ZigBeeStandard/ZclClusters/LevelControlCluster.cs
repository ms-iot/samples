// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AdapterLib
{
    class LevelControlCluster : ZclCluster
    {
        internal const UInt16 CLUSTER_ID = 0x0008;
        internal const string CLUSTER_NAME = "LevelControl";

        internal const UInt16 ATTRIBUTE_CURRENTLEVEL = 0x0000;
        internal const UInt16 ATTRIBUTE_REMAININGTIME = 0x0001;
        internal const UInt16 ATTRIBUTE_ONOFFTRANSITIONTIME = 0x0010;
        internal const UInt16 ATTRIBUTE_ONLEVEL = 0x0011;

        internal const byte COMMAND_MOVETOLEVEL = 0x00;
        internal const byte COMMAND_MOVE = 0x01;
        internal const byte COMMAND_STEP = 0x02;
        internal const byte COMMAND_STOP = 0x03;
        internal const byte COMMAND_MOVETOLEVEL_WITHONOFF = 0x04;
        internal const byte COMMAND_MOVE_WITHONOFF = 0x05;
        internal const byte COMMAND_STEP_WITHONOFF = 0x06;
        internal const byte COMMAND_STOP_WITHONOFF = 0x07;

        internal const string LEVEL_PARAM = "Level";
        internal const byte MAX_LEVEL = (byte.MaxValue - 1);
        internal LevelControlCluster(ZigBeeEndPoint endPoint, List<UInt16> supportedAttributes)
            : base(endPoint)
        {
            // set cluster name and cluster Id
            Name = CLUSTER_NAME;
            m_Id = CLUSTER_ID;

            // if supportedAttributes is null assume all attributes supported
            bool supportAllAttributesBydefault = false;
            if (supportedAttributes == null)
            {
                supportAllAttributesBydefault = true;
            }
            // current level attribute (read only)
            if (supportAllAttributesBydefault ||
                supportedAttributes.Contains(ATTRIBUTE_CURRENTLEVEL))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_CURRENTLEVEL, "CurrentLevel", ZclHelper.UINT8_TYPE, true, false);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // remaining time attribute (read only)
            if (supportAllAttributesBydefault ||
                supportedAttributes.Contains(ATTRIBUTE_REMAININGTIME))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_REMAININGTIME, "RemainingTime", ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // on/off transition time attribute (read/write)
            if (supportAllAttributesBydefault ||
                supportedAttributes.Contains(ATTRIBUTE_ONOFFTRANSITIONTIME))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_ONOFFTRANSITIONTIME, "OnOffTransitionTime", ZclHelper.UINT16_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // on level attribute (read/write)
            if (supportAllAttributesBydefault ||
                supportedAttributes.Contains(ATTRIBUTE_ONLEVEL))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_ONLEVEL, "OnLevel", ZclHelper.UINT8_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }

            // IMPORTANT NOTE,  parameters MUST be added in parameter list of each command 
            // in the order specified in ZCL standard

            // move to level command (no response required)
            var command = new ZclCommand(this, COMMAND_MOVETOLEVEL, "MoveToLevel", false);
            var parameter = new ZclValue(ZclHelper.UINT8_TYPE, LEVEL_PARAM);
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT16_TYPE, "TransitionTime");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);

            // Move command (no response required)
            command = new ZclCommand(this, COMMAND_MOVE, "Move", false);
            parameter = new ZclValue(ZclHelper.ENUMERATION_8_BIT_TYPE, "MoveMode");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT8_TYPE, "Rate");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);
            
            // Step command (no response required)
            command = new ZclCommand(this, COMMAND_STEP, "Step", false);
            parameter = new ZclValue(ZclHelper.ENUMERATION_8_BIT_TYPE, "StepMode");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT8_TYPE, "StepSize");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT16_TYPE, "TransitionTime");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);
            
            // Stop command (no response required)
            command = new ZclCommand(this, COMMAND_STOP, "Stop", false);
            m_commandList.Add(command.Id, command);

            // move to level with on/off command (no response required)
            command = new ZclCommand(this, COMMAND_MOVETOLEVEL_WITHONOFF, "MoveToLevelWithOnOff", false);
            parameter = new ZclValue(ZclHelper.UINT8_TYPE, LEVEL_PARAM);
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT16_TYPE, "TransitionTime");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);

            // Move command (no response required)
            command = new ZclCommand(this, COMMAND_MOVE_WITHONOFF, "MoveWithOnOff", false);
            parameter = new ZclValue(ZclHelper.ENUMERATION_8_BIT_TYPE, "MoveMode");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT8_TYPE, "Rate");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);

            // Step command (no response required)
            command = new ZclCommand(this, COMMAND_STEP_WITHONOFF, "StepWithOnOff", false);
            parameter = new ZclValue(ZclHelper.ENUMERATION_8_BIT_TYPE, "StepMode");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT8_TYPE, "StepSize");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT16_TYPE, "TransitionTime");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);

            // Stop command (no response required)
            command = new ZclCommand(this, COMMAND_STOP_WITHONOFF, "StopWithOnOff", false);
            m_commandList.Add(command.Id, command);

            // call parent class "post constructor"
            PostChildClassConstructor();
        }
    }
}
