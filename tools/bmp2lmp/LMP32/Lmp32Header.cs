
namespace bmp2lmp
{
    /// <summary>
    /// LMP32 Header
    /// 
    /// This class exists for future tools
    /// </summary>
    internal class Lmp32Header
    {
        /// <summary>
        /// REMOVED because quake loves converting byte arrays to structures
        /// and this ruins the alignment
        /// </summary>
        //internal static byte[] Magic = new byte[] { 0x4C, 0x4D, 0x50, 0x33, 0x32 }; // "MIP32"

        internal int Width { get; set; }

        internal int Height { get; set; }

        internal void Write(BinaryWriter writer)
        {
            //writer.Write(Magic);
            writer.Write(Width);
            writer.Write(Height);
        }
    }
}
