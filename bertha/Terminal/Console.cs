// © Copyright(C) Microsoft. All rights reserved.

using System;
using System.Diagnostics;
using Windows.Foundation;

namespace bertha
{
    class Console
    {
        private int _iCursorX=0;
        private int _iCursorY=0;
        private char [,] Display;
        private char[,] PrevDisplayFrame;
        private int iCursorTick=0;

		private bool bHaveEscape = false;
		private bool bHaveBracket = false;
		private bool bInNumber = false;
		private bool bSecondNumber = false;
        private int iFirstNumber = 0;
        private int iTempNumber = 0;
        private bool _bDirty = false;
        
        private int _iCellWidth=0;
        private int _iCellHeight=0;

        private string _sCurrentLine=string.Empty;
        public string CurrentLine
        {
            get { return _sCurrentLine; }
        }

        public bool bDirty
        {
            get { return _bDirty; }
            set { _bDirty = value;  }
        }

        public int iCursorX
        {
            get { return _iCursorX; }
        }
        public int iCursorY
        {
            get { return _iCursorY; }
        }

        private bool _bCursorVisible = false;
        public bool bCursorVisible
        {
            get { return _bCursorVisible; }
        }

        public int CellWidth 
        {
            get { return _iCellWidth; }
        }

        public int CellHeight
        {
            get { return _iCellHeight; }
        }
        public Console()
        {
            _iCursorX = 0;
            _iCursorY = 0;
            Display=new char[80,25];
            // prev display frame is used to determine whether a character needs to be displayed.
            PrevDisplayFrame = new char[80, 25];

            for (int x = 0; x < 80; x++)
            {
                for (int y = 0; y < 25; y++)
                {
                    Display[x, y] = ' ';
                    PrevDisplayFrame[x, y] = ' ';
                }
            }
        }

        public bool IsDisplayCellInvalid(int x, int y)
        {
            bool bRet=false;
            if (PrevDisplayFrame[x,y] != Display[x,y])
            {
                bRet = true;
                PrevDisplayFrame[x, y] = Display[x, y];
            }
        return bRet;
        }

        public void Tick()
        {
//            Debug.WriteLine(string.Format("Tick: {0}", iCursorTick.ToString()));
            iCursorTick++;
            if (iCursorTick == 20)
            {
                iCursorTick = 0;
                if (_bCursorVisible)
                {
//                    Debug.WriteLine("Cursor Hidden");                    
                    _bCursorVisible = false;
                }
                else
                {
//                    Debug.WriteLine("Cursor Visible");
                    _bCursorVisible = true;
                }
            }
        }

        public void SetSize(Size size)
        {
            Debug.WriteLine("SetChar");
            _iCellWidth=(int)size.Width/80;
            _iCellHeight=(int)size.Height/25;
        }

        void SetChar(char c)
        {
            Debug.WriteLine("SetChar");
            //	RestoreCharAtCursorPos();
	        Display[_iCursorX,_iCursorY] = c;
	        _iCursorX++;
            if (_iCursorX == 80)
            {
                NewLine();
            }
            //	SaveCharAtCursorPos();
            //	SetDisplayCursor('#');
        }

        void DisplayChar(char c)
        {
            Debug.WriteLine(string.Format("DisplayChar {0}", c));
	        SetChar(c);
        //	ShowDisplay();
        //	if (iTerminalXpos > 80)
        //		printf("Line > 80 chars %d\n", iTerminalXpos);
        }

        public void ParseTelnetChar(char c)
        {
            Debug.WriteLine("ParseTelnetChar");
	        if (0x1b != c && bHaveEscape == false && c >= 0x20)
	        {
                DisplayChar(c);
	        }

	        if (bHaveEscape && bHaveBracket && Char.IsDigit(c))
	        {
		        bInNumber = true;
		        iTempNumber = (iTempNumber * 10) + c - '0';
	        }

	        if (bInNumber && ';' == c)
	        {
		        iFirstNumber = iTempNumber;
		        iTempNumber = 0;
		        bInNumber = false;
		        bSecondNumber = true;
	        }

	        if (bInNumber && 'H' == c)
	        {
		        // could be cursor position on line or absolute cursor position
		        if (bSecondNumber)
		        {
			        // iTempNumber == x, iFirstNumber == y
                    CursorPosition(iTempNumber, iFirstNumber);
		        }
		        else
		        {
			        // need to pass '1' as x since we decrement the numbers
			        // VT100 positioning appears to be '1' based
			        // the terminal is '0' based.
			        CursorPosition(1, iTempNumber);
		        }

		        bHaveEscape = false;
		        bHaveBracket = false;
		        bInNumber = false;
		        iTempNumber = 0;
		        iFirstNumber = 0;
		        bSecondNumber = false;
	        }

	        if (bHaveEscape && bHaveBracket && 'K' == c)
	        {
		        bHaveEscape = false;
		        bHaveBracket = false;
		        ClearLine();
	        }

	        if (0x1b == c)
	        {
		        bHaveEscape = true;
	        }

	        if (bHaveEscape && '[' == c)
	        {
		        bHaveBracket = true;
	        }

	        if (!bHaveEscape && 0x0d == c)
	        {
		        CarriageReturn();
	        }

	        if (!bHaveEscape && 0x0a == c)
	        {
		        NewLine();
	        }
        }
        
        public void ScrollDisplay()
        {
            Debug.WriteLine("ScrollDisplay");
            _bDirty = true;
        //	RestoreCharAtCursorPos();
	        for (int y = 1; y < 25; y++)
	        {
		        for (int x = 0; x < 80; x++)
		        {
			        Display[x,y - 1] = Display[x,y];
		        }
	        }
	        for (int x = 0; x < 80; x++)
		        Display[x,24] = ' ';

	        _iCursorY = 24;
        //	SaveCharAtCursorPos();
        }

        public void ClearDisplay()
        {
            Debug.WriteLine("ClearDisplay");
            _bDirty = true;
            //	RestoreCharAtCursorPos();
	        for (int y = 0; y < 25; y++)
	        {
		        for (int x = 0; x < 80; x++)
		        {
			        Display[x,y] = ' ';
		        }
	        }
	        _iCursorX = 0;
	        _iCursorY= 0;
        //	SaveCharAtCursorPos();
        //	SetDisplayCursor('#');
        //	ShowDisplay();
        }

        public void NewLine()
        {
            Debug.WriteLine("NewLine");
            _bDirty = true;
            //	RestoreCharAtCursorPos();
	        _iCursorY++;
            _iCursorX = 0;
	        if (_iCursorY== 25)
		        ScrollDisplay();

            _sCurrentLine = string.Empty;
        //	ShowDisplay();
        //	SaveCharAtCursorPos();
        }

        public void CarriageReturn()
        {
            Debug.WriteLine("CarriageReturn");
            _bDirty = true;
            //	RestoreCharAtCursorPos();
	        _iCursorX = 0;
        //	SaveCharAtCursorPos();
        //	ShowDisplay();
        }

        public void CursorPosition(int x, int y)
        {
            Debug.WriteLine(string.Format("Cursor Position x:{0} y:{1}", x, y));
            _bDirty = true;
            //	RestoreCharAtCursorPos();
	        _iCursorX = x - 1;
	        _iCursorY = y - 1;
        //	SaveCharAtCursorPos();
        //	SetDisplayCursor('#');
        //	ShowDisplay();
        }

        public void ClearLine()
        {
            Debug.WriteLine("ClearLine");
            _bDirty = true;
            //	RestoreCharAtCursorPos();
	        for (int x = _iCursorX; x < 80; x++)
	        {
		        Display[x,_iCursorY] = ' ';
	        }
        //	SaveCharAtCursorPos();
        //	SetDisplayCursor('#');
        //	ShowDisplay();
        }

        public void WriteChar(char c)
        {
            Debug.WriteLine("WriteChar");
            if (c >= 0x20 && c <= 0x7f)
            {
                DisplayChar(c);
                _sCurrentLine += c;
            }
            if (0x08 == c)
            {
                if (_sCurrentLine != string.Empty)
                {
                    if (_sCurrentLine.Length > 0)
                    {
                        _sCurrentLine = _sCurrentLine.Substring(0, _sCurrentLine.Length - 1);
                        if (_iCursorX > 0)
                        {
                            _iCursorX--;
                            Display[_iCursorX,_iCursorY] = ' ';
                        }
                        else
                        {
                            if (_iCursorY > 0)
                            {
                                _iCursorY--;
                                _iCursorX = 79;
                                Display[_iCursorX, _iCursorY] = ' ';
                            }
                        }
                    }
                }
            }
        }

        public void WriteLine(string sString)
        {
            Write(sString);
            NewLine();
        }

        public void WriteLine()
        {
            NewLine();
        }

        public void Write(string sString)
        {
            Debug.WriteLine("Write");
            _sCurrentLine += sString;
            _bDirty = true;
            foreach (char c in sString)
            {
                DisplayChar(c);
            }
        }

        public char GetCharAtDisplayPos(int x, int y)
        {
            return Display[x, y];
        }
    }
}
