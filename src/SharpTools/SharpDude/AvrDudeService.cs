// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

namespace SharpDude
{
    //./avrdude -v -p atmega2560 -C "..\etc\avrdude.conf" -c wiring -b 115200 -D -P "COM4" -U flash:w:"F:\develop\bcs\DCCLite\src\LiteDecoder\.pio\build\megaatmega2560\firmware.hex":i

    struct AvrDudeInfo
    {
        public AvrDudeInfo(string path, string confPath)
        {
            if (string.IsNullOrWhiteSpace(path))
                throw new ArgumentNullException(nameof(path));

            if (string.IsNullOrWhiteSpace(confPath))
                throw new ArgumentNullException(nameof(confPath));

            Path = path;
            ConfPath = confPath;
        }        

        public string Path { get; }
        public string ConfPath { get; }
    }

    public interface IAvrDude
    {
        string GetName();

        string GetConfPath();
    }

    class AvrDude: IAvrDude
    {
        private readonly AvrDudeInfo mInfo;

        public AvrDude(AvrDudeInfo info)
        {
            mInfo = info;
        }

        public string GetName()
        {
            return mInfo.Path;
        }

        public string GetConfPath()
        {
            return mInfo.ConfPath;
        }
    }


    static class AvrDudeService
    {
        static public IAvrDude Create()
        {
            var arduinoAvrRootPath = System.Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86);
            arduinoAvrRootPath = System.IO.Path.Combine(arduinoAvrRootPath, "Arduino\\hardware\\tools\\avr");

            if(!Path.Exists(arduinoAvrRootPath))
            {
                arduinoAvrRootPath = System.Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
                var dirEnum = Directory.EnumerateDirectories(arduinoAvrRootPath, "Arduino??").GetEnumerator();

                while(dirEnum.MoveNext())
                {					
					arduinoAvrRootPath = System.IO.Path.Combine(dirEnum.Current.ToString(), "packages\\arduino\\tools\\avrdude");

                    break;
				}
				
                if(Path.Exists(arduinoAvrRootPath))
                {
					var dudeDirEnum = Directory.EnumerateDirectories(arduinoAvrRootPath, "*arduino*").GetEnumerator();

                    while(dudeDirEnum.MoveNext())
                    {
						arduinoAvrRootPath = dudeDirEnum.Current.ToString();
                        break;
					}
				}
			}

            var arduinoAvrExePath = System.IO.Path.Combine(arduinoAvrRootPath, "bin\\avrdude.exe");

            if(!System.IO.File.Exists(arduinoAvrExePath))
            {
                throw new InvalidOperationException("Cannot find avrdude.exe. Is Arduino IDE installed?");
            }

            var arduinoConfPath = System.IO.Path.Combine(arduinoAvrRootPath, "etc\\avrdude.conf");
            if(!System.IO.File.Exists(arduinoConfPath))
            {
                throw new InvalidOperationException("Cannot find avrdude.conf. But, avrdude.exe found, which Arduino IDe version are you using? Please let us know!");
            }

            return new AvrDude(new AvrDudeInfo(arduinoAvrExePath, arduinoConfPath));
        }
    }
}
