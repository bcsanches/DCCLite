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
using System.IO;

namespace SharpEEPromViewer
{
    class Lump
    {
        static Dictionary<string, Type> gKnownTypes = new()
        {
            { "Bcs0008\0", typeof(RootLump) },
            { "NetU002\0", typeof(NetworkLump) },
            { "Sson001\0", typeof(SessionLump) },
            { "DECS012\0", typeof(DecodersLump) },
            { "ENDEND1\0", typeof(MarkerLump) }
        };

        public readonly string Name;
        public readonly UInt16 Size;

        public List<Lump> mChildren;

        public IEnumerable<Lump> Children { get { return mChildren; } }

        public const int LUMP_HEADER_SIZE = 10;

        protected Lump(string name, UInt16 size)
        {
            Name = name ?? throw new ArgumentNullException(nameof(name));
            Size = size;
        }

        protected void AddChild(Lump lump)
        {
            if (mChildren == null)
                mChildren = new List<Lump>();

            mChildren.Add(lump);
        }

        public static Lump Create(BinaryReader reader)
        {
            var headerBytes = reader.ReadChars(8);
            var name = new String(headerBytes);

            Type foundType;
            if (!gKnownTypes.TryGetValue(name, out foundType))
                foundType = typeof(UnknownLump);

            var size = reader.ReadUInt16();

            return (Lump)System.Activator.CreateInstance(foundType, name, size, reader);
        }

        protected static string ConvertBytesToString(byte[] data)
        {
            int zeroPos;

            for (zeroPos = 0; zeroPos < data.Length; zeroPos++)
            {
                if (data[zeroPos] == 0)
                    break;
            }

            return System.Text.Encoding.ASCII.GetString(data, 0, zeroPos);
        }
    };

    class UnknownLump : Lump
    {
        public UnknownLump(string name, UInt16 size, BinaryReader reader) :
            base(name, size)
        {
            //https://stackoverflow.com/questions/2519864/recommendations-for-a-hex-viewer-control-for-windows-forms

            //nothing todo... unknown....

            //skip...
            reader.ReadBytes(size);
        }

    }

    class NetworkLump : Lump
    {
        public readonly string NodeName;
        public readonly System.Net.NetworkInformation.PhysicalAddress Mac;
        public readonly UInt16 SrcPort;

        public NetworkLump(string name, UInt16 size, BinaryReader reader) :
            base(name, size)
        {
            if (size != 24)
                throw new ArgumentOutOfRangeException("[NetworkLump] Expected size to be 24 bytes, but got " + size);

            const int MAX_NODE_NAME = 16;

            NodeName = Lump.ConvertBytesToString(reader.ReadBytes(MAX_NODE_NAME));
            Mac = new(reader.ReadBytes(6));
            SrcPort = reader.ReadUInt16();
        }
    }

    class SessionLump : Lump
    {
        public readonly System.Net.IPAddress IP;
        public readonly UInt16 ServerPort;

        public SessionLump(string name, UInt16 size, BinaryReader reader) :
            base(name, size)
        {
            if (size != 6)
                throw new ArgumentOutOfRangeException("[SessionLump] Expected size to be 6 bytes, but got " + size);

            IP = new(reader.ReadBytes(4));
            ServerPort = reader.ReadUInt16();
        }
    }

    class DecodersLump : Lump
    {
        public readonly Guid Guid;

        public DecodersLump(string name, UInt16 size, BinaryReader reader) :
            base(name, size)
        {
            if (size < 17)
                throw new ArgumentOutOfRangeException("[DecodersLump] Must have at least 17 bytes, but got " + size);

            Guid = new Guid(reader.ReadBytes(16));

            //skip...
            reader.ReadBytes(size - 16);
        }
    }

    class MarkerLump: Lump
    {
        public MarkerLump(string name, UInt16 size, BinaryReader reader):
            base(name, size)
        {
            if (size > 0)
                throw new ArgumentOutOfRangeException("[MarkerLump] markers should be empty, but size is " + size);

            //nothing todo, empty lump
        }
    }

    class RootLump: Lump
    {
        public RootLump(string name, UInt16 size, BinaryReader reader) :
            base(name, size)
        {
            //empty lump?
            if (Size == 0)
                return;

            int bytesLeft = Size;

            while(bytesLeft > 0)
            {
                var lump = Create(reader);

                this.AddChild(lump);

                bytesLeft -= lump.Size + Lump.LUMP_HEADER_SIZE;
            }            
        }        
    }
}
