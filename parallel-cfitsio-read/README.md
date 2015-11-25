
Intel Environment Build
-----------------------

These modules are required to build.  The first 

    module load gcc
    module load cfitsio

The `gcc` module is needed because the source is C++11 and the Intel compiler
still bootstraps `gcc` headers to provide this support.  To build the code then
you should be able to just do

    make

Execution
---------

Like this,

    srun -n 70 ./parallel-cfitsio-read LOGPREFIX path ...

That is, the first argument has to be a logfile prefix.  There are two job
scripts provided that can be used out of box.  First,

    from-scratch.sh

This takes some data from the cosmo data repository (from DECam) and stages it
to Cori scratch and then uses the executable built above to iterate over all
the input files reading the HDUs (header and data units) in parallel.  The
other script,

    from-burst-buffer.sh

takes the same data set from the cosmo data repository and stages it to the
Burst Buffer and runs the executable built above.

Comments on Scratch
-------------------

What I have observed on all the NERSC file systems is that if you run an
executable like this, that does nothing but read individual HDUs in parallel
from a single file one at a time, is that lower-numbered HDUs are read much
faster than higher numbered ones.  Reading an entire DECam image this way is
much faster than reading it in serial, but the performance degradation as a
function of HDU position is remarkable.

Comments on Burst Buffer
------------------------

Running the code on data staged to the burst buffer does not show the same 
trend.  The time to read a single HDU is relatively flat as a function of 
HDU number, though there are some artifacts like breaks.  Unfortunately the
bandwidth per HDU from the burst buffer seems to be low.

General Comments
----------------

The above comments have been made without me looking into re-striping but I 
doubt that will help given the size of the individual FITS files.

The code steps through a list of files provided on the command line and reads
them in parallel, and at the end of each file iteration there is an MPI barrier
to re-synchronize for the next read.  I envision that there would be some
actual processing on each loaded set of frames from an exposure file, that
there would be some real synchronization due to a collective operation to say,
correct for cross-talk.  So I think it is realistic to have this.  Also, if
outputs have to be written to FITS format (and not HDF5) then synchronization
has to happen anyway so the data can be gathered and written in serial.

I note that if this barrier is removed, then the behavior observed on the
scratch file system is mitigated.  That is, as a function of HDU number the
time to read an HDU starts to turn over.  But this is only because the lower
number HDUs are all read before the higher numbered ones and they just idle.
So less reading is going on overall and the higher numbered HDUs seem to get
more bandwidth.

There are some additional things we can tweak and test here, but I have not
messed with them.  Along with some general improvements to the code I have
listed these.  Mainly the OTF data conversion could be made optional.  Reading
fpacked or pre-funpacked data is transparent to the code and so tests could be
run where the data were first funpacked and then read, to compare with the OTF
CFITSIO decompression.
