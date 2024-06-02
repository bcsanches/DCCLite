// Copyright (C) 2022 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

using System.Threading.Tasks;

namespace SharpCommon
{
    public interface IServoProgrammer
    {
        Task StartAsync(IServoTurnout turnout);

        Task StopAsync();

        Task SetPositionAsync(int position);

        /// <summary>
        /// Deploy the current parameters to the remote servo.
        /// 
        /// Programmer stops after the deploy, so IsRunning returns false after calling this
        /// </summary>
        /// <param name="flags"></param>
        /// <param name="startPos"></param>
        /// <param name="endPos"></param>
        /// <param name="msOperationTime"></param>
        /// <returns></returns>
        Task DeployAsync(ServoTurnoutFlags flags, uint startPos, uint endPos, uint msOperationTime);

        /// <summary>
        ///     Must return after StartAsync has been called
        /// </summary>
        /// 
        /// <returns>True if Start was called</returns>
        bool IsRunning();
    }
}
