
namespace wal2tga
{
    internal enum TgaDataType : byte 
    {
        None = 0,

        Uncompressed_Colormap = 1,

        Uncompressed_RGB = 2,

        Uncompressed_BW = 3,

        RLE_Colormap = 9,

        RLE_RGB = 10,

        RLE_BW = 11,

        Huffman_Colormap = 32,

        Huffman_Colormap_4Pass = 33,
    }

    /// <summary>
    /// TGA file reader and writer class. Only supports Zombono 32-bit TGAs.
    /// </summary>
    internal class TgaFile
    {
        public byte imageIdSize;
        public byte colorMapType;
        public TgaDataType imageType;
        public short colorMapOrigin;   //0x00
        public short colorMapLength;   //ignored
        public byte colorMapEntrySize; //ignored
        public short imageOriginX;     //0?
        public short imageOriginY;     //0?
        public ushort imageWidth;
        public ushort imageHeight;
        public byte bitsPerPixel;      //32

        public byte[] imageData;       //how do we fit the WAL mipmaps into here?

        private TgaFile(ushort width, ushort height)
        {
            imageWidth = width;
            imageHeight = height;
            imageData = new byte[imageWidth * imageHeight];
        }

        public static TgaFile Read(string filename)
        {
            using (BinaryReader br = new(new FileStream(filename, FileMode.Open)))
            {
                // create image data
                br.BaseStream.Seek(14, SeekOrigin.Begin);

                TgaFile fileData = new(br.ReadUInt16(), br.ReadUInt16());

                // validate BPP (only 32-bit TGAs are supported)
                fileData.bitsPerPixel = br.ReadByte();

                if (fileData.bitsPerPixel != 32)
                {
                    PrintErrorAndExit("Only 32-bit TGAs are supported!", 6);
                }

                br.BaseStream.Seek(0, SeekOrigin.Begin);

                fileData.imageIdSize = br.ReadByte();
                fileData.colorMapType = br.ReadByte();
                fileData.imageType = (TgaDataType)br.ReadByte();

                if (fileData.imageType != TgaDataType.Uncompressed_RGB)
                {
                    PrintErrorAndExit("Only RGB-type TGA images are supported!", 7);
                }

                fileData.colorMapOrigin = br.ReadInt16();
                fileData.colorMapLength = br.ReadInt16();
                fileData.colorMapEntrySize = br.ReadByte();
                fileData.imageOriginX = br.ReadInt16();
                fileData.imageOriginY = br.ReadInt16();
                
                // pixel format = RGBA
            }
        }

        public static bool Write(string fileName, ushort width, ushort height)
        {
            using (BinaryWriter bw = new BinaryWriter(new FileStream(fileName, FileMode.OpenOrCreate)))
            {
                bw.Write((byte)0);                                  // Image ID Length
                bw.Write((byte)0);                                  // Color Map Type
                bw.Write((byte)TgaDataType.Uncompressed_RGB);       // Image Data Type
                bw.Write((short)0);                                 // Color Map Origin
                bw.Write((short)0);                                 // Color Map Type
                bw.Write((byte)0);                                  // Color Map Entry Size
                bw.Write((short)0);                                 // Image Origin X
                bw.Write((short)0);                                 // Image Origin Y
                bw.Write(width);                                    // Image Width
                bw.Write(height);                                   // Image Height
                bw.Write(32);                                       // Image BPP
                
            }
        }
    }
}
