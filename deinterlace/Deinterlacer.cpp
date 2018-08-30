#include "pch.h"
#ifndef _WIN32
#include <string.h> // Linux
#endif
#include <assert.h>
#include "Deinterlacer.h"


Deinterlacer::Deinterlacer()
:
yuvSize_( 0 )
{
}


void Deinterlacer::deinterlace( const unsigned char * pYUVSourceData,
                                const int size,
                                const SUBSAMPLING subsampling,
                                const int ySamplesPerRow,
                                const int uSamplesPerRow,
                                const int vSamplesPerRow,
                                const int rowsToProcess )
{
    // "Blending" algorithm: line N in the output image is the average of lines N and N-1 from the input image.
    // The first line stays unchanged

    
    if( yuvData_.size() < size )
        yuvData_.resize( size );

    yuvSize_ = size;

    // Y
    const int numItemsY( ySamplesPerRow * rowsToProcess );
    const unsigned char * pSource1( pYUVSourceData );
    const unsigned char * pSource2( pSource1 + ySamplesPerRow );
    const unsigned char * pSourceEnd( pSource1 + numItemsY );
    
    unsigned char * pDest( yuvData_.data() );
    
    memcpy( pDest, pSource1, ySamplesPerRow ); // copy luma values for the first line
    pDest += ySamplesPerRow;
    
    //
    // dummy loop - this could be refactored as for-loop with OpenMP enabled. Profiling would tell us if it's worth the trouble...
    //
    while( pSource2 != pSourceEnd )
        *pDest++ = static_cast<unsigned char>( ( *pSource1++ + *pSource2++ ) >> 1 );
    
    const int uvSamplesPerRow[] = { uSamplesPerRow, vSamplesPerRow };
    // const int numItemsUV     [] = { uSamplesPerRow * rowsToProcess, vSamplesPerRow * rowsToProcess }; // 4:4:4 & 4:2:2
    // const int numItemsUV[] = { uSamplesPerRow * rowsToProcess / 2, vSamplesPerRow * rowsToProcess / 2 };         // 4:2:0 & 4:4:0

    const int denominator( subsampling == _444 || subsampling == _422 ? 0 : 1 );
    const int numItemsUV[] = { ( uSamplesPerRow * rowsToProcess ) >> denominator, ( vSamplesPerRow * rowsToProcess ) >> denominator };

    for( int i( 0 ); i < 2; ++i )
    {
        // U, V
        memcpy( pDest, pSource2, uvSamplesPerRow[ i ] ); // copy chroma values for the first line
        pDest += uvSamplesPerRow[ i ];

        pSource1 += uvSamplesPerRow[ i ];
        pSource2 += uvSamplesPerRow[ i ];

        pSourceEnd += numItemsUV[ i ];

        while( pSource2 != pSourceEnd )
            *pDest++ = static_cast<unsigned char>( ( *pSource1++ + *pSource2++ ) >> 1 );
    }

    assert( pSource2 <= pYUVSourceData + size && pDest <= yuvData_.data() + yuvSize_ );
}
