using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SharpTerminal
{
    interface IConsole
    {
        void Println(string text);
        void Clear();
    }
}
