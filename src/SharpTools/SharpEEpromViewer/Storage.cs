﻿// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
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
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;

namespace SharpEEPromViewer
{
	class Item
	{
		[Category("Information")]
		public bool Deprecated { get; }

		[Category("Information")]
		public byte Slot { get; }

		[Category("Information")]
		public UInt16 Size { get; }
		

		protected Item(byte slot, UInt16 size, bool deprecated = false)            
		{
			Size = size;
			Slot = slot;
			Deprecated = deprecated;
		}
	}

	class Decoder: Item
	{
		public byte Pin { get; }

		protected Decoder(byte slot, UInt16 size, byte pin):
			base(slot, size)
		{
			Pin = pin;
		}
	}

	class OutputDecoder : Decoder
	{
		public bool IsInverted { get; }
		public bool ShouldIgnoreSaveState { get; }

		public bool ShouldActivateOnPowerUp { get; }

		[Flags]
		enum Flags : byte
		{
			OUTD_INVERTED_OPERATION = 0x01,
			OUTD_IGNORE_SAVED_STATE = 0x02,
			OUTD_ACTIVATE_ON_POWER_UP = 0x04
		}

		public OutputDecoder(byte slot, BinaryReader reader):
			base(slot, 2, reader.ReadByte())
		{
			var flags = (Flags)reader.ReadByte();

			IsInverted = (flags & Flags.OUTD_INVERTED_OPERATION) == Flags.OUTD_INVERTED_OPERATION;
			ShouldIgnoreSaveState = (flags & Flags.OUTD_IGNORE_SAVED_STATE) == Flags.OUTD_IGNORE_SAVED_STATE;
			ShouldActivateOnPowerUp = (flags & Flags.OUTD_ACTIVATE_ON_POWER_UP) == Flags.OUTD_ACTIVATE_ON_POWER_UP;
		}
	}

	class SensorDecoderV017 : Decoder
	{

		public bool HasPullUp { get; }
		public bool IsInverted { get; }

		[Description("Delay to send activate event in seconds")]
		public byte ActivateDelay { get; }

		[Description("Delay to send deactivate event in seconds")]
		public byte DeactivateDelay { get; }

		[Flags]
		enum Flags : byte
		{
			SNRD_PULL_UP = 0x01,
			SNRD_INVERTED = 0x02
		}

		public SensorDecoderV017(byte slot, BinaryReader reader) :
			base(slot, 4, reader.ReadByte())
		{
			Flags flags = (Flags)reader.ReadByte();

			HasPullUp = (flags & Flags.SNRD_PULL_UP) == Flags.SNRD_PULL_UP;
			IsInverted = (flags & Flags.SNRD_INVERTED) == Flags.SNRD_INVERTED;

			ActivateDelay = reader.ReadByte();
			DeactivateDelay = reader.ReadByte();
		}
	}

	class SensorDecoderV019 : Decoder
	{

		public bool HasPullUp { get; }
		public bool IsInverted { get; }

		[Description("Delay to send activate event in milliseconds")]
		public ushort ActivateDelayMs { get; }

		[Description("Delay to send deactivate event in milliseconds")]
		public ushort DeactivateDelayMs { get; }

		[Flags]
		enum Flags : byte
		{
			SNRD_PULL_UP = 0x01,
			SNRD_INVERTED = 0x02
		}

		public SensorDecoderV019(byte slot, BinaryReader reader) :
			base(slot, 6, reader.ReadByte())
		{
			Flags flags = (Flags)reader.ReadByte();

			HasPullUp = (flags & Flags.SNRD_PULL_UP) == Flags.SNRD_PULL_UP;
			IsInverted = (flags & Flags.SNRD_INVERTED) == Flags.SNRD_INVERTED;

			ActivateDelayMs = reader.ReadUInt16();
			DeactivateDelayMs = reader.ReadUInt16();
		}
	}

	class SensorDecoder: Decoder
	{

		public bool HasPullUp { get; }
		public bool IsInverted { get; }

		[Description("Delay to send activate event in milliseconds")]
		public ushort ActivateDelayMs { get; }

		[Description("Delay to send deactivate event in milliseconds")]
		public ushort DeactivateDelayMs { get; }

		[Description("Delay to start watching pin in milliseconds")]
		public ushort StartDelayMs { get; }

		[Flags]
		enum Flags: byte
		{
			SNRD_PULL_UP = 0x01,
			SNRD_INVERTED = 0x02
		}

		public SensorDecoder(byte slot, BinaryReader reader):
			base(slot, 8, reader.ReadByte())
		{
			Flags flags = (Flags)reader.ReadByte();

			HasPullUp = (flags & Flags.SNRD_PULL_UP) == Flags.SNRD_PULL_UP;
			IsInverted = (flags & Flags.SNRD_INVERTED) == Flags.SNRD_INVERTED;

			ActivateDelayMs = reader.ReadUInt16();
			DeactivateDelayMs = reader.ReadUInt16();
			StartDelayMs = reader.ReadUInt16();
		}
	}

	class ServoTurnoutDecoder: Decoder
	{
		[Flags]
		enum Flags: byte
		{
			SRVT_INVERTED_OPERATION = 0x04,
			SRVT_IGNORE_SAVED_STATE = 0x08,
			SRVT_ACTIVATE_ON_POWER_UP = 0x10,
			SRVT_INVERTED_FROG = 0x20,
			SRVT_INVERTED_POWER = 0x40
		}

		public bool IsInverted { get; }
		public bool ShouldIgnoreSaveState { get; }
		public bool ShouldActivateOnPowerUp { get; }

		public bool IsFrogInverted { get; }
		public bool IsPowerInverted { get; }

		public byte? PowerPin { get; }
		public byte? FrogPin { get; }

		public byte StartPos { get; }

		public byte EndPos { get; }

		[Description("Delay in milliseconds between each degree")]
		public byte Ticks { get; }

		[Description("Calculated: total range in degrees")]
		public int Range { get; }

		[Description("Calculated: total movement time (in milliseconds)")]
		public int TotalTime { get; }

		private static byte? ReadOptionalPin(BinaryReader reader)
		{
			var v = reader.ReadByte();

			//128 indicates null pin
			return v == 128 ? null : v;
		}

		public ServoTurnoutDecoder(byte slot, BinaryReader reader):
			base(slot, 7, reader.ReadByte())
		{
			var flags = (Flags)reader.ReadByte();

			IsInverted = (flags & Flags.SRVT_INVERTED_OPERATION) == Flags.SRVT_INVERTED_OPERATION;
			ShouldIgnoreSaveState = (flags & Flags.SRVT_IGNORE_SAVED_STATE) == Flags.SRVT_IGNORE_SAVED_STATE;
			ShouldActivateOnPowerUp = (flags & Flags.SRVT_ACTIVATE_ON_POWER_UP) == Flags.SRVT_ACTIVATE_ON_POWER_UP;
			IsFrogInverted = (flags & Flags.SRVT_INVERTED_FROG) == Flags.SRVT_INVERTED_FROG;
			IsPowerInverted = (flags & Flags.SRVT_INVERTED_POWER) == Flags.SRVT_INVERTED_POWER;

			PowerPin = ReadOptionalPin(reader);
			FrogPin = ReadOptionalPin(reader);
			StartPos = reader.ReadByte();
			EndPos = reader.ReadByte();
			Ticks = reader.ReadByte();

			Range = EndPos - StartPos;
			TotalTime = Range * Ticks;
		}
	}

	class TurntableAutoInverterDecoderV016 : Item
	{
		[Flags]
		enum Flags : byte
		{
			TRTD_REMOTE_ACTIVE = 0x40,
			TRTD_ACTIVE = 0x80
		}

		public bool IsActive { get; }

		public byte SensorAIndex { get; }
		public byte SensorBIndex { get; }

		public byte TrackAPin0 { get; }
		public byte TrackAPin1 { get; }
		public byte TrackBPin0 { get; }
		public byte TrackBPin1 { get; }

		public TurntableAutoInverterDecoderV016(byte slot, BinaryReader reader) :
			base(slot, 7, true)
		{
			var flags = (Flags)reader.ReadByte();

			IsActive = (flags & Flags.TRTD_ACTIVE) == Flags.TRTD_ACTIVE;

			SensorAIndex = reader.ReadByte();
			SensorBIndex = reader.ReadByte();

			TrackAPin0 = reader.ReadByte();
			TrackAPin1 = reader.ReadByte();
			TrackBPin0 = reader.ReadByte();
			TrackBPin1 = reader.ReadByte();
		}
	}

	class TurntableAutoInverterDecoderV018: Item
	{
		[Flags]
		enum Flags : byte
		{
			TRTD_REMOTE_ACTIVE = 0x40,
			TRTD_ACTIVE = 0x80            
		}

		public bool IsActive { get; }

		public byte FlipInterval { get; }

		public byte SensorAIndex { get; }
		public byte SensorBIndex { get; }

		public byte TrackAPin0 { get; }
		public byte TrackAPin1 { get; }
		public byte TrackBPin0 { get; }
		public byte TrackBPin1 { get; }

		public TurntableAutoInverterDecoderV018(byte slot, BinaryReader reader):
			base(slot, 8)
		{
			var flags = (Flags)reader.ReadByte();

			IsActive = (flags & Flags.TRTD_ACTIVE) == Flags.TRTD_ACTIVE;

			FlipInterval = reader.ReadByte();

			SensorAIndex = reader.ReadByte();
			SensorBIndex = reader.ReadByte();

			TrackAPin0 = reader.ReadByte();
			TrackAPin1 = reader.ReadByte();
			TrackBPin0 = reader.ReadByte();
			TrackBPin1 = reader.ReadByte();
		}
	}

	class TurntableAutoInverterDecoderV019 : Item
	{
		[Flags]
		enum Flags : byte
		{
			TRTD_REMOTE_ACTIVE = 0x40,
			TRTD_ACTIVE = 0x80
		}

		public bool IsActive { get; }

		public byte FlipInterval { get; }

		public byte SensorAIndex { get; }
		public byte SensorBIndex { get; }

		public byte TrackAPin { get; }
		public byte TrackBPin { get; }

		public TurntableAutoInverterDecoderV019(byte slot, BinaryReader reader) :
			base(slot, 6)
		{
			var flags = (Flags)reader.ReadByte();

			IsActive = (flags & Flags.TRTD_ACTIVE) == Flags.TRTD_ACTIVE;

			FlipInterval = reader.ReadByte();

			SensorAIndex = reader.ReadByte();
			SensorBIndex = reader.ReadByte();

			TrackAPin = reader.ReadByte();
			TrackBPin = reader.ReadByte();
		}
	}

	class TurntableAutoInverterDecoder : Item
	{
		[Flags]
		enum Flags : byte
		{
			TRTD_INVERTED = 0x01,
			TRTD_REMOTE_ACTIVE = 0x40,
			TRTD_ACTIVE = 0x80
		}

		public bool IsActive { get; }

		public bool IsInverted { get; }

		public byte FlipInterval { get; }

		public byte SensorAIndex { get; }
		public byte SensorBIndex { get; }

		public byte TrackAPin { get; }		
		public byte TrackBPin { get; }		

		public TurntableAutoInverterDecoder(byte slot, BinaryReader reader) :
			base(slot, 6)
		{
			var flags = (Flags)reader.ReadByte();

			IsActive = (flags & Flags.TRTD_ACTIVE) == Flags.TRTD_ACTIVE;
			IsInverted = (flags & Flags.TRTD_INVERTED) == Flags.TRTD_INVERTED;

			FlipInterval = reader.ReadByte();

			SensorAIndex = reader.ReadByte();
			SensorBIndex = reader.ReadByte();

			TrackAPin = reader.ReadByte();
			TrackBPin = reader.ReadByte();
		}
	}

	class QuadInverterDecoderV016 : Item
	{
		[Flags]
		enum Flags : byte
		{
			QUAD_IGNORE_SAVED_STATE = 0x02,
			QUAD_ACTIVATE_ON_POWER_UP = 0x04,
			QUAD_ACTIVE = 0x80
		}

		public bool IsActive { get; }

		public bool IgnoreSavedState { get; }

		public bool ActivateOnPowerUp { get; }

		public byte TrackAPin0 { get; }
		public byte TrackAPin1 { get; }
		public byte TrackBPin0 { get; }
		public byte TrackBPin1 { get; }

		public QuadInverterDecoderV016(byte slot, BinaryReader reader) :
			base(slot, 5, true)
		{
			var flags = (Flags)reader.ReadByte();

			IsActive = (flags & Flags.QUAD_ACTIVE) == Flags.QUAD_ACTIVE;
			IgnoreSavedState = (flags & Flags.QUAD_IGNORE_SAVED_STATE) == Flags.QUAD_IGNORE_SAVED_STATE;
			ActivateOnPowerUp = (flags & Flags.QUAD_ACTIVATE_ON_POWER_UP) == Flags.QUAD_ACTIVATE_ON_POWER_UP;
			
			TrackAPin0 = reader.ReadByte();
			TrackAPin1 = reader.ReadByte();
			TrackBPin0 = reader.ReadByte();
			TrackBPin1 = reader.ReadByte();
		}
	}

	class QuadInverterDecoderV019 : Item
	{
		[Flags]
		enum Flags : byte
		{
			QUAD_IGNORE_SAVED_STATE = 0x02,
			QUAD_ACTIVATE_ON_POWER_UP = 0x04,
			QUAD_ACTIVE = 0x80
		}

		public bool IsActive { get; }

		public bool IgnoreSavedState { get; }

		public bool ActivateOnPowerUp { get; }

		public byte FlipInterval { get; }

		public byte TrackAPin0 { get; }
		public byte TrackAPin1 { get; }
		public byte TrackBPin0 { get; }
		public byte TrackBPin1 { get; }

		public QuadInverterDecoderV019(byte slot, BinaryReader reader) :
			base(slot, 6)
		{
			var flags = (Flags)reader.ReadByte();

			IsActive = (flags & Flags.QUAD_ACTIVE) == Flags.QUAD_ACTIVE;
			IgnoreSavedState = (flags & Flags.QUAD_IGNORE_SAVED_STATE) == Flags.QUAD_IGNORE_SAVED_STATE;
			ActivateOnPowerUp = (flags & Flags.QUAD_ACTIVATE_ON_POWER_UP) == Flags.QUAD_ACTIVATE_ON_POWER_UP;

			FlipInterval = reader.ReadByte();

			TrackAPin0 = reader.ReadByte();
			TrackAPin1 = reader.ReadByte();
			TrackBPin0 = reader.ReadByte();
			TrackBPin1 = reader.ReadByte();
		}
	}

	class QuadInverterDecoder : Item
	{
		[Flags]
		enum Flags : byte
		{
			QUAD_INVERTED = 0x01,
			QUAD_IGNORE_SAVED_STATE = 0x02,
			QUAD_ACTIVATE_ON_POWER_UP = 0x04,
			QUAD_ACTIVE = 0x80
		}

		public bool IsActive { get; }

		public bool IsInverted { get; }

		public bool IgnoreSavedState { get; }

		public bool ActivateOnPowerUp { get; }

		public byte FlipInterval { get; }

		public byte TrackAPin0 { get; }
		public byte TrackAPin1 { get; }
		public byte TrackBPin0 { get; }
		public byte TrackBPin1 { get; }

		public QuadInverterDecoder(byte slot, BinaryReader reader) :
			base(slot, 6)
		{
			var flags = (Flags)reader.ReadByte();

			IsInverted = (flags & Flags.QUAD_INVERTED) == Flags.QUAD_INVERTED;
			IsActive = (flags & Flags.QUAD_ACTIVE) == Flags.QUAD_ACTIVE;
			IgnoreSavedState = (flags & Flags.QUAD_IGNORE_SAVED_STATE) == Flags.QUAD_IGNORE_SAVED_STATE;
			ActivateOnPowerUp = (flags & Flags.QUAD_ACTIVATE_ON_POWER_UP) == Flags.QUAD_ACTIVATE_ON_POWER_UP;

			FlipInterval = reader.ReadByte();

			TrackAPin0 = reader.ReadByte();
			TrackAPin1 = reader.ReadByte();
			TrackBPin0 = reader.ReadByte();
			TrackBPin1 = reader.ReadByte();
		}
	}

	class Lump
	{
		static readonly Dictionary<string, Type> gKnownTypes = new()
		{
			{ "Bcs0008\0", typeof(RootLump) },
			{ "NetU002\0", typeof(NetworkLumpV002) },
			{ "NetU003\0", typeof(NetworkLump) },
			{ "Sson001\0", typeof(SessionLumpV001) },
			{ "Sson002\0", typeof(SessionLump) },
			{ "DECS015\0", typeof(DecodersLumpV016) },
			{ "DECS016\0", typeof(DecodersLumpV016) },  //015 and 016 the same, but 016 has QuadInverterDecoder
			{ "DECS017\0", typeof(DecodersLumpV017) },  //017 Turntable auto inverter changed
			{ "DECS018\0", typeof(DecodersLumpV018) },  //018 Sensor using word instead of byte for delays
			{ "DECS019\0", typeof(DecodersLump) },      //019 Turntable auto inverter using single pin for each track
			{ "DECS020\0", typeof(DecodersLump) },      //020 Quad inverter using invert flag (no storage size change)
			{ "ENDEND1\0", typeof(MarkerLump) }
		};

		[Category("Information")]
		public bool Deprecated { get; }

		[Category("Information")]
		public string Name { get; }

		[Category("Information")]
		public UInt16 Size { get; }

		public List<Lump> mChildren;

		public List<Item> mItems;

		public IEnumerable<Item> Items { get { return mItems; } }

		public IEnumerable<Lump> Children { get { return mChildren; } }

		public const int LUMP_HEADER_SIZE = 10;

		protected Lump(string name, UInt16 size, bool deprecated = false)
		{
			Name = name ?? throw new ArgumentNullException(nameof(name));
			Size = size;
			Deprecated = deprecated;
		}

		protected void AddChild(Lump lump)
		{
			mChildren ??= [];

			mChildren.Add(lump);
		}

		protected void AddItem(Item item)
		{
			mItems ??= [];

			mItems.Add(item);
		}

		public static Lump Create(BinaryReader reader)
		{
			var headerBytes = reader.ReadChars(8);
			var name = new String(headerBytes);
			 
			if (!gKnownTypes.TryGetValue(name, out Type foundType))
				throw new Exception("Lump " + name + " not supported");
				//foundType = typeof(UnknownLump);

			var size = reader.ReadUInt16();

			name = name.Replace('\0', ' ').Trim();

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

	class NetworkLumpV002 : Lump
	{
		public string NodeName { get; }
		public System.Net.NetworkInformation.PhysicalAddress Mac { get; }
		public UInt16 SrcPort { get; }

		public NetworkLumpV002(string name, UInt16 size, BinaryReader reader) :
			base(name, size, true)
		{
			if (size != 24)
				throw new ArgumentOutOfRangeException("[NetworkLumpV002] Expected size to be 24 bytes, but got " + size);

			const int MAX_NODE_NAME = 16;

			NodeName = Lump.ConvertBytesToString(reader.ReadBytes(MAX_NODE_NAME));
			Mac = new(reader.ReadBytes(6));
			SrcPort = reader.ReadUInt16();
		}
	}

	class NetworkLump : Lump
	{
		public string NodeName { get; }          

		public NetworkLump(string name, UInt16 size, BinaryReader reader) :
			base(name, size)
		{
			if (size != 16)
				throw new ArgumentOutOfRangeException("[NetworkLump] Expected size to be 16 bytes, but got " + size);

			const int MAX_NODE_NAME = 16;

			NodeName = Lump.ConvertBytesToString(reader.ReadBytes(MAX_NODE_NAME));
		}
	}

	class SessionLumpV001 : Lump
	{
		public System.Net.IPAddress IP { get; }
		public UInt16 ServerPort { get; }

		public SessionLumpV001(string name, UInt16 size, BinaryReader reader) :
			base(name, size, true)
		{
			if (size != 6)
				throw new ArgumentOutOfRangeException("[SessionLump] Expected size to be 6 bytes, but got " + size);

			IP = new(reader.ReadBytes(4));
			ServerPort = reader.ReadUInt16();
		}
	}

	class SessionLump : Lump
	{        
		public UInt16 ServerPort { get; }

		public SessionLump(string name, UInt16 size, BinaryReader reader) :
			base(name, size)
		{
			if (size != 2)
				throw new ArgumentOutOfRangeException("[SessionLump] Expected size to be 2 bytes, but got " + size);
			
			ServerPort = reader.ReadUInt16();
		}
	}

	class DecodersLumpV016 : Lump
	{
		enum DecoderTypes : byte
		{
			DEC_NULL = 0,
			DEC_OUTPUT = 1,
			DEC_SENSOR = 2,
			DEC_SERVO_TURNOUT = 3,
			DEC_SIGNAL = 4,			//Only virtual, not implemented on Arduino
			DEC_TURNTABLE_AUTO_INVERTER = 5,
			DEC_QUAD_INVERTER = 6
		};

		static readonly Dictionary<DecoderTypes, Type> gKnownTypes = new()
		{
			{ DecoderTypes.DEC_OUTPUT,                  typeof(OutputDecoder) },
			{ DecoderTypes.DEC_SENSOR,                  typeof(SensorDecoder) },
			{ DecoderTypes.DEC_SERVO_TURNOUT,           typeof(ServoTurnoutDecoder) },
			{ DecoderTypes.DEC_TURNTABLE_AUTO_INVERTER, typeof(TurntableAutoInverterDecoderV016) },
			{ DecoderTypes.DEC_QUAD_INVERTER,           typeof(QuadInverterDecoderV016) }
		};        

		public Guid Guid { get; }

		public DecodersLumpV016(string name, UInt16 size, BinaryReader reader) :
			base(name, size, true)
		{
			if (size < 17)
				throw new ArgumentOutOfRangeException("[DecodersLump] Must have at least 17 bytes, but got " + size);

			Guid = new Guid(reader.ReadBytes(16));

			var bytesLeft = size - 16;
			
			for(; ;)
			{
				--bytesLeft;
				var decType = (DecoderTypes)reader.ReadByte();
				if (decType == DecoderTypes.DEC_NULL)
					break;

				if (!gKnownTypes.TryGetValue(decType, out Type decoderClassType))
					throw new ArgumentOutOfRangeException("dectype not suppported: " + decType.ToString());

				--bytesLeft;
				var decQuantity = reader.ReadByte();

				for(var i = 0; i < decQuantity; ++i)
				{
					--bytesLeft;
					var slot = reader.ReadByte();

					var decoder = (Item) System.Activator.CreateInstance(decoderClassType, slot, reader);

					bytesLeft -= decoder.Size;

					this.AddItem(decoder);
				}

			}            

			if(bytesLeft != 0)
			{
				throw new ArgumentOutOfRangeException("bytes left...");
			}

			//skip...
			//reader.ReadBytes(size - 16);
		}
	}

	class DecodersLumpV017 : Lump
	{
		enum DecoderTypes : byte
		{
			DEC_NULL = 0,
			DEC_OUTPUT = 1,
			DEC_SENSOR = 2,
			DEC_SERVO_TURNOUT = 3,
			DEC_SIGNAL = 4,         //Only virtual, not implemented on Arduino
			DEC_TURNTABLE_AUTO_INVERTER = 5,
			DEC_QUAD_INVERTER = 6
		};

		static readonly Dictionary<DecoderTypes, Type> gKnownTypes = new()
		{
			{ DecoderTypes.DEC_OUTPUT,                  typeof(OutputDecoder) },
			{ DecoderTypes.DEC_SENSOR,                  typeof(SensorDecoderV017) },
			{ DecoderTypes.DEC_SERVO_TURNOUT,           typeof(ServoTurnoutDecoder) },
			{ DecoderTypes.DEC_TURNTABLE_AUTO_INVERTER, typeof(TurntableAutoInverterDecoder) },
			{ DecoderTypes.DEC_QUAD_INVERTER,           typeof(QuadInverterDecoder) }
		};

		public Guid Guid { get; }

		public DecodersLumpV017(string name, UInt16 size, BinaryReader reader) :
			base(name, size)
		{
			if (size < 17)
				throw new ArgumentOutOfRangeException("[DecodersLump] Must have at least 17 bytes, but got " + size);

			Guid = new Guid(reader.ReadBytes(16));

			var bytesLeft = size - 16;

			for (; ; )
			{
				--bytesLeft;
				var decType = (DecoderTypes)reader.ReadByte();
				if (decType == DecoderTypes.DEC_NULL)
					break;

				if (!gKnownTypes.TryGetValue(decType, out Type decoderClassType))
					throw new ArgumentOutOfRangeException("dectype not suppported: " + decType.ToString());

				--bytesLeft;
				var decQuantity = reader.ReadByte();

				for (var i = 0; i < decQuantity; ++i)
				{
					--bytesLeft;
					var slot = reader.ReadByte();

					var decoder = (Item)System.Activator.CreateInstance(decoderClassType, slot, reader);

					bytesLeft -= decoder.Size;

					this.AddItem(decoder);
				}

			}

			if (bytesLeft != 0)
			{
				throw new ArgumentOutOfRangeException("bytes left...");
			}

			//skip...
			//reader.ReadBytes(size - 16);
		}
	}

	class DecodersLumpV018 : Lump
	{
		enum DecoderTypes : byte
		{
			DEC_NULL = 0,
			DEC_OUTPUT = 1,
			DEC_SENSOR = 2,
			DEC_SERVO_TURNOUT = 3,
			DEC_SIGNAL = 4,         //Only virtual, not implemented on Arduino
			DEC_TURNTABLE_AUTO_INVERTER = 5,
			DEC_QUAD_INVERTER = 6
		};

		static readonly Dictionary<DecoderTypes, Type> gKnownTypes = new()
		{
			{ DecoderTypes.DEC_OUTPUT,                  typeof(OutputDecoder) },
			{ DecoderTypes.DEC_SENSOR,                  typeof(SensorDecoder) },
			{ DecoderTypes.DEC_SERVO_TURNOUT,           typeof(ServoTurnoutDecoder) },
			{ DecoderTypes.DEC_TURNTABLE_AUTO_INVERTER, typeof(TurntableAutoInverterDecoderV018) },
			{ DecoderTypes.DEC_QUAD_INVERTER,           typeof(QuadInverterDecoder) }
		};

		public Guid Guid { get; }

		public DecodersLumpV018(string name, UInt16 size, BinaryReader reader) :
			base(name, size)
		{
			if (size < 17)
				throw new ArgumentOutOfRangeException("[DecodersLump] Must have at least 17 bytes, but got " + size);

			Guid = new Guid(reader.ReadBytes(16));

			var bytesLeft = size - 16;

			for (; ; )
			{
				--bytesLeft;
				var decType = (DecoderTypes)reader.ReadByte();
				if (decType == DecoderTypes.DEC_NULL)
					break;

				if (!gKnownTypes.TryGetValue(decType, out Type decoderClassType))
					throw new ArgumentOutOfRangeException("dectype not suppported: " + decType.ToString());

				--bytesLeft;
				var decQuantity = reader.ReadByte();

				for (var i = 0; i < decQuantity; ++i)
				{
					--bytesLeft;
					var slot = reader.ReadByte();

					var decoder = (Item)System.Activator.CreateInstance(decoderClassType, slot, reader);

					bytesLeft -= decoder.Size;

					this.AddItem(decoder);
				}

			}

			if (bytesLeft != 0)
			{
				throw new ArgumentOutOfRangeException("bytes left...");
			}

			//skip...
			//reader.ReadBytes(size - 16);
		}
	}

	class DecodersLumpV019 : Lump
	{
		enum DecoderTypes : byte
		{
			DEC_NULL = 0,
			DEC_OUTPUT = 1,
			DEC_SENSOR = 2,
			DEC_SERVO_TURNOUT = 3,
			DEC_SIGNAL = 4,         //Only virtual, not implemented on Arduino
			DEC_TURNTABLE_AUTO_INVERTER = 5,
			DEC_QUAD_INVERTER = 6
		};

		static readonly Dictionary<DecoderTypes, Type> gKnownTypes = new()
		{
			{ DecoderTypes.DEC_OUTPUT,                  typeof(OutputDecoder) },
			{ DecoderTypes.DEC_SENSOR,                  typeof(SensorDecoderV019) },				//does not have StartDelay (uint16)
			{ DecoderTypes.DEC_SERVO_TURNOUT,           typeof(ServoTurnoutDecoder) },
			{ DecoderTypes.DEC_TURNTABLE_AUTO_INVERTER, typeof(TurntableAutoInverterDecoderV019) },	//flag: inverted (no size change)
			{ DecoderTypes.DEC_QUAD_INVERTER,           typeof(QuadInverterDecoderV019) }			//flag: inverted (no size change)
		};

		public Guid Guid { get; }

		public DecodersLumpV019(string name, UInt16 size, BinaryReader reader) :
			base(name, size)
		{
			if (size < 17)
				throw new ArgumentOutOfRangeException("[DecodersLumpV019] Must have at least 17 bytes, but got " + size);

			Guid = new Guid(reader.ReadBytes(16));

			var bytesLeft = size - 16;

			for (; ; )
			{
				--bytesLeft;
				var decType = (DecoderTypes)reader.ReadByte();
				if (decType == DecoderTypes.DEC_NULL)
					break;

				if (!gKnownTypes.TryGetValue(decType, out Type decoderClassType))
					throw new ArgumentOutOfRangeException("dectype not suppported: " + decType.ToString());

				--bytesLeft;
				var decQuantity = reader.ReadByte();

				for (var i = 0; i < decQuantity; ++i)
				{
					--bytesLeft;
					var slot = reader.ReadByte();

					var decoder = (Item)System.Activator.CreateInstance(decoderClassType, slot, reader);

					bytesLeft -= decoder.Size;

					this.AddItem(decoder);
				}

			}

			if (bytesLeft != 0)
			{
				throw new ArgumentOutOfRangeException("bytes left...");
			}

			//skip...
			//reader.ReadBytes(size - 16);
		}
	}

	class DecodersLump : Lump
	{
		enum DecoderTypes : byte
		{
			DEC_NULL = 0,
			DEC_OUTPUT = 1,
			DEC_SENSOR = 2,
			DEC_SERVO_TURNOUT = 3,
			DEC_SIGNAL = 4,         //Only virtual, not implemented on Arduino
			DEC_TURNTABLE_AUTO_INVERTER = 5,
			DEC_QUAD_INVERTER = 6
		};

		static readonly Dictionary<DecoderTypes, Type> gKnownTypes = new()
		{
			{ DecoderTypes.DEC_OUTPUT,                  typeof(OutputDecoder) },
			{ DecoderTypes.DEC_SENSOR,                  typeof(SensorDecoder) },
			{ DecoderTypes.DEC_SERVO_TURNOUT,           typeof(ServoTurnoutDecoder) },
			{ DecoderTypes.DEC_TURNTABLE_AUTO_INVERTER, typeof(TurntableAutoInverterDecoder) },
			{ DecoderTypes.DEC_QUAD_INVERTER,           typeof(QuadInverterDecoder) }
		};

		public Guid Guid { get; }

		public DecodersLump(string name, UInt16 size, BinaryReader reader) :
			base(name, size)
		{
			if (size < 17)
				throw new ArgumentOutOfRangeException("[DecodersLump] Must have at least 17 bytes, but got " + size);

			Guid = new Guid(reader.ReadBytes(16));

			var bytesLeft = size - 16;

			for (; ; )
			{
				--bytesLeft;
				var decType = (DecoderTypes)reader.ReadByte();
				if (decType == DecoderTypes.DEC_NULL)
					break;

				if (!gKnownTypes.TryGetValue(decType, out Type decoderClassType))
					throw new ArgumentOutOfRangeException("dectype not suppported: " + decType.ToString());

				--bytesLeft;
				var decQuantity = reader.ReadByte();

				for (var i = 0; i < decQuantity; ++i)
				{
					--bytesLeft;
					var slot = reader.ReadByte();

					var decoder = (Item)System.Activator.CreateInstance(decoderClassType, slot, reader);

					bytesLeft -= decoder.Size;

					this.AddItem(decoder);
				}

			}

			if (bytesLeft != 0)
			{
				throw new ArgumentOutOfRangeException("bytes left...");
			}

			//skip...
			//reader.ReadBytes(size - 16);
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
