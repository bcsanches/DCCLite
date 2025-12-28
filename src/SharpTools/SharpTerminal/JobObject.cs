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
using System.Diagnostics;
using System.Runtime.InteropServices;

static class JobObject
{
	private const int JobObjectExtendedLimitInformation = 9;
	private const uint JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE = 0x00002000;

	[StructLayout(LayoutKind.Sequential)]
	struct JOBOBJECT_BASIC_LIMIT_INFORMATION
	{
		public long PerProcessUserTimeLimit;
		public long PerJobUserTimeLimit;
		public uint LimitFlags;
		public UIntPtr MinimumWorkingSetSize;
		public UIntPtr MaximumWorkingSetSize;
		public uint ActiveProcessLimit;
		public long Affinity;
		public uint PriorityClass;
		public uint SchedulingClass;
	}

	[StructLayout(LayoutKind.Sequential)]
	struct IO_COUNTERS
	{
		public ulong ReadOperationCount;
		public ulong WriteOperationCount;
		public ulong OtherOperationCount;
		public ulong ReadTransferCount;
		public ulong WriteTransferCount;
		public ulong OtherTransferCount;
	}

	[StructLayout(LayoutKind.Sequential)]
	struct JOBOBJECT_EXTENDED_LIMIT_INFORMATION
	{
		public JOBOBJECT_BASIC_LIMIT_INFORMATION BasicLimitInformation;
		public IO_COUNTERS IoInfo;
		public UIntPtr ProcessMemoryLimit;
		public UIntPtr JobMemoryLimit;
		public UIntPtr PeakProcessMemoryUsed;
		public UIntPtr PeakJobMemoryUsed;
	}

	[DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
	static extern IntPtr CreateJobObject(IntPtr lpJobAttributes, string lpName);

	[DllImport("kernel32.dll")]
	static extern bool SetInformationJobObject(
		IntPtr hJob,
		int JobObjectInfoClass,
		IntPtr lpJobObjectInfo,
		uint cbJobObjectInfoLength);

	[DllImport("kernel32.dll", SetLastError = true)]
	static extern bool AssignProcessToJobObject(IntPtr job, IntPtr process);

	public static IntPtr CreateKillOnCloseJob()
	{
		IntPtr job = CreateJobObject(IntPtr.Zero, null);
		if (job == IntPtr.Zero)
			throw new System.ComponentModel.Win32Exception();

		var info = new JOBOBJECT_EXTENDED_LIMIT_INFORMATION();
		info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

		int length = Marshal.SizeOf(info);
		IntPtr ptr = Marshal.AllocHGlobal(length);

		try
		{
			Marshal.StructureToPtr(info, ptr, false);

			if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, ptr, (uint)length))
				throw new System.ComponentModel.Win32Exception();
		}
		finally
		{
			Marshal.FreeHGlobal(ptr);
		}
		
		return job;
	}

	public static void AddProcess(IntPtr job, Process process)
	{
		if (!AssignProcessToJobObject(job, process.Handle))
			throw new System.ComponentModel.Win32Exception();
	}
}
