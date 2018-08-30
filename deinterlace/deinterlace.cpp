// deinterlace.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "LibJpeg.h"
#include "Deinterlacer.h"


#ifdef linux
#else
#ifdef _WIN64
#ifdef _DEBUG
#pragma comment( lib, "x64/Debug/libjpeg.lib" )
#else
#pragma comment( lib, "x64/Release/libjpeg.lib" )
#endif
#else
#error "sorry, x86 not supported for now..."
#endif
#endif


int main( int argc, char ** argv )
{
    if( argc != 3 )
    {
        std::cout << "Usage: " << argv[ 0 ] << " interlaced.jpg deinterlaced.jpg" << std::endl;
        return 0;
    }

    LibJpeg jpeg;

    if( jpeg.decompress( argv[ 1 ] ) != 0 )
    {
        std::cout << "Image decompression failed!\n";
        return 0;
    }

    int yuvSizeBytes;
    const std::vector<char> & yuvData( jpeg.get( yuvSizeBytes ) );

    Deinterlacer deinterlacer;
    deinterlacer.deinterlace( reinterpret_cast<const unsigned char*>( yuvData.data() ),
                              yuvSizeBytes,
                              jpeg.subsampling(),
                              jpeg.ySamplesPerRow(),
                              jpeg.uSamplesPerRow(),
                              jpeg.vSamplesPerRow(),
                              jpeg.rowsToProcess() );
    
    const std::vector<unsigned char> & deinterlacedYuvData( deinterlacer.get( yuvSizeBytes ) );

    if( jpeg.compress( jpeg.width(),
                       jpeg.height(),
                       jpeg.subsampling(),
                       jpeg.ySamplesPerRow(),
                       jpeg.uSamplesPerRow(),
                       jpeg.vSamplesPerRow(),
                       jpeg.rowsToProcess(),
                       reinterpret_cast<const char*>( deinterlacedYuvData.data() ),
                       yuvSizeBytes,
                       95,
                       argv[ 2 ] ) != 0 )
        std::cout << "Image compression failed!\n";

    return 0;
}
