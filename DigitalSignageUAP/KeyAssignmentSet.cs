/**
    Copyright(c) Microsoft Open Technologies, Inc.All rights reserved.
   The MIT License(MIT)
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
**/

namespace OnScreenKeyboardSample
{
    public class KeyAssignmentSet
    {
        private static KeyAssignmentSet s_theKeyAssignmentSet;
        private KeyModel[] theKeyAssignmentArray;

        public static KeyAssignmentSet KeyAssignment
        {
            get
            {
                if (s_theKeyAssignmentSet == null)
                {
                    s_theKeyAssignmentSet = new KeyAssignmentSet();
                }
                return s_theKeyAssignmentSet;
            }
        }

        /// <summary>
        /// Get the array of all the KeyAssignment objects within this set, in the same order and number as the key-button array within the virtual keyboard.
        /// </summary>
        public KeyModel[] KeyAssignments
        {
            get
            {
                if (theKeyAssignmentArray == null)
                {
                    theKeyAssignmentArray = new KeyModel[] {
                        KeyVK_Oem3, KeyVK_1, KeyVK_2, KeyVK_3, KeyVK_4, KeyVK_5, KeyVK_6, KeyVK_7, KeyVK_8, KeyVK_9, KeyVK_0, KeyVK_OemMinus, KeyVK_OemPlus,
                        KeyVK_Q, KeyVK_W, KeyVK_E, KeyVK_R, KeyVK_T, KeyVK_Y, KeyVK_U, KeyVK_I, KeyVK_O, KeyVK_P, KeyVK_OemOpenBrackets, KeyVK_Oem6, KeyVK_Oem5,
                        KeyVK_A, KeyVK_S, KeyVK_D, KeyVK_F, KeyVK_G, KeyVK_H, KeyVK_J, KeyVK_K, KeyVK_L, KeyVK_Oem1, KeyVK_Oem7,
                        KeyVK_Z, KeyVK_X, KeyVK_C, KeyVK_V, KeyVK_B, KeyVK_N, KeyVK_M, KeyVK_OemComma, KeyVK_OemPeriod, KeyVK_OemQuestion
                    };
                }
                return theKeyAssignmentArray;
            }
        }

        #region The individual key assignments

        #region 0:  KeyVK_Oem3 (VK_OEM_3)
        /// <summary>
        /// Get the KeyAssignment for VK_OEM_3 (0: Grace Accent).
        /// </summary>
        public virtual KeyModel KeyVK_Oem3
        {
            get
            {
                if (vk_Oem3 == null)
                {
                    vk_Oem3 = new KeyModelWithTwoGlyphs(0x0060, 0x007E, false);
                }
                return vk_Oem3;
            }
        }
        protected KeyModel vk_Oem3;
        #endregion

        #region 1:  KeyVK_1 + !
        public virtual KeyModel KeyVK_1
        {
            get
            {
                if (vk_1 == null)
                {
                    vk_1 = new KeyModelWithTwoGlyphs(0x0031, 0x0021, false);
                }
                return vk_1;
            }
        }
        protected KeyModel vk_1;
        #endregion

        #region 2:  KeyVK_2 + Amphora
        public virtual KeyModel KeyVK_2
        {
            get
            {
                if (vk_2 == null)
                {
                    vk_2 = new KeyModelWithTwoGlyphs(0x0032, 0x0040, false);
                }
                return vk_2;
            }
        }
        protected KeyModel vk_2;
        #endregion

        #region 3:  KeyVK_3 + #
        public virtual KeyModel KeyVK_3
        {
            get
            {
                if (vk_3 == null)
                {
                    vk_3 = new KeyModelWithTwoGlyphs(0x0033, 0x0023, false);
                }
                return vk_3;
            }
        }
        protected KeyModel vk_3;
        #endregion

        #region 4:  KeyVK_4 + $
        public virtual KeyModel KeyVK_4
        {
            get
            {
                if (vk_4 == null)
                {
                    vk_4 = new KeyModelWithTwoGlyphs(0x0034, 0x0024, false);
                }
                return vk_4;
            }
        }
        protected KeyModel vk_4;
        #endregion

        #region 5:  KeyVK_5 + %
        public virtual KeyModel KeyVK_5
        {
            get
            {
                if (vk_5 == null)
                {
                    vk_5 = new KeyModelWithTwoGlyphs(0x0035, 0x0025, false);
                }
                return vk_5;
            }
        }
        protected KeyModel vk_5;
        #endregion

        #region 6:  KeyVK_6
        public virtual KeyModel KeyVK_6
        {
            get
            {
                if (vk_6 == null)
                {
                    vk_6 = new KeyModelWithTwoGlyphs(0x0036, 0x005E, false);
                }
                return vk_6;
            }
        }
        protected KeyModel vk_6;
        #endregion

        #region 7:  KeyVK_7 + Ampersand
        public virtual KeyModel KeyVK_7
        {
            get
            {
                if (vk_7 == null)
                {
                    vk_7 = new KeyModelWithTwoGlyphs(0x0037, 0x0026, false);
                }
                return vk_7;
            }
        }
        protected KeyModel vk_7;
        #endregion

        #region 8:  KeyVK_8 + *
        public virtual KeyModel KeyVK_8
        {
            get
            {
                if (vk_8 == null)
                {
                    vk_8 = new KeyModelWithTwoGlyphs(0x0038, 0x002A, false);
                }
                return vk_8;
            }
        }
        protected KeyModel vk_8;
        #endregion

        #region 9:  KeyVK_9 + (
        public virtual KeyModel KeyVK_9
        {
            get
            {
                if (vk_9 == null)
                {
                    vk_9 = new KeyModelWithTwoGlyphs(0x0039, 0x0028, false);
                }
                return vk_9;
            }
        }
        protected KeyModel vk_9;
        #endregion

        #region 10: KeyVK_0 + )
        public virtual KeyModel KeyVK_0
        {
            get
            {
                if (vk_0 == null)
                {
                    vk_0 = new KeyModelWithTwoGlyphs(0x0030, 0x0029, false);
                }
                return vk_0;
            }
        }
        protected KeyModel vk_0;
        #endregion

        #region 11: KeyVK_OemMinus (VK_OEM_MINUS) + underscore
        /// <summary>
        /// 11  Hyphen
        /// </summary>
        public virtual KeyModel KeyVK_OemMinus
        {
            get
            {
                if (vk_OemMinus == null)
                {
                    vk_OemMinus = new KeyModelWithTwoGlyphs(0x002D, 0x005F, false);
                }
                return vk_OemMinus;
            }
        }
        protected KeyModel vk_OemMinus;
        #endregion

        #region 12: KeyVK_OemPlus (VK_OEM_PLUS, = + Plus-sign)
        public virtual KeyModel KeyVK_OemPlus
        {
            get
            {
                if (vk_OemPlus == null)
                {
                    vk_OemPlus = new KeyModelWithTwoGlyphs(0x03D, 0x002B, false);
                }
                return vk_OemPlus;
            }
        }
        protected KeyModel vk_OemPlus;
        #endregion

        #region 13: KeyVK_Q
        public virtual KeyModel KeyVK_Q
        {
            get
            {
                if (vk_Q == null)
                {
                    vk_Q = new KeyModel(0x0071, 0x0051);
                }
                return vk_Q;
            }
        }
        protected KeyModel vk_Q;
        #endregion

        #region 14: KeyVK_W
        public virtual KeyModel KeyVK_W
        {
            get
            {
                if (vk_W == null)
                {
                    vk_W = new KeyModel(0x0077, 0x0057);
                }
                return vk_W;
            }
        }
        protected KeyModel vk_W;
        #endregion

        #region 15: KeyVK_E
        /// <summary>
        /// 15
        /// </summary>
        public virtual KeyModel KeyVK_E
        {
            get
            {
                if (vk_E == null)
                {
                    vk_E = new KeyModel(0x0065, 0x0045);
                }
                return vk_E;
            }
        }
        protected KeyModel vk_E;
        #endregion

        #region 16: KeyVK_R
        /// <summary>
        /// 16
        /// </summary>
        public virtual KeyModel KeyVK_R
        {
            get
            {
                if (vk_R == null)
                {
                    vk_R = new KeyModel(0x0072, 0x0052);
                }
                return vk_R;
            }
        }
        protected KeyModel vk_R;
        #endregion

        #region 17: KeyVK_T
        /// <summary>
        /// 17
        /// </summary>
        public virtual KeyModel KeyVK_T
        {
            get
            {
                if (vk_T == null)
                {
                    vk_T = new KeyModel(0x0074, 0x0054);
                }
                return vk_T;
            }
        }
        protected KeyModel vk_T;
        #endregion

        #region 18: KeyVK_Y
        /// <summary>
        /// 18
        /// </summary>
        public virtual KeyModel KeyVK_Y
        {
            get
            {
                if (vk_Y == null)
                {
                    vk_Y = new KeyModel(0x0079, 0x0059);
                }
                return vk_Y;
            }
        }
        protected KeyModel vk_Y;
        #endregion

        #region 19: KeyVK_U
        /// <summary>
        /// 19
        /// </summary>
        public virtual KeyModel KeyVK_U
        {
            get
            {
                if (vk_U == null)
                {
                    vk_U = new KeyModel(0x0075, 0x0055);
                }
                return vk_U;
            }
        }
        protected KeyModel vk_U;
        #endregion

        #region 20: KeyVK_I
        public virtual KeyModel KeyVK_I
        {
            get
            {
                if (vk_I == null)
                {
                    vk_I = new KeyModel(0x0069, 0x0049);
                }
                return vk_I;
            }
        }
        protected KeyModel vk_I;
        #endregion

        #region 21: KeyVK_O
        /// <summary>
        /// 21
        /// </summary>
        public virtual KeyModel KeyVK_O
        {
            get
            {
                if (vk_O == null)
                {
                    vk_O = new KeyModel(0x006F, 0x004F);
                }
                return vk_O;
            }
        }
        protected KeyModel vk_O;
        #endregion

        #region 22: KeyVK_P
        /// <summary>
        /// 22
        /// </summary>
        public virtual KeyModel KeyVK_P
        {
            get
            {
                if (vk_P == null)
                {
                    vk_P = new KeyModel(0x0070, 0x0050);
                }
                return vk_P;
            }
        }
        protected KeyModel vk_P;
        #endregion

        #region 23: VK_OemOpenBrackes (VK_OEM_4), [ + {
        /// <summary>
        /// 23  Left brackets
        /// </summary>
        public virtual KeyModel KeyVK_OemOpenBrackets
        {
            get
            {
                if (vk_OemOpenBrackets == null)
                {
                    vk_OemOpenBrackets = new KeyModelWithTwoGlyphs(0x005B, 0x007B, false);
                }
                return vk_OemOpenBrackets;
            }
        }
        protected KeyModel vk_OemOpenBrackets;
        #endregion

        #region 24: VK_OEM_6, ] + }
        /// <summary>
        /// 24  Right brackets
        /// </summary>
        public virtual KeyModel KeyVK_Oem6
        {
            get
            {
                if (vk_Oem6 == null)
                {
                    vk_Oem6 = new KeyModelWithTwoGlyphs(0x005D, 0x007D, false);
                }
                return vk_Oem6;
            }
        }
        protected KeyModel vk_Oem6;
        #endregion

        #region 25: KeyVK_Oem5
        /// <summary>
        /// 25
        /// </summary>
        public virtual KeyModel KeyVK_Oem5
        {
            get
            {
                if (vk_Oem5 == null)
                {
                    vk_Oem5 = new KeyModelWithTwoGlyphs(0x005C, 0x007C, false);
                }
                return vk_Oem5;
            }
        }
        protected KeyModel vk_Oem5;
        #endregion

        #region 26: KeyVK_A
        /// <summary>
        /// 26
        /// </summary>
        public virtual KeyModel KeyVK_A
        {
            get
            {
                if (vk_A == null)
                {
                    vk_A = new KeyModel(0x0061, 0x0041);
                }
                return vk_A;
            }
        }
        protected KeyModel vk_A;
        #endregion

        #region 27: KeyVK_S
        /// <summary>
        /// 27
        /// </summary>
        public virtual KeyModel KeyVK_S
        {
            get
            {
                if (vk_S == null)
                {
                    vk_S = new KeyModel(0x0073, 0x0053);
                }
                return vk_S;
            }
        }
        protected KeyModel vk_S;
        #endregion

        #region 28: KeyVK_D
        /// <summary>
        /// 28
        /// </summary>
        public virtual KeyModel KeyVK_D
        {
            get
            {
                if (vk_D == null)
                {
                    vk_D = new KeyModel(0x0064, 0x0044);
                }
                return vk_D;
            }
        }
        protected KeyModel vk_D;
        #endregion

        #region 29: KeyVK_F
        /// <summary>
        /// 29
        /// </summary>
        public virtual KeyModel KeyVK_F
        {
            get
            {
                if (vk_F == null)
                {
                    vk_F = new KeyModel(0x0066, 0x0046);
                }
                return vk_F;
            }
        }
        protected KeyModel vk_F;
        #endregion

        #region 30: KeyVK_G
        /// <summary>
        /// 30
        /// </summary>
        public virtual KeyModel KeyVK_G
        {
            get
            {
                if (vk_G == null)
                {
                    vk_G = new KeyModel(0x0067, 0x0047);
                }
                return vk_G;
            }
        }
        protected KeyModel vk_G;
        #endregion

        #region 31: KeyVK_H
        /// <summary>
        /// 31
        /// </summary>
        public virtual KeyModel KeyVK_H
        {
            get
            {
                if (vk_H == null)
                {
                    vk_H = new KeyModel(0x0068, 0x0048);
                }
                return vk_H;
            }
        }
        protected KeyModel vk_H;
        #endregion

        #region 32: KeyVK_J
        /// <summary>
        /// 32
        /// </summary>
        public virtual KeyModel KeyVK_J
        {
            get
            {
                if (vk_J == null)
                {
                    vk_J = new KeyModel(0x006A, 0x004A);
                }
                return vk_J;
            }
        }
        protected KeyModel vk_J;
        #endregion

        #region 33: KeyVK_K
        /// <summary>
        /// 33
        /// </summary>
        public virtual KeyModel KeyVK_K
        {
            get
            {
                if (vk_K == null)
                {
                    vk_K = new KeyModel(0x006B, 0x004B);
                }
                return vk_K;
            }
        }
        protected KeyModel vk_K;
        #endregion

        #region 34: KeyVK_L
        /// <summary>
        /// 34
        /// </summary>
        public virtual KeyModel KeyVK_L
        {
            get
            {
                if (vk_L == null)
                {
                    vk_L = new KeyModel(0x006C, 0x004C);
                }
                return vk_L;
            }
        }
        protected KeyModel vk_L;
        #endregion

        #region 35: KeyVK_Oem1 (VK_OEM_1)
        /// <summary>
        /// 35
        /// </summary>
        public virtual KeyModel KeyVK_Oem1
        {
            get
            {
                if (vk_Oem1 == null)
                {
                    vk_Oem1 = new KeyModelWithTwoGlyphs(0x003B, 0x003A, false);
                }
                return vk_Oem1;
            }
        }
        protected KeyModel vk_Oem1;
        #endregion

        #region 36: KeyVK_Oem7 (VK_OEM_7)
        /// <summary>
        /// 36  Apostrophe/Double-Quotation mark
        /// </summary>
        public virtual KeyModel KeyVK_Oem7
        {
            get
            {
                if (vk_Oem7 == null)
                {
                    vk_Oem7 = new KeyModelWithTwoGlyphs(0x0027, 0x0022, false);
                }
                return vk_Oem7;
            }
        }
        protected KeyModel vk_Oem7;
        #endregion

        #region 37: KeyVK_Z
        /// <summary>
        /// 37
        /// </summary>
        public virtual KeyModel KeyVK_Z
        {
            get
            {
                if (vk_Z == null)
                {
                    vk_Z = new KeyModel(0x007A, 0x005A);
                }
                return vk_Z;
            }
        }
        protected KeyModel vk_Z;
        #endregion

        #region 38: KeyVK_X
        /// <summary>
        /// 38
        /// </summary>
        public virtual KeyModel KeyVK_X
        {
            get
            {
                if (vk_X == null)
                {
                    vk_X = new KeyModel(0x0078, 0x0058);
                }
                return vk_X;
            }
        }
        protected KeyModel vk_X;
        #endregion

        #region 39: KeyVK_C
        /// <summary>
        /// 39
        /// </summary>
        public virtual KeyModel KeyVK_C
        {
            get
            {
                if (vk_C == null)
                {
                    vk_C = new KeyModel(0x0063, 0x0043);
                }
                return vk_C;
            }
        }
        protected KeyModel vk_C;
        #endregion

        #region 40: KeyVK_V
        /// <summary>
        /// 40
        /// </summary>
        public virtual KeyModel KeyVK_V
        {
            get
            {
                if (vk_V == null)
                {
                    vk_V = new KeyModel(0x0076, 0x0056);
                }
                return vk_V;
            }
        }
        protected KeyModel vk_V;
        #endregion

        #region 41: KeyVK_B
        /// <summary>
        /// 41
        /// </summary>
        public virtual KeyModel KeyVK_B
        {
            get
            {
                if (vk_B == null)
                {
                    vk_B = new KeyModel(0x0062, 0x0042);
                }
                return vk_B;
            }
        }
        protected KeyModel vk_B;
        #endregion

        #region 42: KeyVK_N
        /// <summary>
        /// 42
        /// </summary>
        public virtual KeyModel KeyVK_N
        {
            get
            {
                if (vk_N == null)
                {
                    vk_N = new KeyModel(0x006E, 0x004E);
                }
                return vk_N;
            }
        }
        protected KeyModel vk_N;
        #endregion

        #region 43: KeyVK_M
        /// <summary>
        /// 43
        /// </summary>
        public virtual KeyModel KeyVK_M
        {
            get
            {
                if (vk_M == null)
                {
                    vk_M = new KeyModel(0x006D, 0x004D);
                }
                return vk_M;
            }
        }
        protected KeyModel vk_M;
        #endregion

        #region 44: KeyVK_OemComma
        /// <summary>
        /// 44  Commas / Less-than sign
        /// </summary>
        public virtual KeyModel KeyVK_OemComma
        {
            get
            {
                if (vk_OemComma == null)
                {
                    vk_OemComma = new KeyModelWithTwoGlyphs(0x002C, 0x003C, false);
                }
                return vk_OemComma;
            }
        }
        protected KeyModel vk_OemComma;
        #endregion

        #region 45: KeyVK_OemPeriod (VK_OEM_PERIOD)
        /// <summary>
        /// 45  Period / Greater-than sign
        /// </summary>
        public virtual KeyModel KeyVK_OemPeriod
        {
            get
            {
                if (vk_OemPeriod == null)
                {
                    vk_OemPeriod = new KeyModelWithTwoGlyphs(0x002E, 0x003E, false);
                }
                return vk_OemPeriod;
            }
        }
        protected KeyModel vk_OemPeriod;
        #endregion

        #region 46: KeyVK_OemQuestion (VK_OEM_2)
        /// <summary>
        /// 46  Solidus
        /// </summary>
        public virtual KeyModel KeyVK_OemQuestion
        {
            get
            {
                if (vk_OemQuestion == null)
                {
                    vk_OemQuestion = new KeyModel(0x002F, false);
                }
                return vk_OemQuestion;
            }
        }
        protected KeyModel vk_OemQuestion;
        #endregion

        #endregion The individual key assignments
    }
}