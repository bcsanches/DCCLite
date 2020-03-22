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
    public interface IArduino
    {
        string BoardName { get; }
        string AvrDudeName { get; }

        string ImageName { get; }

        string Version { get; }
    }
    class Arduino: IArduino
    {
        public Arduino(string boardName, string avrDudeName, string imageName, string version)
        {
            if (string.IsNullOrWhiteSpace(boardName))
                throw new ArgumentNullException(nameof(boardName));

            if (string.IsNullOrWhiteSpace(avrDudeName))
                throw new ArgumentNullException(nameof(avrDudeName));

            if (string.IsNullOrWhiteSpace(imageName))
                throw new ArgumentNullException(nameof(imageName));

            if (string.IsNullOrWhiteSpace(version))
                throw new ArgumentNullException(nameof(version));

            BoardName = boardName;
            AvrDudeName = avrDudeName;
            ImageName = imageName;
            Version = version;
        }

        public override string ToString()
        {
            return BoardName;
        }

        public string BoardName { get; }
        public string AvrDudeName { get; }

        public string ImageName { get; }

        public string Version { get; }
    }    

    static class ArduinoService
    {
        struct BoardInfo
        {
            public BoardInfo(string name, string avrDudeName, string devPath)
            {
                Name = name;
                AvrDudeName = avrDudeName;
                DevelopmentPath = devPath;
            }        

            public readonly string Name;
            public readonly string AvrDudeName;

            public readonly string DevelopmentPath;
        }

        private static BoardInfo[] gBoards = 
        {
            new BoardInfo("mega2560", "atmega2560", @"megaatmega2560\firmware.hex"),
            new BoardInfo("uno", "atmega328p", @"uno\firmware.hex")
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

                BoardInfo? boardInfo = gBoards.Where(x => x.Name == arduinoName).FirstOrDefault();
                if (boardInfo == null)
                    continue;

                string version = parts[1] + '.' + parts[2] + '.' + parts[3];


                arduinoList.Add(new Arduino(boardInfo?.Name, boardInfo?.AvrDudeName, imgFileLocation, version));
            }

            return arduinoList.ToArray();
        }
    }
}
