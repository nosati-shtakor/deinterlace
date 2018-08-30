#pragma once
// #include <type_traits>


namespace RAII
{

class AUTO_FOPEN
{
public:
    AUTO_FOPEN( const char * const pFileName, const char * const pMode ) : f_( fopen( pFileName, pMode ) ){}

    AUTO_FOPEN( const AUTO_FOPEN & ) = delete;
    AUTO_FOPEN & operator=( const AUTO_FOPEN & ) = delete;

    //
    // TO DO: moving?
    //

    ~AUTO_FOPEN()
    {
        if( f_ )
            fclose( f_ );
    }

    FILE * get()const noexcept
    {
        return f_;
    }

    bool valid()const noexcept
    {
        return f_ != nullptr;
    }

private:
    FILE * f_;
};


//
// note: this is a class template because this way we can include this file in any
//       environment with no libjpeg sources at all and still use the rest of the RAII classes.
//
template<typename TJPEG_COMP_DECOMP,
         void( * const pJpeg_create_fun )( TJPEG_COMP_DECOMP *, int, size_t ),
         void( * const pJpeg_destroy_fun )( TJPEG_COMP_DECOMP * )>
class AUTO_JPEG_CONTEXT
{
public:
    explicit AUTO_JPEG_CONTEXT( const int version )
    {
        static_assert( pJpeg_create_fun && pJpeg_destroy_fun, "I hope you are wearing helmet..." );
        pJpeg_create_fun( &cinfo_, version, sizeof( cinfo_ ) );
    }

    AUTO_JPEG_CONTEXT( const AUTO_JPEG_CONTEXT & ) = delete;
    AUTO_JPEG_CONTEXT & operator=( const AUTO_JPEG_CONTEXT & ) = delete;

    ~AUTO_JPEG_CONTEXT()
    {
        pJpeg_destroy_fun( &cinfo_ );
    }

    TJPEG_COMP_DECOMP * get()noexcept
    {
        return &cinfo_;
    }

private:

    TJPEG_COMP_DECOMP cinfo_ = { 0 };
};


template<typename TJPEG_COMP_STRUCT, typename T2,
         void( * const pJpeg_start_fun )( TJPEG_COMP_STRUCT *, T2 ),
         void( * const pJpeg_finish_fun)( TJPEG_COMP_STRUCT * )>
class AUTO_JPEG_START_FINISH_COMPRESS
{
public:
    AUTO_JPEG_START_FINISH_COMPRESS( TJPEG_COMP_STRUCT * const pcinfo, const T2 writeAllTables )
    :
    pcinfo_( pcinfo )
    {
        static_assert( pJpeg_start_fun && pJpeg_finish_fun, "I hope you are wearing helmet..." );
        pJpeg_start_fun( pcinfo, writeAllTables );
    }

    AUTO_JPEG_START_FINISH_COMPRESS( const AUTO_JPEG_START_FINISH_COMPRESS & ) = delete;
    AUTO_JPEG_START_FINISH_COMPRESS & operator=( const AUTO_JPEG_START_FINISH_COMPRESS & ) = delete;

    ~AUTO_JPEG_START_FINISH_COMPRESS()
    {
        pJpeg_finish_fun( pcinfo_ );
    }

private:
    TJPEG_COMP_STRUCT * const pcinfo_;
};


} // namespace RAII
