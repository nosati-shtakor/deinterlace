#include "pch.h"
#include <fstream>
#include "LibJpeg.h"
#include "utility.h"


LibJpeg::LibJpeg()
:
imageSize_     ( 0 ),
subsampling_   ( invalid ),
width_         ( 0 ),
height_        ( 0 ),
ySamplesPerRow_( 0 ),
numRows_       ( 0 ),
uSamplesPerRow_( 0 ),
vSamplesPerRow_( 0 )
{

}


int LibJpeg::decompress( const char * const pFileName )
{
    const RAII::AUTO_FOPEN jpgFile( pFileName, "rb" );
    if( !jpgFile.valid() )
        return -1; // TO DO;

    jpeg_error_mgr jerr;

    RAII::AUTO_JPEG_CONTEXT<jpeg_decompress_struct, jpeg_CreateDecompress, jpeg_destroy_decompress> decompressCtx( JPEG_LIB_VERSION );
        
    jpeg_decompress_struct & cinfo( *decompressCtx.get() );
        
    cinfo.err = jpeg_std_error( &jerr );
    // cinfo.err->error_exit = my_error_exit;

    jpeg_stdio_src( &cinfo, jpgFile.get() );
    
    jpeg_read_header( &cinfo, TRUE );

    cinfo.dct_method            = JDCT_FLOAT;
    cinfo.out_color_space       = JCS_YCbCr;
    cinfo.do_fancy_upsampling   = FALSE;
    cinfo.raw_data_out          = TRUE;
    
    if( !jpeg_start_decompress( &cinfo ) )
        return -2; // TO DO;

    subsampling_ = getSampling( &cinfo );

    if( subsampling_ == invalid )
        return -3; // TO DO:...

    imageSize_ = getImageSize( cinfo );
    
    if( pYUVData_.size() < imageSize_ )
        pYUVData_.resize( imageSize_ ); // TO DO: wasting memory here in some cases. Should we fix this?!

    read( cinfo, subsampling_ );

    width_  = cinfo.output_width;
    height_ = cinfo.output_height;

    ySamplesPerRow_ = cinfo.comp_info[ 0 ].width_in_blocks  * DCTSIZE;
    numRows_        = cinfo.comp_info[ 0 ].height_in_blocks * DCTSIZE;

    uSamplesPerRow_ = cinfo.comp_info[ 1 ].width_in_blocks * DCTSIZE;
    vSamplesPerRow_ = cinfo.comp_info[ 2 ].width_in_blocks * DCTSIZE;

    // 
    // for debugging ONLY!!
    //
    // dump2File( pYUVData_.data(), cinfo );
    // 

    jpeg_finish_decompress( &cinfo );

    return 0;
}


void LibJpeg::read( jpeg_decompress_struct & cinfo, const SUBSAMPLING subsampling )
{
    char * y[ DCTSIZE << 2 ] = { nullptr }; // max value for the v_samp_factor is 4 (if the comment in jpeglib.h is correct)
    char * u[ DCTSIZE << 2 ] = { nullptr };
    char * v[ DCTSIZE << 2 ] = { nullptr };

    const int numLines( cinfo.max_v_samp_factor * DCTSIZE );

    const int ySamplesPerRow( cinfo.comp_info[ 0 ].width_in_blocks  * DCTSIZE );
    const int yNumRows      ( cinfo.comp_info[ 0 ].height_in_blocks * DCTSIZE );
    const int uSamplesPerRow( cinfo.comp_info[ 1 ].width_in_blocks  * DCTSIZE );
    const int uNumRows      ( cinfo.comp_info[ 1 ].height_in_blocks * DCTSIZE );
    const int vSamplesPerRow( cinfo.comp_info[ 2 ].width_in_blocks  * DCTSIZE );
    const int vNumRows      ( cinfo.comp_info[ 2 ].height_in_blocks * DCTSIZE );

    char * pRawYOutput( pYUVData_.data() );
    char * pRawUOutput( pRawYOutput + ySamplesPerRow * yNumRows );
    char * pRawVOutput( pRawUOutput + uSamplesPerRow * uNumRows );

    char ** ppYUV[] = { y, u, v };
    const int shift( subsampling == _420 || subsampling == _440 ? 1 : 0 );

    for( int i( 0 ); i < yNumRows; i += numLines )
    {
        for( int j( 0 ); j < numLines; ++j )
        {
            y[ j ] = pRawYOutput;
            pRawYOutput += ySamplesPerRow;

            if( j < ( numLines >> shift ) )
            {
                u[ j ] = pRawUOutput;
                v[ j ] = pRawVOutput;

                pRawUOutput += uSamplesPerRow;
                pRawVOutput += vSamplesPerRow;
            }
        }

        const int result( jpeg_read_raw_data( &cinfo, reinterpret_cast<JSAMPIMAGE>( &ppYUV ), numLines ) );
    }
}


void LibJpeg::my_error_exit( j_common_ptr cinfo_ptr )
{
    const jpeg_decompress_struct * const pcinfo( reinterpret_cast<jpeg_decompress_struct*>( cinfo_ptr ) );

    std::cout << pcinfo->err->jpeg_message_table[ pcinfo->err->msg_code - pcinfo->err->first_addon_message ] << std::endl;
}


SUBSAMPLING LibJpeg::getSampling( const jpeg_decompress_struct * const pjds )const
{
    const jpeg_component_info * const pjci( pjds->comp_info );

    if( pjci->h_samp_factor == 1 && pjci->v_samp_factor == 1 )
        return _444;
    else if( pjci->h_samp_factor == 2 && pjci->v_samp_factor == 1 )
        return _422;
    else if( pjci->h_samp_factor == 1 && pjci->v_samp_factor == 2 )
        return _440;
    else if( pjci->h_samp_factor == 2 && pjci->v_samp_factor == 2 )
        return _420;

    return invalid;
}


int LibJpeg::getImageSize( const jpeg_decompress_struct & cinfo )const
{
    int size( 0 );

    for( int i( 0 ); i < 3; ++i )
    {
        const int samplesPerRow( cinfo.comp_info[ i ].width_in_blocks * DCTSIZE );
        const int numRows      ( cinfo.comp_info[ i ].height_in_blocks * DCTSIZE );

        size += ( samplesPerRow * numRows );
    }
    
    return size;
}


int LibJpeg::compress( const int width,
                       const int height,
                       const SUBSAMPLING subsampling,
                       const int ySamplesPerRow,
                       const int uSamplesPerRow,
                       const int vSamplesPerRow,
                       const int rowsToProcess,
                       const char * pYUVData,
                       const int yuvSize,
                       const int quality,
                       const char * const pFileName )
{
    const RAII::AUTO_FOPEN jpgFile( pFileName, "wb" );
    if( !jpgFile.valid() )
        return -1; // TO DO;

    jpeg_error_mgr jerr;

    RAII::AUTO_JPEG_CONTEXT<jpeg_compress_struct, jpeg_CreateCompress, jpeg_destroy_compress> compressCtx( JPEG_LIB_VERSION );
        
    jpeg_compress_struct & cinfo( *compressCtx.get() );
    
    cinfo.err = jpeg_std_error( &jerr );

    cinfo.image_width       = width;
    cinfo.image_height      = height;
    cinfo.input_components  = 3;
        
    jpeg_set_defaults( &cinfo );

    cinfo.in_color_space = JCS_YCbCr;

    jpeg_set_colorspace( &cinfo, JCS_YCbCr );
    
    setSamplingFactors( cinfo, subsampling );

    jpeg_set_quality( &cinfo, quality, TRUE );

    cinfo.dct_method  = JDCT_FLOAT;
    cinfo.raw_data_in = TRUE;

    jpeg_stdio_dest( &cinfo, jpgFile.get() );
    
    write( cinfo, pYUVData, yuvSize, ySamplesPerRow, uSamplesPerRow, vSamplesPerRow, rowsToProcess, subsampling );
    
    return 0;
}


void LibJpeg::write( jpeg_compress_struct & cinfo,
                     const char * pYUVData,
                     const int yuvSize,
                     const int ySamplesPerRow,
                     const int uSamplesPerRow,
                     const int vSamplesPerRow,
                     const int rowsToProcess,
                     const SUBSAMPLING subsampling )
{
    yuvSize;

    const RAII::AUTO_JPEG_START_FINISH_COMPRESS<jpeg_compress_struct, boolean, jpeg_start_compress, jpeg_finish_compress> startFinish( &cinfo, TRUE );

    const char * y[ DCTSIZE << 2 ] = { nullptr }; // max value for the v_samp_factor is 4 (if the comment in jpeglib.h is correct)
    const char * u[ DCTSIZE << 2 ] = { nullptr };
    const char * v[ DCTSIZE << 2 ] = { nullptr };

    const int numLines( cinfo.max_v_samp_factor * DCTSIZE );
    const int shift( subsampling == _420 || subsampling == _440 ? 1 : 0 );

    const char * pRawYOutput( pYUVData );
    const char * pRawUOutput( pRawYOutput + ySamplesPerRow * rowsToProcess );
    const char * pRawVOutput( pRawUOutput + ( ( uSamplesPerRow * rowsToProcess ) >> shift ) );

    const char ** ppYUV[] = { y, u, v };

    for( int i( 0 ); i < rowsToProcess; i += numLines )
    {
        for( int j( 0 ); j < numLines; ++j )
        {
            y[ j ] = pRawYOutput;
            pRawYOutput += ySamplesPerRow;

            if( j < ( numLines >> shift ) )
            {
                u[ j ] = pRawUOutput;
                v[ j ] = pRawVOutput;
                        
                pRawUOutput += uSamplesPerRow;
                pRawVOutput += vSamplesPerRow;
            }
        }

        const int result( jpeg_write_raw_data( &cinfo, reinterpret_cast<JSAMPIMAGE>( &ppYUV ), numLines ) );
    }
}


void LibJpeg::setSamplingFactors( jpeg_compress_struct & cinfo, const SUBSAMPLING subsampling )const
{
    jpeg_component_info * const pjci( cinfo.comp_info );

    switch( subsampling )
    {
        case _444:
            pjci[ 0 ].h_samp_factor = 1;
            pjci[ 0 ].v_samp_factor = 1;
            break;
        case _422:
            pjci[ 0 ].h_samp_factor = 2;
            pjci[ 0 ].v_samp_factor = 1;
            break;
        case _420:
            pjci[ 0 ].h_samp_factor = 2;
            pjci[ 0 ].v_samp_factor = 2;
            break;
        case _440:
            pjci[ 0 ].h_samp_factor = 1;
            pjci[ 0 ].v_samp_factor = 2;
            break;
    }

    pjci[ 1 ].h_samp_factor = 1;
    pjci[ 1 ].v_samp_factor = 1;
    pjci[ 2 ].h_samp_factor = 1;
    pjci[ 2 ].v_samp_factor = 1;
}


//
// for debugging ONLY
//
// to see what did we get, convert to bitmap file:
// ffmpeg -s 1024x576  -vcodec rawvideo -f rawvideo -pix_fmt yuv444p -i dump.yuv dump.bmp
//
#ifdef __DEBUG__
void LibJpeg::dump2File( const char * const pData, const jpeg_decompress_struct & cinfo )const
{
    if( !pData )
        return;

    if( ( cinfo.image_width & 1 ) || ( cinfo.image_height & 1 ) )
        return; // provide even (width x height)
    
    const int ySamplesPerRow( cinfo.comp_info[ 0 ].width_in_blocks  * DCTSIZE );
    const int yNumRows      ( cinfo.comp_info[ 0 ].height_in_blocks * DCTSIZE );
    const int uSamplesPerRow( cinfo.comp_info[ 1 ].width_in_blocks  * DCTSIZE );
    const int uNumRows      ( cinfo.comp_info[ 1 ].height_in_blocks * DCTSIZE );
    const int vSamplesPerRow( cinfo.comp_info[ 2 ].width_in_blocks  * DCTSIZE );
    const int vNumRows      ( cinfo.comp_info[ 2 ].height_in_blocks * DCTSIZE );

    int yActualBytes;
    int yActualRows;
    int uvActualBytes;
    int uvActualRows;

    switch( subsampling_ )
    {
        case _420:
            yActualBytes  = cinfo.image_width;
            yActualRows   = cinfo.image_height;
            uvActualBytes = cinfo.image_width >> 1;
            uvActualRows  = cinfo.image_height >> 1;
            break;
        case _422:
            yActualBytes  = cinfo.image_width;
            yActualRows   = cinfo.image_height;
            uvActualBytes = cinfo.image_width >> 1;
            uvActualRows  = cinfo.image_height;
            break;
        case _440:
            yActualBytes  = cinfo.image_width;
            yActualRows   = cinfo.image_height;
            uvActualBytes = cinfo.image_width;
            uvActualRows  = cinfo.image_height >> 1;
            break;
        case _444:
            yActualBytes  = cinfo.image_width;
            yActualRows   = cinfo.image_height;
            uvActualBytes = cinfo.image_width;
            uvActualRows  = cinfo.image_height;
            break;
        default:
            return;
    }

    std::ofstream yuvFile{ "dump.yuv", std::ios::out | std::ios::binary };

    const char * pY( pData );
    const char * pU( pData + ySamplesPerRow * yNumRows );
    const char * pV( pU    + uSamplesPerRow * uNumRows );

    for( int i( 0 ); i < yActualRows; ++i )
    {
        yuvFile.write( pY, yActualBytes );
        pY += ySamplesPerRow;
    }

    for( int i( 0 ); i < uvActualRows; ++i )
    {
        yuvFile.write( pU, uvActualBytes );
        pU += uSamplesPerRow;
    }

    for( int i( 0 ); i < uvActualRows; ++i )
    {
        yuvFile.write( pV, uvActualBytes );
        pV += vSamplesPerRow;
    }
    
    yuvFile.close();
}
#endif
