// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Gpio;

namespace NotepadService
{
    internal class Notepad
    {
        List<string> notes;
        bool newNotes;
        GpioPin newNotesPin;

        public Notepad()
        {
            notes = new List<string>();
            newNotes = false;          
        }

        public async Task InitGpioPin()
        {
            var controller = await GpioController.GetDefaultAsync();
            newNotesPin = controller.OpenPin(5);
            newNotesPin.Write(GpioPinValue.Low);
            newNotesPin.SetDriveMode(GpioPinDriveMode.Output);      
        }

        public async Task AddNote(string note)
        {
            if (newNotesPin == null)
            {
                await InitGpioPin();
            }
            notes.Add(note);
            NewNotesAvailable = true;
            if (NoteAdded != null)
            {
                NoteAdded.Invoke(this,new EventArgs());
            }
        }

        public bool NewNotesAvailable
        {
            get
            {
                return newNotes;
            }
            set
            {
                bool changed = value != newNotes;
                newNotes = value;
                if (newNotes)
                {
                    newNotesPin.Write(GpioPinValue.High);
                }
                else
                {
                    newNotesPin.Write(GpioPinValue.Low);
                }
                if (changed && NewNoteStateChanged!= null)
                {
                    NewNoteStateChanged.Invoke(this, new EventArgs());
                }
                
            }
        }


        public List<string> GetNotes()
        {
            NewNotesAvailable = false;
            return notes;
        }

        public event EventHandler NoteAdded;
        public event EventHandler NewNoteStateChanged;
    }
}
