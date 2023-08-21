using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace bmp2lmp
{
    /// <summary>
    /// LMP32 Header
    /// 
    /// This class exists for future tools
    /// </summary>
    internal class Lmp32Header
    {
        internal static byte[] Magic = new byte[] { 0x4C, 0x4D, 0x50, 0x33, 0x32 }; // "MIP32"

        internal int Width { get; set; }

        internal int Height { get; set; }

        internal void Write(BinaryWriter writer)
        {
            writer.Write(Magic);
            writer.Write(Width);
            writer.Write(Height);
        }
    }
}
