// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpDude
{
    [Flags]
    enum ArduinoFlags
    {
        None = 0,
        RequiresReset = 1
    };

    public interface IArduino
    {        
        string BoardName { get; }
        string AvrDudeName { get; }

        string Programmer { get; }

        int BaudRate { get; }

        string ImageName { get; }

        string Version { get; }

        bool RequiresReset { get; }
    }
    class Arduino: IArduino
    {
        public Arduino(string boardName, string avrDudeName, string programmer, int baudRate, ArduinoFlags flags, string imageName, string version)
        {
            if (string.IsNullOrWhiteSpace(boardName))
                throw new ArgumentNullException(nameof(boardName));

            if (string.IsNullOrWhiteSpace(avrDudeName))
                throw new ArgumentNullException(nameof(avrDudeName));

            if (string.IsNullOrWhiteSpace(programmer))
                throw new ArgumentNullException(nameof(programmer));

            if (string.IsNullOrWhiteSpace(imageName))
                throw new ArgumentNullException(nameof(imageName));

            if (string.IsNullOrWhiteSpace(version))
                throw new ArgumentNullException(nameof(version));

            BoardName = boardName;
            AvrDudeName = avrDudeName;
            Programmer = programmer;
            BaudRate = baudRate;
            ImageName = imageName;
            Version = version;
            
            RequiresReset = (flags & ArduinoFlags.RequiresReset) != 0;
        }

        public override string ToString()
        {
            return BoardName;
        }

        public string BoardName { get; }
        public string AvrDudeName { get; }

        public string Programmer { get; }

        public int BaudRate { get; }

        public string ImageName { get; }

        public string Version { get; }

        public bool RequiresReset { get; }
    }    

    static class ArduinoService
    {
        struct BoardInfo
        {
            public BoardInfo(string name, string avrDudeName, string programmer, int baudRate, ArduinoFlags flags, string devPath)
            {
                Name = name;
                AvrDudeName = avrDudeName;
                Programmer = programmer;
                BaudRate = baudRate;
                DevelopmentPath = devPath;
                Flags = flags;
            }        

            public readonly string Name;
            public readonly string AvrDudeName;
            public readonly string Programmer;
            public readonly int BaudRate;
            public readonly ArduinoFlags Flags;

            public readonly string DevelopmentPath;
        }

        private static BoardInfo[] gBoards = 
        {
            new BoardInfo("mega2560", "atmega2560", "wiring", 115200, ArduinoFlags.None, @"megaatmega2560\firmware.hex"),
            new BoardInfo("uno", "atmega328p", "arduino", 115200, ArduinoFlags.None, @"uno\firmware.hex"),
            new BoardInfo("leonardo", "atmega32u4", "avr109", 57600, ArduinoFlags.RequiresReset, @"leonardo\firmware.hex")
        };

        static private void DevelopmentFix()
        {
            var myExeLocation = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetEntryAssembly().Location);

            var pioBase = System.IO.Path.Combine(myExeLocation, @"..\..\..\src\LiteDecoder\.pio\build");

            foreach (var board in gBoards)
            {
                var devImgPath = System.IO.Path.Combine(pioBase, board.DevelopmentPath);
                if (!System.IO.File.Exists(devImgPath))
                    continue;

                var devDest = System.IO.Path.Combine(myExeLocation, "dcclite.firmware." + board.Name + ".vx.x.x.hex");
                if (!System.IO.File.Exists(devDest))
                    System.IO.File.Copy(devImgPath, devDest);
            }
        }

        static public IArduino[] GetAvailableConfigs()
        {
            /*
             * dcclite.firmware.mega2560.v0.3.0.hex
             *
             *
            */            

            DevelopmentFix();

            string imageFilesMask = "dcclite.firmware.*.hex";            

            var myExeLocation = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetEntryAssembly().Location);

            var arduinoList = new List<IArduino>();
            foreach (var imgFileLocation in System.IO.Directory.EnumerateFiles(myExeLocation, imageFilesMask))
            {
                var imgFileName = System.IO.Path.GetFileName(imgFileLocation);

                var parts = imgFileName.Replace("dcclite.firmware.", "").Split('.');
                if (parts.Length < 4)
                    continue;

                var arduinoName = parts[0];

                BoardInfo? tmp = gBoards.Where(x => x.Name == arduinoName).FirstOrDefault();
                if (tmp == null)
                    continue;

                BoardInfo boardInfo = (BoardInfo)tmp;

                string version = parts[1] + '.' + parts[2] + '.' + parts[3];


                arduinoList.Add(new Arduino(
                    boardInfo.Name, 
                    boardInfo.AvrDudeName, 
                    boardInfo.Programmer, 
                    boardInfo.BaudRate, 
                    boardInfo.Flags, 
                    imgFileLocation, 
                    version
                ));
            }

            return arduinoList.ToArray();
        }
    }
}
