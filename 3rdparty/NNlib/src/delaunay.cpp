/******************************************************************************
 *
 * File:           delaunay.c
 *
 * Created:        04/08/2000
 *
 * Author:         Pavel Sakov
 *                 CSIRO Marine Research
 *
 * Purpose:        Delaunay triangulation - a wrapper to triangulate()
 *
 * Description:    None
 *
 * Revisions:      10/06/2003 PS: delaunay_build(); delaunay_destroy();
 *                   struct delaunay: from now on, only shallow copy of the
 *                   input data is contained in struct delaunay. This saves
 *                   memory and is consistent with libcsa.
 *                 30/10/2007 PS: added delaunay_addflag() and
 *                   delaunay_resetflags(); modified delaunay_circles_find()
 *                   to reset the flags to 0 on return. This is very important
 *                   for large datasets, many thanks to John Gerschwitz,
 *                   Petroleum Geo-Services, for identifying the problem.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include "istack.h"
#include "nan.h"
#include "delaunay.h"
#include "nn.h"
#include "nn_internal.h"

/*
 * This parameter is used in search of tricircles containing a given point:
 *   if there are no more triangles than N_SEARCH_TURNON
 *     do linear search
 *   else
 *     do more complicated stuff
 */
#define N_SEARCH_TURNON 20
#define N_FLAGS_TURNON 1000
#define N_FLAGS_INC 100

// Boost polygon allows to remove the proprietary software Triangle.C by using its voronoi builder to create the Delaunay triangulation. 
#include <boost/polygon/voronoi.hpp>
#include <boost/polygon/point_data.hpp>
#include <boost/polygon/segment_data.hpp>
using namespace boost::polygon;

//STL
#include <functional>

namespace triangulation
{
   typedef struct
   {
      // the triangle vertices
      int v1;
      int v2;
      int v3;
      // the indexes of the neighbouring triangles 
      int n1;
      int n2;
      int n3;
   } triangle;

   typedef struct 
   {
      // the edge vertices
      int p1;
      int p2;
   } edge;

   const double eps = std::numeric_limits<double>::epsilon( );

   inline bool operator== ( const edge& lhs, const edge& rhs )
   {
      if ( lhs.p1 == rhs.p1  &&  lhs.p2 == rhs.p2 )
         return true;
      if ( lhs.p1 == rhs.p2  &&  lhs.p2 == rhs.p1 )
         return true;
      return false;
   }

   inline bool operator== ( const triangle  t1, const triangle  t2 )
   {
      // create two int array to be sorted
      std::vector<int> t1v, t2v;
      t1v.push_back( t1.v1 );
      t1v.push_back( t1.v2 );
      t1v.push_back( t1.v3 );
      t2v.push_back( t2.v1 );
      t2v.push_back( t2.v2 );
      t2v.push_back( t2.v3 );

      // sort the array 
      std::sort( t1v.begin( ), t1v.end( ), std::greater<int>( ) );
      std::sort( t2v.begin( ), t2v.end( ), std::greater<int>( ) );

      // compare the vertices
      if ( t1v[0] != t2v[0] ) return  false;
      if ( t1v[1] != t2v[1] ) return  false;
      if ( t1v[2] != t2v[2] ) return  false; 

      return true;
   }

   // Loop over the voronoi cells and connect the centers of the cells sharing the same edge.
   // Skip the triangle if both heighbouring edges lies to infinity.
   void triangulateFromVoronoi( const std::vector<point_data<double>> & points, std::vector<triangle>& dTriang )
   {
      voronoi_diagram<double> vd;
      construct_voronoi( points.begin( ), points.end( ), &vd );

      for ( size_t ic = 0; ic != vd.cells( ).size( ); ++ic )
      {
         if ( vd.cells( )[ic].contains_point( ) )
         {
            int pointInd = vd.cells( )[ic].source_index( );
            double xc = x( points[pointInd] );
            double yc = y( points[pointInd] );
            const voronoi_edge<double>* edge = vd.cells( )[ic].incident_edge( );
            do
            {
               // Next edge in CCW order
               edge = edge->next( ); 
               const voronoi_edge<double>::voronoi_edge_type* currentEdge = edge;
               const voronoi_edge<double>::voronoi_edge_type* nextEdge = edge->next( );
               int boundEdge = 0;
               if ( currentEdge->is_infinite( ) ) boundEdge += 1;
               if ( nextEdge->is_infinite( ) )   boundEdge += 1;
               if ( boundEdge == 2 ) continue;
               // The new triangle. The neighbouring triangles (n1, n2, n3) will be determined later
               triangle t;
               t.n1 = -1;
               t.n2 = -1;
               t.n3 = -1;
               t.v1 = pointInd;
               t.v2 = currentEdge->twin( )->cell( )->source_index( );
               t.v3 = nextEdge->twin( )->cell( )->source_index( );
               if ( std::find( dTriang.begin( ), dTriang.end( ), t ) == dTriang.end( ) ) dTriang.push_back( t );
            } while ( edge != vd.cells( )[ic].incident_edge( ) );
         }
      }

      // Determine the neighbouring triangles. le is the vector of the local triangle edges, ge is the vector of the global triangle edges, to compare against to 
      edge le[3];
      edge ge[3];
      for ( size_t it = 0; it != dTriang.size( ); ++it )
      {
         le[0].p1 = dTriang[it].v1;
         le[0].p2 = dTriang[it].v2;
         le[1].p1 = dTriang[it].v2;
         le[1].p2 = dTriang[it].v3;
         le[2].p1 = dTriang[it].v3;
         le[2].p2 = dTriang[it].v1;

         for ( size_t iti = 0; iti != dTriang.size( ); ++iti )
         {

            ge[0].p1 = dTriang[iti].v1;
            ge[0].p2 = dTriang[iti].v2;
            ge[1].p1 = dTriang[iti].v2;
            ge[1].p2 = dTriang[iti].v3;
            ge[2].p1 = dTriang[iti].v3;
            ge[2].p2 = dTriang[iti].v1;

            if ( iti != it )
            {
               // Use the same order for neighbour as in triangle.C. This is important for the search algorithm implemented later.
               if ( ( le[0] == ge[0] || le[0] == ge[1] || le[0] == ge[2] ) ) { dTriang[it].n3 = iti; }
               else if ( ( le[1] == ge[0] || le[1] == ge[1] || le[1] == ge[2] ) ) { dTriang[it].n1 = iti; }
               else if ( ( le[2] == ge[0] || le[2] == ge[1] || le[2] == ge[2] ) ) { dTriang[it].n2 = iti; }

               if ( dTriang[it].n1 != -1 && dTriang[it].n2 != -1 && dTriang[it].n3 != -1 ) break;
            }
         }
      }
   }
}

static delaunay* delaunay_create()
{
   delaunay* d = (delaunay*)malloc( sizeof( delaunay ) );

    d->npoints = 0;
    d->points = NULL;
    d->xmin = DBL_MAX;
    d->xmax = -DBL_MAX;
    d->ymin = DBL_MAX;
    d->ymax = -DBL_MAX;
    d->ntriangles = 0;
    d->triangles = NULL;
    d->circles = NULL;
    d->neighbours = NULL;
    d->n_point_triangles = NULL;
    d->point_triangles = NULL;
    d->nedges = 0;
    d->edges = NULL;
    d->flags = NULL;
    d->first_id = -1;
    d->t_in = NULL;
    d->t_out = NULL;
    d->nflags = 0;
    d->nflagsallocated = 0;
    d->flagids = NULL;

    return d;
}

// This function uses the Delaunay triangulation computed from the boost voronoi builder
static void tio2delaunayTriangulation( delaunay* d )
{

   int i, j;

   for ( i = 0, j = 0; i < d->npoints; ++i ) {
      point* p = &d->points[i];

      if ( p->x < d->xmin )
         d->xmin = p->x;
      if ( p->x > d->xmax )
         d->xmax = p->x;
      if ( p->y < d->ymin )
         d->ymin = p->y;
      if ( p->y > d->ymax )
         d->ymax = p->y;
   }
   if ( nn_verbose ) {
      fprintf( stderr, "input:\n" );
      for ( i = 0, j = 0; i < d->npoints; ++i ) {
         point* p = &d->points[i];

         fprintf( stderr, "  %d: %15.7g %15.7g %15.7g\n", i, p->x, p->y, p->z );
      }
   }

   // The vector of points must not have duplicates
   std::vector<point_data<double>> points;
   for ( i = 0; i < d->npoints; ++i )
   {
      point_data<double> newPoint( d->points[i].x, d->points[i].y );
      if ( std::find( points.begin( ), points.end( ), newPoint ) == points.end( ) ) points.push_back( newPoint );
   }
   std::vector<triangulation::triangle> dTriang;
   triangulation::triangulateFromVoronoi( points, dTriang );

   d->ntriangles = dTriang.size( );

   if ( d->ntriangles > 0 )
   {
      d->triangles = (triangle *)malloc( d->ntriangles * sizeof( triangle ) );
      d->neighbours = (triangle_neighbours *)malloc( d->ntriangles * sizeof( triangle_neighbours ) );
      d->circles = (circle *)malloc( d->ntriangles * sizeof( circle ) );
      d->n_point_triangles = (int *)calloc( d->npoints, sizeof( int ) );
      d->point_triangles = (int **)malloc( d->npoints * sizeof( int* ) );
      d->flags = (int *)calloc( d->ntriangles, sizeof( int ) );
   }

   //edges
   std::vector<triangulation::edge>   globalEdges;
   std::vector<triangulation::edge>   localEdges( 3 );

   if ( nn_verbose )
      fprintf( stderr, "triangles:\n" );
   for ( size_t it = 0; it != dTriang.size( ); ++it )
   {
      triangle* t = &d->triangles[it];
      triangle_neighbours* n = &d->neighbours[it];
      circle* c = &d->circles[it];
      int status;

      // save the triangles
      t->vids[0] = dTriang[it].v1;
      t->vids[1] = dTriang[it].v2;
      t->vids[2] = dTriang[it].v3;

      // save the unique edges
      triangulation::edge ed1, ed2, ed3;

      ed1.p1 = dTriang[it].v1;
      ed1.p2 = dTriang[it].v2;
      ed2.p1 = dTriang[it].v2;
      ed2.p2 = dTriang[it].v3;
      ed3.p1 = dTriang[it].v3;
      ed3.p2 = dTriang[it].v1;

      localEdges[0] = ed1;
      localEdges[1] = ed2;
      localEdges[2] = ed3;

      for ( int tp = 0; tp != 3; ++tp )
      {
         std::vector<triangulation::edge>::iterator it;
         if ( std::find( globalEdges.begin( ), globalEdges.end( ), localEdges[tp] ) == globalEdges.end( ) ) globalEdges.push_back( localEdges[tp] );
      }

      // save the found neighbours
      n->tids[0] = dTriang[it].n1;
      n->tids[1] = dTriang[it].n2;
      n->tids[2] = dTriang[it].n3;

      if ( nn_verbose )
         fprintf( stderr, "  %d: (%d,%d,%d)\n", it, t->vids[0], t->vids[1], t->vids[2] );

      status = circle_build1( c, &d->points[t->vids[0]], &d->points[t->vids[1]], &d->points[t->vids[2]] );
      assert( status );
   }

   for ( i = 0; i < d->ntriangles; ++i ) {
      triangle* t = &d->triangles[i];

      for ( j = 0; j < 3; ++j )
         d->n_point_triangles[t->vids[j]]++;
   }
   if ( d->ntriangles > 0 ) {
      for ( i = 0; i < d->npoints; ++i ) {
         if ( d->n_point_triangles[i] > 0 )
            d->point_triangles[i] = (int *)malloc( d->n_point_triangles[i] * sizeof( int ) );
         else
            d->point_triangles[i] = NULL;
         d->n_point_triangles[i] = 0;
      }
   }
   for ( i = 0; i < d->ntriangles; ++i ) {
      triangle* t = &d->triangles[i];

      for ( j = 0; j < 3; ++j ) {
         int vid = t->vids[j];

         d->point_triangles[vid][d->n_point_triangles[vid]] = i;
         d->n_point_triangles[vid]++;
      }
   }

   if ( globalEdges.size( ) > 0 )
   {
      int * edgelist = (int *)malloc( globalEdges.size( ) * 2 * sizeof( int ) );
      for ( int tp = 0; tp != globalEdges.size( ); ++tp )
      {
         edgelist[tp * 2] = globalEdges[tp].p1;
         edgelist[tp * 2 + 1] = globalEdges[tp].p2;
      }
      d->nedges = globalEdges.size( );
      d->edges = (int *)malloc( d->nedges * 2 * sizeof( int ) );
      memcpy( d->edges, edgelist, d->nedges * 2 * sizeof( int ) );
      free( edgelist );
   }
}

/* Builds Delaunay triangulation of the given array of points.
 *
 * @param np Number of points
 * @param points Array of points [np] (input)
 * @param ns Number of forced segments
 * @param segments Array of (forced) segment endpoint indices [2*ns]
 * @param nh Number of holes
 * @param holes Array of hole (x,y) coordinates [2*nh]
 * @return Delaunay triangulation structure with triangulation results
 */
delaunay* delaunay_build(int np, point points[], int ns, int segments[], int nh, double holes[] )
{
    delaunay* d = delaunay_create();

    char cmd[64] = "eznC";
    int i, j;
    
    if (np == 0) {
        free(d);
        return NULL;
    }

    if (!nn_verbose)
        strcat(cmd, "Q");
    else if (nn_verbose > 1)
        strcat(cmd, "VV");
    if (ns != 0)
        strcat(cmd, "p");

    if (nn_verbose)
        fflush(stderr);

    /*
     * climax 
     */

    if (nn_verbose)
        fflush(stderr);

    d->npoints = np;
    d->points = points;
    tio2delaunayTriangulation( d );

    return d;
}

/* Destroys Delaunay triangulation.
 *
 * @param d Structure to be destroyed
 */
void delaunay_destroy(delaunay* d)
{
    if (d == NULL)
        return;

    if (d->point_triangles != NULL) {
        int i;

        for (i = 0; i < d->npoints; ++i)
            if (d->point_triangles[i] != NULL)
                free(d->point_triangles[i]);
        free(d->point_triangles);
    }
    if (d->nedges > 0)
        free(d->edges);
    if (d->n_point_triangles != NULL)
        free(d->n_point_triangles);
    if (d->flags != NULL)
        free(d->flags);
    if (d->circles != NULL)
        free(d->circles);
    if (d->neighbours != NULL)
        free(d->neighbours);
    if (d->triangles != NULL)
        free(d->triangles);
    if (d->t_in != NULL)
        istack_destroy(d->t_in);
    if (d->t_out != NULL)
        istack_destroy(d->t_out);
    if (d->flagids != NULL)
        free(d->flagids);
    free(d);
}

/* Returns whether the point p is on the right side of the vector (p0, p1).
 */
static int onrightside(point* p, point* p0, point* p1)
{
    return (p1->x - p->x) * (p0->y - p->y) > (p0->x - p->x) * (p1->y - p->y);
}

/* Finds triangle specified point belongs to (if any).
 *
 * @param d Delaunay triangulation
 * @param p Point to be mapped
 * @param seed Triangle index to start with
 * @return Triangle id if successful, -1 otherwhile
 */
int delaunay_xytoi(delaunay* d, point* p, int id)
{
    triangle* t;
    int i;

    if (p->x < d->xmin || p->x > d->xmax || p->y < d->ymin || p->y > d->ymax)
        return -1;

    if (id < 0 || id > d->ntriangles)
        id = 0;
    t = &d->triangles[id];
    do {
        for (i = 0; i < 3; ++i) {
            int i1 = (i + 1) % 3;

            if (onrightside(p, &d->points[t->vids[i]], &d->points[t->vids[i1]])) {
                id = d->neighbours[id].tids[(i + 2) % 3];
                if (id < 0)
                    return id;
                t = &d->triangles[id];
                break;
            }
        }
    } while (i < 3);

    return id;
}

static void delaunay_addflag(delaunay* d, int i)
{
    if (d->nflags == d->nflagsallocated) {
        d->nflagsallocated += N_FLAGS_INC;
        d->flagids = (int *)realloc( d->flagids, d->nflagsallocated * sizeof( int ) );
    }
    d->flagids[d->nflags] = i;
    d->nflags++;
}

static void delaunay_resetflags(delaunay* d)
{
    int i;

    for (i = 0; i < d->nflags; ++i)
        d->flags[d->flagids[i]] = 0;
    d->nflags = 0;
}

/* Finds all tricircles specified point belongs to.
 *
 * @param d Delaunay triangulation
 * @param p Point to be mapped
 * @param n Pointer to the number of tricircles within `d' containing `p'
 *          (output)
 * @param out Pointer to an array of indices of the corresponding triangles 
 *            [n] (output)
 *
 * There is a standard search procedure involving search through triangle
 * neighbours (not through vertex neighbours). It must be a bit faster due to
 * the smaller number of triangle neighbours (3 per triangle) but may fail
 * for a point outside convex hall.
 *
 * We may wish to modify this procedure in future: first check if the point
 * is inside the convex hall, and depending on that use one of the two
 * search algorithms. It not 100% clear though whether this will lead to a
 * substantial speed gains because of the check on convex hall involved.
 */
void delaunay_circles_find(delaunay* d, point* p, int* n, int** out)
{
    /*
     * This flag was introduced as a hack to handle some degenerate cases. It 
     * is set to 1 only if the triangle associated with the first circle is
     * already known to contain the point. In this case the circle is assumed 
     * to contain the point without a check. In my practice this turned
     * useful in some cases when point p coincided with one of the vertices
     * of a thin triangle. 
     */
    int contains = 0;
    int i;

    if (d->t_in == NULL) {
        d->t_in = istack_create();
        d->t_out = istack_create();
    }

    /*
     * if there are only a few data points, do linear search
     */
    if (d->ntriangles <= N_SEARCH_TURNON) {
        istack_reset(d->t_out);

        for (i = 0; i < d->ntriangles; ++i) {
            if (circle_contains(&d->circles[i], p)) {
                istack_push(d->t_out, i);
            }
        }

        *n = d->t_out->n;
        *out = d->t_out->v;

        return;
    }
    /*
     * otherwise, do a more complicated stuff
     */

    /*
     * It is important to have a reasonable seed here. If the last search
     * was successful -- start with the last found tricircle, otherwhile (i) 
     * try to find a triangle containing p; if fails then (ii) check
     * tricircles from the last search; if fails then (iii) make linear
     * search through all tricircles 
     */
    if (d->first_id < 0 || !circle_contains(&d->circles[d->first_id], p)) {
        /*
         * if any triangle contains p -- start with this triangle 
         */
        d->first_id = delaunay_xytoi(d, p, d->first_id);
        contains = (d->first_id >= 0);

        /*
         * if no triangle contains p, there still is a chance that it is
         * inside some of circumcircles 
         */
        if (d->first_id < 0) {
            int nn = d->t_out->n;
            int tid = -1;

            /*
             * first check results of the last search 
             */
            for (i = 0; i < nn; ++i) {
                tid = d->t_out->v[i];
                if (circle_contains(&d->circles[tid], p))
                    break;
            }
            /*
             * if unsuccessful, search through all circles 
             */
            if (tid < 0 || i == nn) {
                double nt = d->ntriangles;

                for (tid = 0; tid < nt; ++tid) {
                    if (circle_contains(&d->circles[tid], p))
                        break;
                }
                if (tid == nt) {
                    istack_reset(d->t_out);
                    *n = 0;
                    *out = NULL;
                    return;     /* failed */
                }
            }
            d->first_id = tid;
        }
    }

    istack_reset(d->t_in);
    istack_reset(d->t_out);

    istack_push(d->t_in, d->first_id);
    d->flags[d->first_id] = 1;
    delaunay_addflag(d, d->first_id);

    /*
     * main cycle 
     */
    while (d->t_in->n > 0) {
        int tid = istack_pop(d->t_in);
        triangle* t = &d->triangles[tid];

        if (contains || circle_contains(&d->circles[tid], p)) {
            istack_push(d->t_out, tid);
            for (i = 0; i < 3; ++i) {
                int vid = t->vids[i];
                int nt = d->n_point_triangles[vid];
                int j;

                for (j = 0; j < nt; ++j) {
                    int ntid = d->point_triangles[vid][j];

                    if (d->flags[ntid] == 0) {
                        istack_push(d->t_in, ntid);
                        d->flags[ntid] = 1;
                        delaunay_addflag(d, ntid);
                    }
                }
            }
        }
        contains = 0;
    }

    *n = d->t_out->n;
    *out = d->t_out->v;
    delaunay_resetflags(d);
}
