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
using System.Windows.Forms;

namespace SharpTerminal
{
    public partial class ServerSelectionForm : Form
    {    

        public ServerSelectionForm()
        {
            InitializeComponent();            
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);            
        }        

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            base.OnFormClosed(e);            
        }            
    }
}
