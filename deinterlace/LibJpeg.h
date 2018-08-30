#pragma once
#include <vector>
#include "custom_types.h"

extern "C"
{
#include "jpeglib.h"
};




class LibJpeg
{
public:
    LibJpeg();
    
    LibJpeg( const LibJpeg & ) = delete;
    LibJpeg & operator=( const LibJpeg & ) = delete;

    
    int decompress( const char * const pFileName );
    int compress( const int width,
                  const int height,
                  const SUBSAMPLING subsampling,
                  const int ySamplesPerRow,
                  const int uSamplesPerRow,
                  const int vSamplesPerRow,
                  const int rowsToProcess,
                  const char * pYUVData,
                  const int yuvSize,
                  const int quality,
                  const char * const pFileName );

    const std::vector<char> & get( int & sizeInBytes )const noexcept
    {
        sizeInBytes = imageSize_;
        return pYUVData_;
    }

    SUBSAMPLING subsampling()const noexcept
    {
        return subsampling_;
    }
    
    int width()const noexcept
    {
        return width_;
    }

    int height()const noexcept
    {
        return height_;
    }

    int ySamplesPerRow()const noexcept
    {
        return ySamplesPerRow_;
    }

    int uSamplesPerRow()const noexcept
    {
        return uSamplesPerRow_;
    }

    int vSamplesPerRow()const noexcept
    {
        return vSamplesPerRow_;
    }

    int rowsToProcess()const noexcept
    {
        return numRows_;
    }

private:

    SUBSAMPLING getSampling( const jpeg_decompress_struct * const pjds )const;
    int getImageSize( const jpeg_decompress_struct & cinfo )const;
    void read( jpeg_decompress_struct & cinfo, const SUBSAMPLING subsampling );

    void setSamplingFactors( jpeg_compress_struct & cinfo, const SUBSAMPLING subsampling )const;

    void write( jpeg_compress_struct & cinfo,
                const char * pYUVData,
                const int yuvSize,
                const int ySamplesPerRow,
                const int uSamplesPerRow,
                const int vSamplesPerRow,
                const int rowsToProcess,
                const SUBSAMPLING subsampling );

#ifdef __DEBUG__
    void dump2File( const char * const pData, const jpeg_decompress_struct & cinfo )const; // DEBUG only
#endif

    static void my_error_exit( j_common_ptr cinfo_ptr );


    std::vector<char> pYUVData_;
    int imageSize_;
    SUBSAMPLING subsampling_;
    int width_;
    int height_;

    int ySamplesPerRow_;
    int numRows_;
    int uSamplesPerRow_;
    int vSamplesPerRow_;
};

