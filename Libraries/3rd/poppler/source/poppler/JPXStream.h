//========================================================================
//
// JPXStream.h
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2019, 2021 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef JPXSTREAM_H
#define JPXSTREAM_H

#include "Object.h"
#include "Stream.h"

class JArithmeticDecoder;
class JArithmeticDecoderStats;

//------------------------------------------------------------------------

enum JPXColorSpaceType
{
    jpxCSBiLevel = 0,
    jpxCSYCbCr1 = 1,
    jpxCSYCbCr2 = 3,
    jpxCSYCBCr3 = 4,
    jpxCSPhotoYCC = 9,
    jpxCSCMY = 11,
    jpxCSCMYK = 12,
    jpxCSYCCK = 13,
    jpxCSCIELab = 14,
    jpxCSsRGB = 16,
    jpxCSGrayscale = 17,
    jpxCSBiLevel2 = 18,
    jpxCSCIEJab = 19,
    jpxCSCISesRGB = 20,
    jpxCSROMMRGB = 21,
    jpxCSsRGBYCbCr = 22,
    jpxCSYPbPr1125 = 23,
    jpxCSYPbPr1250 = 24
};

struct JPXColorSpecCIELab
{
    unsigned int rl, ol, ra, oa, rb, ob, il;
};

struct JPXColorSpecEnumerated
{
    JPXColorSpaceType type; // color space type
    union {
        JPXColorSpecCIELab cieLab;
    };
};

struct JPXColorSpec
{
    unsigned int meth; // method
    int prec; // precedence
    union {
        JPXColorSpecEnumerated enumerated;
    };
};

//------------------------------------------------------------------------

struct JPXPalette
{
    unsigned int nEntries; // number of entries in the palette
    unsigned int nComps; // number of components in each entry
    unsigned int *bpc; // bits per component, for each component
    int *c; // color data:
            //   c[i*nComps+j] = entry i, component j
};

//------------------------------------------------------------------------

struct JPXCompMap
{
    unsigned int nChannels; // number of channels
    unsigned int *comp; // codestream components mapped to each channel
    unsigned int *type; // 0 for direct use, 1 for palette mapping
    unsigned int *pComp; // palette components to use
};

//------------------------------------------------------------------------

struct JPXChannelDefn
{
    unsigned int nChannels; // number of channels
    unsigned int *idx; // channel indexes
    unsigned int *type; // channel types
    unsigned int *assoc; // channel associations
};

//------------------------------------------------------------------------

struct JPXTagTreeNode
{
    bool finished; // true if this node is finished
    unsigned int val; // current value
};

//------------------------------------------------------------------------

struct JPXCodeBlock
{
    //----- size
    unsigned int x0, y0, x1, y1; // bounds

    //----- persistent state
    bool seen; // true if this code-block has already
               //   been seen
    unsigned int lBlock; // base number of bits used for pkt data length
    unsigned int nextPass; // next coding pass

    //---- info from first packet
    unsigned int nZeroBitPlanes; // number of zero bit planes

    //----- info for the current packet
    unsigned int included; // code-block inclusion in this packet:
                           //   0=not included, 1=included
    unsigned int nCodingPasses; // number of coding passes in this pkt
    unsigned int *dataLen; // data lengths (one per codeword segment)
    unsigned int dataLenSize; // size of the dataLen array

    //----- coefficient data
    int *coeffs;
    char *touched; // coefficient 'touched' flags
    unsigned short len; // coefficient length
    JArithmeticDecoder // arithmetic decoder
            *arithDecoder;
    JArithmeticDecoderStats // arithmetic decoder stats
            *stats;
};

//------------------------------------------------------------------------

struct JPXSubband
{
    //----- computed
    unsigned int x0, y0, x1, y1; // bounds
    unsigned int nXCBs, nYCBs; // number of code-blocks in the x and y
                               //   directions

    //----- tag trees
    unsigned int maxTTLevel; // max tag tree level
    JPXTagTreeNode *inclusion; // inclusion tag tree for each subband
    JPXTagTreeNode *zeroBitPlane; // zero-bit plane tag tree for each
                                  //   subband

    //----- children
    JPXCodeBlock *cbs; // the code-blocks (len = nXCBs * nYCBs)
};

//------------------------------------------------------------------------

struct JPXPrecinct
{
    //----- computed
    unsigned int x0, y0, x1, y1; // bounds of the precinct

    //----- children
    JPXSubband *subbands; // the subbands
};

//------------------------------------------------------------------------

struct JPXResLevel
{
    //----- from the COD and COC segments (main and tile)
    unsigned int precinctWidth; // log2(precinct width)
    unsigned int precinctHeight; // log2(precinct height)

    //----- computed
    unsigned int x0, y0, x1, y1; // bounds of the tile-comp (for this res level)
    unsigned int bx0[3], by0[3], // subband bounds
            bx1[3], by1[3];

    //---- children
    JPXPrecinct *precincts; // the precincts
};

//------------------------------------------------------------------------

struct JPXTileComp
{
    //----- from the SIZ segment
    bool sgned; // 1 for signed, 0 for unsigned
    unsigned int prec; // precision, in bits
    unsigned int hSep; // horizontal separation of samples
    unsigned int vSep; // vertical separation of samples

    //----- from the COD and COC segments (main and tile)
    unsigned int style; // coding style parameter (Scod / Scoc)
    unsigned int nDecompLevels; // number of decomposition levels
    unsigned int codeBlockW; // log2(code-block width)
    unsigned int codeBlockH; // log2(code-block height)
    unsigned int codeBlockStyle; // code-block style
    unsigned int transform; // wavelet transformation

    //----- from the QCD and QCC segments (main and tile)
    unsigned int quantStyle; // quantization style
    unsigned int *quantSteps; // quantization step size for each subband
    unsigned int nQuantSteps; // number of entries in quantSteps

    //----- computed
    unsigned int x0, y0, x1, y1; // bounds of the tile-comp, in ref coords
    unsigned int w; // x1 - x0
    unsigned int cbW; // code-block width
    unsigned int cbH; // code-block height

    //----- image data
    int *data; // the decoded image data
    int *buf; // intermediate buffer for the inverse
              //   transform

    //----- children
    JPXResLevel *resLevels; // the resolution levels
                            //   (len = nDecompLevels + 1)
};

//------------------------------------------------------------------------

struct JPXTile
{
    bool init;

    //----- from the COD segments (main and tile)
    unsigned int progOrder; // progression order
    unsigned int nLayers; // number of layers
    unsigned int multiComp; // multiple component transformation

    //----- computed
    unsigned int x0, y0, x1, y1; // bounds of the tile, in ref coords
    unsigned int maxNDecompLevels; // max number of decomposition levels used
                                   //   in any component in this tile

    //----- progression order loop counters
    unsigned int comp; //   component
    unsigned int res; //   resolution level
    unsigned int precinct; //   precinct
    unsigned int layer; //   layer

    //----- children
    JPXTileComp *tileComps; // the tile-components (len = JPXImage.nComps)
};

//------------------------------------------------------------------------

struct JPXImage
{
    //----- from the SIZ segment
    unsigned int xSize, ySize; // size of reference grid
    unsigned int xOffset, yOffset; // image offset
    unsigned int xTileSize, yTileSize; // size of tiles
    unsigned int xTileOffset, // offset of first tile
            yTileOffset;
    unsigned int nComps; // number of components

    //----- computed
    unsigned int nXTiles; // number of tiles in x direction
    unsigned int nYTiles; // number of tiles in y direction

    //----- children
    JPXTile *tiles; // the tiles (len = nXTiles * nYTiles)
};

//------------------------------------------------------------------------

class JPXStream : public FilterStream
{
public:
    JPXStream(Stream *strA);
    virtual ~JPXStream();
    StreamKind getKind() const override { return strJPX; }
    void reset() override;
    void close() override;
    int getChar() override;
    int lookChar() override;
    GooString *getPSFilter(int psLevel, const char *indent) override;
    bool isBinary(bool last = true) const override;
    void getImageParams(int *bitsPerComponent, StreamColorSpaceMode *csMode) override;

private:
    void fillReadBuf();
    void getImageParams2(int *bitsPerComponent, StreamColorSpaceMode *csMode);
    bool readBoxes();
    bool readColorSpecBox(unsigned int dataLen);
    bool readCodestream(unsigned int len);
    bool readTilePart();
    bool readTilePartData(unsigned int tileIdx, unsigned int tilePartLen, bool tilePartToEOC);
    bool readCodeBlockData(JPXTileComp *tileComp, JPXResLevel *resLevel, JPXPrecinct *precinct, JPXSubband *subband, unsigned int res, unsigned int sb, JPXCodeBlock *cb);
    void inverseTransform(JPXTileComp *tileComp);
    void inverseTransformLevel(JPXTileComp *tileComp, unsigned int r, JPXResLevel *resLevel);
    void inverseTransform1D(JPXTileComp *tileComp, int *data, unsigned int offset, unsigned int n);
    bool inverseMultiCompAndDC(JPXTile *tile);
    bool readBoxHdr(unsigned int *boxType, unsigned int *boxLen, unsigned int *dataLen);
    int readMarkerHdr(int *segType, unsigned int *segLen);
    bool readUByte(unsigned int *x);
    bool readByte(int *x);
    bool readUWord(unsigned int *x);
    bool readULong(unsigned int *x);
    bool readNBytes(int nBytes, bool signd, int *x);
    void startBitBuf(unsigned int byteCountA);
    bool readBits(int nBits, unsigned int *x);
    void skipSOP();
    void skipEPH();
    unsigned int finishBitBuf();

    BufStream *bufStr; // buffered stream (for lookahead)

    unsigned int nComps; // number of components
    unsigned int *bpc; // bits per component, for each component
    unsigned int width, height; // image size
    bool haveImgHdr; // set if a JP2/JPX image header has been
                     //   found
    JPXColorSpec cs; // color specification
    bool haveCS; // set if a color spec has been found
    JPXPalette palette; // the palette
    bool havePalette; // set if a palette has been found
    JPXCompMap compMap; // the component mapping
    bool haveCompMap; // set if a component mapping has been found
    JPXChannelDefn channelDefn; // channel definition
    bool haveChannelDefn; // set if a channel defn has been found

    JPXImage img; // JPEG2000 decoder data
    unsigned int bitBuf; // buffer for bit reads
    int bitBufLen; // number of bits in bitBuf
    bool bitBufSkip; // true if next bit should be skipped
                     //   (for bit stuffing)
    unsigned int byteCount; // number of available bytes left

    unsigned int curX, curY, curComp; // current position for lookChar/getChar
    unsigned int readBuf; // read buffer
    unsigned int readBufLen; // number of valid bits in readBuf
};

#endif
