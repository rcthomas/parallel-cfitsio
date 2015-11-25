// File    : parallel-cfitsio-read.cc
// ----------------------------------
// Created : Wed Nov 25 14:19:14 2015
// Authors : Rollin C. Thomas (RCT) - rcthomas@lbl.gov
// 
// MPI parallel read test of multi-extension FITS files with CFITSIO library.
// This is designed around using raw DECam exposure files available in the 
// Cosmo Data Repository.  These could be in the standard fpacked format or
// pre-uncompressed with funpack.  A comparison could be useful.
//
// Items:
// * Option parsing (primarily for log prefix), default value for log prefix.
// * Better management of timing data than just separate files for each rank.
// * Make on-the-fly data conversion an option for testing if it matters.

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <sstream>

#include <fitsio.h>
#include <mpi.h>

// Declarations.

void assert_mpi_status( const int mpi_status );
void assert_cfitsio_status( const int cfitsio_status );
std::string log_path( const std::string& prefix, const int mpi_rank );

// Straightforward main function.

int main( int argc, char* argv[] )
{

    // Initialize MPI.

    int mpi_status = MPI_Init( &argc, &argv );
    assert_mpi_status( mpi_status );

    // Size of world communicator.

    int mpi_size;
    mpi_status = MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
    assert_mpi_status( mpi_status );

    // Rank of this process.

    int mpi_rank;
    mpi_status = MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    assert_mpi_status( mpi_status );

    // Initiate logging for this MPI rank.  Contents are the HDU position this
    // rank reads and the time it took in milliseconds to open, read, close.

    std::ofstream log( log_path( argv[ 1 ], mpi_rank ) );

    // Iterate over all the file paths passed on the command line and simply
    // read an assigned HDU out of each.  There is a barrier at the end of the
    // loop body.  See README.md for comments on this.

    for( auto i = 2; i < argc; ++i )
    {

        // Start timer.

        auto start = std::chrono::steady_clock::now();

        // Open FITS file.

        fitsfile* fptr;
        int cfitsio_status = 0;
        fits_open_file( &fptr, argv[ i ], READONLY, &cfitsio_status );
        assert_cfitsio_status( cfitsio_status );

        // Move to relevant HDU.  This is just MPI rank plus 2.

        int hdunum = mpi_rank + 2;
        int hdutype = 0;
        fits_movabs_hdu( fptr, hdunum, &hdutype, &cfitsio_status );
        assert_cfitsio_status( cfitsio_status );

        // HDU image parameters.

        int bitpix = 0;
        fits_get_img_type( fptr, &bitpix, &cfitsio_status );
        assert_cfitsio_status( cfitsio_status );

        int naxis = 0;
        fits_get_img_dim( fptr, &naxis, &cfitsio_status );
        assert_cfitsio_status( cfitsio_status );

        int maxdim = 2;
        long* naxes = new long [ naxis ];
        fits_get_img_size( fptr, maxdim, naxes, &cfitsio_status );
        assert_cfitsio_status( cfitsio_status );

        // Allocate data array.  Right now do conversion to double but we can also test not doing this.

        int datatype = TDOUBLE;
        long* fpixel = new long [ naxis ];
        std::fill( fpixel, fpixel + naxis, 1 );
        long nelements = std::accumulate( naxes, naxes + naxis, 1, std::multiplies< long >() );
        double* array = new double [ nelements ];
        int anynul;

        fits_read_pix( fptr, datatype, fpixel, nelements, nullptr, array, &anynul, &cfitsio_status );
        assert_cfitsio_status( cfitsio_status );

        // Close FITS file.

        fits_close_file( fptr, &cfitsio_status );
        assert_cfitsio_status( cfitsio_status );

        // Log time to process in milliseconds.

        auto diff = std::chrono::steady_clock::now() - start;
        log << hdunum << " " << std::chrono::duration< double, std::milli >( diff ).count() << std::endl;

        // Clean up.

        delete [] naxes;
        delete [] fpixel;
        delete [] array;

        // Wait for everyone else.

        mpi_status = MPI_Barrier( MPI_COMM_WORLD );
        assert_mpi_status( mpi_status );

    }

    // Shutdown.

    log.close();
    mpi_status = MPI_Finalize();
    assert_mpi_status( mpi_status );
    return EXIT_SUCCESS;

}

void assert_mpi_status( const int mpi_status )
{

    if( mpi_status == 0 ) return;

    int error_class;
    MPI_Error_class( mpi_status, &error_class );

    char error_string[ MPI_MAX_ERROR_STRING ];
    int length;
    MPI_Error_string( mpi_status, error_string, &length );

    std::stringstream ss;
    ss << "MPI error code " << mpi_status;
    ss << " | error class " << error_class;
    ss << " | "             << error_string;

    throw std::runtime_error( ss.str() );

}

void assert_cfitsio_status( const int cfitsio_status )
{

    if( cfitsio_status == 0 ) return;

    char status_message[ FLEN_STATUS ];
    fits_get_errstatus( cfitsio_status, status_message );
    
    std::stringstream ss;
    ss << "CFITSIO error code " << cfitsio_status;
    ss << " | "                 << status_message;

    throw std::runtime_error( ss.str() );

}

std::string log_path( const std::string& prefix, const int mpi_rank )
{
    std::stringstream ss;
    ss.fill( '0' );
    ss.width( 3 );
    ss << mpi_rank;
    return prefix + "." + ss.str() + ".log";
}
