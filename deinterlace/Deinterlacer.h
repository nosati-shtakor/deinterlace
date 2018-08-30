#pragma once
#include <vector>
#include "custom_types.h"


class Deinterlacer
{
public:
    Deinterlacer();

    Deinterlacer( const Deinterlacer & ) = delete;
    Deinterlacer & operator=( const Deinterlacer & ) = delete;

    void deinterlace( const unsigned char * pYUVSourceData,
                      const int size,
                      const SUBSAMPLING subsampling,
                      const int ySamplesPerRow,
                      const int uSamplesPerRow,
                      const int vSamplesPerRow,
                      const int rowsToProcess );


    const std::vector<unsigned char> & get( int & size )const noexcept
    {
        size = yuvSize_;
        return yuvData_;
    }
    
private:

    std::vector<unsigned char> yuvData_;
    int yuvSize_;
};
