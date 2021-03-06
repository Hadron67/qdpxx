/*! @file
 * @brief Scalar layout routines
 * 
 * Layout routines for scalar implementation
 * QDP data parallel interface
 *
 * Layout
 *
 * This routine provides various layouts, including
 *    lexicographic
 *    2-checkerboard  (even/odd-checkerboarding of sites)
 *    32-style checkerboard (even/odd-checkerboarding of hypercubes)
 */

#include "qdp_diagnostics.h"

#include "qdp.h"
#include "qdp_util.h"
#include "qdp_allocator.h"

namespace QDP 
{
  float pool_size_in_gb = 8.0;

  //-----------------------------------------------------------------------------
  // Layout stuff specific to a scalar architecture
  namespace Layout
  {
    //-----------------------------------------------------
    //! Local data specific to a scalar architecture
    /*! 
     * NOTE: the disadvantage to using a struct to keep things together is
     * that subsequent groupings of namespaces can not just add onto the
     * current namespace. 
     */
    struct LocalLayout_t
    {
      //! Total lattice volume
      int vol;

      //! Lattice size
      multi1d<int> nrow;

      //! Subgrid lattice volume
      int subgrid_vol;

      //! Logical node coordinates
      multi1d<int> logical_coord;

      //! Logical system size
      multi1d<int> logical_size;
		
      //! IO Grid size
      multi1d<int> iogrid;

    } _layout;


    //-----------------------------------------------------
    // Functions

    //! Main destruction routine
    void destroy() {RNG::finalizeRNG();}

    //! Set virtual grid (problem grid) lattice size
    void setLattSize(const multi1d<int>& nrows) {_layout.nrow = nrows;}

    //! Set SMP flag -- true if using smp/multiprocessor mode on a node
    /*! For now, this is ignored */
    void setSMPFlag(bool flag) {}

    //! Set number of processors in a multi-threaded implementation
    /*! For now, this is ignored */
    void setNumProc(int N) {}

    //! Virtual grid (problem grid) lattice size
    const multi1d<int>& lattSize() {return _layout.nrow;}

    //! Total lattice volume
    int vol() {return _layout.vol;}

    //! Subgrid lattice volume
    int sitesOnNode() {return _layout.subgrid_vol;}

    //! Returns whether this is the primary node
    /*! Always true on a scalar platform */
    bool primaryNode() {return true;}

    //! Subgrid (grid on each node) lattice size
    const multi1d<int>& subgridLattSize() {return _layout.nrow;}

    //! Returns the node number of this node
    int nodeNumber() {return 0;}

    //! Returns the logical node number for the corresponding lattice coordinate
    int nodeNumber(const multi1d<int>& coord) {return 0;}

    //! Returns the number of nodes
    int numNodes() {return 1;}

    //! Returns the logical node coordinates for this node
    const multi1d<int>& nodeCoord() {return _layout.logical_coord;}

    //! Returns the logical size of this machine
    const multi1d<int>& logicalSize() {return _layout.logical_size;}

    //! Returns the node number given some logical node coordinate
    /*! This is not meant to be speedy */
    int getNodeNumberFrom(const multi1d<int>& node_coord) {return 0;}

    //! Returns the logical node coordinates given some node number
    /*! This is not meant to be speedy */
    multi1d<int> getLogicalCoordFrom(int node) 
    {
      multi1d<int> node_coord(Nd);
      node_coord = 0;
      return node_coord;
    }

	
	  
    //! check if I/O grid is defined
    /*! Always defined for scalar node: it is 1x1x1x1 */
    bool isIOGridDefined(void) QDP_CONST 
    { 
      return true; 
    }
	  
    //! number of I/O nodes
    int numIONodeGrid(void) QDP_CONST
    {
      return 1;
    }
	  
    //! Default initializer
    void setIONodeGridDefaults()
    {	  
      // Scalar machine: It's 1x1x1x1
      _layout.iogrid.resize(Nd);
      for(int i=0; i < Nd; i++) { 
	_layout.iogrid[i] = 1;
      }		
    }

    //! Set the I/O Node grid -- satisfy interface
    void setIONodeGrid(const multi1d<int>& io_grid) 
    {
      // Completely	ignore user supplied grid :)
      // Its scalar	
      setIONodeGridDefaults();	
    }
	
    //! Get the I/O Node grid
    const multi1d<int>& getIONodeGrid() QDP_CONST
    {
      return _layout.iogrid;
    }
	  
    //! Initializer for layout
    void init() {}

    //! The linearized site index for the corresponding lexicographic site
    int linearSiteIndex(int lexicosite)
    {
      return linearSiteIndex(crtesn(lexicosite, lattSize()));
    }

    //! Initializer for all the layout defaults
    void initDefaults()
    {
      // Default set and subsets
      initDefaultSets();

      // Default maps
      initDefaultMaps();

      // Initialize RNG
      RNG::initDefaultRNG();

      // Set default profile level
      setProfileLevel(getProgramProfileLevel());
	
      // Set IO node grid defaults.
      setIONodeGridDefaults();	
    }

    //! Initializer for layout
    void create()
    {
      if ( ! QDP_isInitialized() )
	QDP_error_exit("QDP is not initialized");

      if (_layout.nrow.size() != Nd)
	QDP_error_exit("dimension of lattice size not the same as the default");

      _layout.vol=1;

      for(int i=0; i < Nd; ++i) 
	_layout.vol *= _layout.nrow[i];

      _layout.subgrid_vol = _layout.vol;
  
      _layout.logical_coord.resize(Nd);
      _layout.logical_size.resize(Nd);

      _layout.logical_coord = 0;
      _layout.logical_size = 1;

#if QDP_DEBUG >= 2
      fprintf(stderr,"vol=%d\n",_layout.vol);
#endif

      // Diagnostics
      QDPIO::cout << "Lattice initialized:\n";
      QDPIO::cout << "  problem size =";
      for(int i=0; i < Nd; ++i)
	QDPIO::cout << " " << _layout.nrow[i];
      QDPIO::cout << std::endl;

      // Same as the problem size... we are scalar
      QDPIO::cout << "  layout size =";
      for(int i=0; i < Nd; ++i)
	QDPIO::cout << " " << _layout.nrow[i];
      QDPIO::cout << std::endl;

      QDPIO::cout << "  logical machine size =";
      for(int i=0; i < Nd; ++i)
	QDPIO::cout << " " << (int)1;
      QDPIO::cout << std::endl;

      QDPIO::cout << "  subgrid size =";
      for(int i=0; i < Nd; ++i)
	QDPIO::cout << " " << _layout.nrow[i];
      QDPIO::cout << std::endl;

      QDPIO::cout << "  total number of nodes = " << 1 << std::endl;
      QDPIO::cout << "  total volume = " << _layout.vol << std::endl;
      QDPIO::cout << "  subgrid volume = " << _layout.vol << std::endl;


      // Sanity check - check the layout functions make sense
#pragma omp parallel for
      for(int i=0; i < _layout.vol; ++i) 
      {
	int j = Layout::linearSiteIndex(Layout::siteCoords(Layout::nodeNumber(),i));

#if QDP_DEBUG >= 3
	{
	  fprintf(stderr,"call nodenumber\n");
	  int noden = Layout::nodeNumber();
	  fprintf(stderr,"call sitecoords\n");
	  multi1d<int> coord = Layout::siteCoords(noden,i);
	  fprintf(stderr,"call linearsiteindex\n");
	  int j = Layout::linearSiteIndex(coord);
	  fprintf(stderr,"site= %d   coord= %d %d %d %d   j= %d node=%d\n",
		  i,coord[0],coord[1],coord[2],coord[3],
		  j,noden);
	}
#endif

	if (i != j)
	  QDP_error_exit("Layout::create - Layout problems, the layout functions do not work correctly with this lattice size");
      }
     size_t pool_size_in_MB = static_cast<size_t>(floor(pool_size_in_gb*1024.0));

      Allocator::theQDPAllocator::Instance().init(pool_size_in_MB);
      // Initialize various defaults
      initDefaults();

      QDPIO::cout << "Finished lattice layout" << std::endl;
    }
  }


  //-----------------------------------------------------------------------------
#if QDP_USE_LEXICO_LAYOUT == 1

QDPXX_MESSAGE("Using a lexicographic layout")

  namespace Layout
  {
    //! Reconstruct the lattice coordinate from the node and site number
    /*! 
     * This is the inverse of the nodeNumber and linearSiteIndex functions.
     * The API requires this function to be here.
     */
    multi1d<int> siteCoords(int node, int linearsite) // ignore node
    {
      return crtesn(linearsite, lattSize());
    }

    //! The linearized site index for the corresponding coordinate
    /*! This layout is a simple lexicographic lattice ordering */
    int linearSiteIndex(const multi1d<int>& coord)
    {
      return local_site(coord, lattSize());
    }
  }


  //-----------------------------------------------------------------------------

#elif QDP_USE_CB2_LAYOUT == 1

QDPXX_MESSAGE("Using a 2 checkerboard (red/black) layout")

  namespace Layout
  {
    //! Reconstruct the lattice coordinate from the node and site number
    /*! 
     * This is the inverse of the nodeNumber and linearSiteIndex functions.
     * The API requires this function to be here.
     */
    multi1d<int> siteCoords(int node, int linearsite) // ignore node
    {
      int vol_cb = vol() >> 1;
      multi1d<int> cb_nrow = lattSize();
      cb_nrow[0] >>= 1;

      int cb = linearsite / vol_cb;
      multi1d<int> coord = crtesn(linearsite % vol_cb, cb_nrow);

      int cbb = cb;
      for(int m=1; m<coord.size(); ++m)
	cbb += coord[m];
      cbb = cbb & 1;

      coord[0] = 2*coord[0] + cbb;

      return coord;
    }

    //! The linearized site index for the corresponding coordinate
    /*! This layout is appropriate for a 2 checkerboard (red/black) lattice */
    int linearSiteIndex(const multi1d<int>& coord)
    {
      int vol_cb = vol() >> 1;
      multi1d<int> cb_nrow = lattSize();
      cb_nrow[0] >>= 1;

      multi1d<int> cb_coord = coord;

      cb_coord[0] >>= 1;    // Number of checkerboards
    
      int cb = 0;
      for(int m=0; m<coord.size(); ++m)
	cb += coord[m];
      cb = cb & 1;

      return local_site(cb_coord, cb_nrow) + cb*vol_cb;
    }
  }



  //-----------------------------------------------------------------------------

#elif QDP_USE_CB3D_LAYOUT == 1

QDPXX_MESSAGE("Using a 2 checkerboard (red/black) layout but in 3D. Time running fastest")

  namespace Layout
  {

   
    //! Reconstruct the lattice coordinate from the node and site number
    /*! 
     * This is the inverse of the nodeNumber and linearSiteIndex functions.
     * The API requires this function to be here.
     *
     * NB: Time is local and fastest running 
     */

    multi1d<int> siteCoords(int node, int linearsite) // ignore node
    {
      int vol_cb = vol() / 2;
      multi1d<int> cb_nrow = lattSize();
      cb_nrow[0] /=2;

      int cb = linearsite / vol_cb;

      // This now uses crtesn with the t running fastest
      multi1d<int> coord = crtesn(linearsite % vol_cb, cb_nrow);

      int cbb = cb;
      for(int m=1; m<coord.size()-1; ++m) // Nd-1 checkerboard
	cbb += coord[m];

      cbb = cbb & 1;

      coord[0] = 2*coord[0] + cbb;

      return coord;
    }

    //! The linearized site index for the corresponding coordinate
    /*! This layout is appropriate for a 2 checkerboard (red/black) lattice */
    int linearSiteIndex(const multi1d<int>& coord)
    {
      int vol_cb = vol() / 2;
      multi1d<int> cb_nrow = lattSize();
      cb_nrow[0] /= 2;

      multi1d<int> cb_coord = coord;

      cb_coord[0] /= 2;    // Number of checkerboards
    
      int cb = 0;
      for(int m=0; m<coord.size()-1; ++m) // 3d checkerboard
	cb += coord[m];

      cb = cb & 1;

      // Use funky local site 
      return local_site(cb_coord, cb_nrow) + cb*vol_cb;
    }
  }


  //-----------------------------------------------------------------------------

#elif QDP_USE_CB32_LAYOUT == 1

QDPXX_MESSAGE("Using a 32 checkerboard layout")

  namespace Layout
  {
    //! Reconstruct the lattice coordinate from the node and site number
    /*! 
     * This is the inverse of the nodeNumber and linearSiteIndex functions.
     * The API requires this function to be here.
     */
    multi1d<int> siteCoords(int node, int linearsite) // ignore node
    {
      int vol_cb = vol() >> (Nd+1);
      multi1d<int> cb_nrow(Nd);
      cb_nrow[0] = lattSize()[0] >> 2;
      for(int i=1; i < Nd; ++i) 
	cb_nrow[i] = lattSize()[i] >> 1;

      int subl = linearsite / vol_cb;
      multi1d<int> coord = crtesn(linearsite % vol_cb, cb_nrow);

      int cb = 0;
      for(int m=1; m<Nd; ++m)
	cb += coord[m];
      cb &= 1;

      coord[0] <<= 2;
      for(int m=1; m<Nd; ++m)
	coord[m] <<= 1;

      subl ^= (cb << Nd);
      for(int m=0; m<Nd; ++m)
	coord[m] ^= (subl & (1 << m)) >> m;
      coord[0] ^= (subl & (1 << Nd)) >> (Nd-1);   // this gets the hypercube cb

      return coord;
    }

    //! The linearized site index for the corresponding coordinate
    /*! This layout is appropriate for a 32-style checkerboard lattice */
    int linearSiteIndex(const multi1d<int>& coord)
    {
      int vol_cb = vol() >> (Nd+1);
      multi1d<int> cb_nrow(Nd);
      cb_nrow[0] = lattSize()[0] >> 2;
      for(int i=1; i < Nd; ++i) 
	cb_nrow[i] = lattSize()[i] >> 1;

      int subl = coord[Nd-1] & 1;
      for(int m=Nd-2; m >= 0; --m)
	subl = (subl << 1) + (coord[m] & 1);

      int cb = 0;
      for(int m=0; m < Nd; ++m)
	cb += coord[m] >> 1;

      subl += (cb & 1) << Nd;   // Final color or checkerboard

      // Construct the checkerboard lattice coord
      multi1d<int> cb_coord(Nd);

      cb_coord[0] = coord[0] >> 2;
      for(int m=1; m < Nd; ++m)
	cb_coord[m] = coord[m] >> 1;

      return local_site(cb_coord, cb_nrow) + subl*vol_cb;
    }
  }

#else

#error "no appropriate layout defined"

#endif

  //-----------------------------------------------------------------------------


} // namespace QDP;
