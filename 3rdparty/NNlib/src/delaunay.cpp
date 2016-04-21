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
// use triangle for the triangulation. It is a LGPL library that could possibly be substituted with Poly2Tri
#define USETRIANGLE

#ifdef USETRIANGLE
#define ANSI_DECLARATORS        /* for triangle.h */
#else
#include "poly2tri.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include "triangle.h"
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

#ifdef USETRIANGLE
static void tio_init(struct triangulateio* tio)
{
    tio->pointlist = NULL;
    tio->pointattributelist = NULL;
    tio->pointmarkerlist = NULL;
    tio->numberofpoints = 0;
    tio->numberofpointattributes = 0;
    tio->trianglelist = NULL;
    tio->triangleattributelist = NULL;
    tio->trianglearealist = NULL;
    tio->neighborlist = NULL;
    tio->numberoftriangles = 0;
    tio->numberofcorners = 0;
    tio->numberoftriangleattributes = 0;
    tio->segmentlist = 0;
    tio->segmentmarkerlist = NULL;
    tio->numberofsegments = 0;
    tio->holelist = NULL;
    tio->numberofholes = 0;
    tio->regionlist = NULL;
    tio->numberofregions = 0;
    tio->edgelist = NULL;
    tio->edgemarkerlist = NULL;
    tio->normlist = NULL;
    tio->numberofedges = 0;
}

static void tio_destroy(struct triangulateio* tio)
{
    if (tio->pointlist != NULL)
        free(tio->pointlist);
    if (tio->pointattributelist != NULL)
        free(tio->pointattributelist);
    if (tio->pointmarkerlist != NULL)
        free(tio->pointmarkerlist);
    if (tio->trianglelist != NULL)
        free(tio->trianglelist);
    if (tio->triangleattributelist != NULL)
        free(tio->triangleattributelist);
    if (tio->trianglearealist != NULL)
        free(tio->trianglearealist);
    if (tio->neighborlist != NULL)
        free(tio->neighborlist);
    if (tio->segmentlist != NULL)
        free(tio->segmentlist);
    if (tio->segmentmarkerlist != NULL)
        free(tio->segmentmarkerlist);
    if (tio->holelist != NULL)
        free(tio->holelist);
    if (tio->regionlist != NULL)
        free(tio->regionlist);
    if (tio->edgelist != NULL)
        free(tio->edgelist);
    if (tio->edgemarkerlist != NULL)
        free(tio->edgemarkerlist);
    if (tio->normlist != NULL)
        free(tio->normlist);
}
#endif

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

#ifdef USETRIANGLE
static void tio2delaunay(struct triangulateio* tio_out, delaunay* d)
{
    int i, j;

    /*
     * I assume that all input points appear in tio_out in the same order as 
     * they were written to tio_in. I have seen no exceptions so far, even
     * if duplicate points were presented. Just in case, let us make a couple
     * of checks. 
     */
    assert(tio_out->numberofpoints == d->npoints);
    assert(tio_out->pointlist[2 * d->npoints - 2] == d->points[d->npoints - 1].x && tio_out->pointlist[2 * d->npoints - 1] == d->points[d->npoints - 1].y);

    for (i = 0, j = 0; i < d->npoints; ++i) {
        point* p = &d->points[i];

        if (p->x < d->xmin)
            d->xmin = p->x;
        if (p->x > d->xmax)
            d->xmax = p->x;
        if (p->y < d->ymin)
            d->ymin = p->y;
        if (p->y > d->ymax)
            d->ymax = p->y;
    }
    if (nn_verbose) {
        fprintf(stderr, "input:\n");
        for (i = 0, j = 0; i < d->npoints; ++i) {
            point* p = &d->points[i];

            fprintf(stderr, "  %d: %15.7g %15.7g %15.7g\n", i, p->x, p->y, p->z);
        }
    }

    d->ntriangles = tio_out->numberoftriangles;

    if (d->ntriangles > 0) {
        d->triangles = (triangle *) malloc(d->ntriangles * sizeof(triangle));
        d->neighbours = (triangle_neighbours *) malloc(d->ntriangles * sizeof(triangle_neighbours));
        d->circles = (circle *) malloc(d->ntriangles * sizeof(circle));
        d->n_point_triangles = (int *) calloc(d->npoints, sizeof(int));
        d->point_triangles = (int **) malloc(d->npoints * sizeof(int*));
        d->flags = (int *) calloc(d->ntriangles, sizeof(int));
    }

    if (nn_verbose)
        fprintf(stderr, "triangles:\n");
    for (i = 0; i < d->ntriangles; ++i) {
        int offset = i * 3;
        triangle* t = &d->triangles[i];
        triangle_neighbours* n = &d->neighbours[i];
        circle* c = &d->circles[i];
        int status;

        t->vids[0] = tio_out->trianglelist[offset];
        t->vids[1] = tio_out->trianglelist[offset + 1];
        t->vids[2] = tio_out->trianglelist[offset + 2];

        n->tids[0] = tio_out->neighborlist[offset];
        n->tids[1] = tio_out->neighborlist[offset + 1];
        n->tids[2] = tio_out->neighborlist[offset + 2];

        status = circle_build1(c, &d->points[t->vids[0]], &d->points[t->vids[1]], &d->points[t->vids[2]]);
        assert(status);

        if (nn_verbose)
            fprintf(stderr, "  %d: (%d,%d,%d)\n", i, t->vids[0], t->vids[1], t->vids[2]);
    }

    for (i = 0; i < d->ntriangles; ++i) {
        triangle* t = &d->triangles[i];

        for (j = 0; j < 3; ++j)
            d->n_point_triangles[t->vids[j]]++;
    }
    if (d->ntriangles > 0) {
        for (i = 0; i < d->npoints; ++i) {
            if (d->n_point_triangles[i] > 0)
                d->point_triangles[i] = (int *)malloc(d->n_point_triangles[i] * sizeof(int));
            else
                d->point_triangles[i] = NULL;
            d->n_point_triangles[i] = 0;
        }
    }
    for (i = 0; i < d->ntriangles; ++i) {
        triangle* t = &d->triangles[i];

        for (j = 0; j < 3; ++j) {
            int vid = t->vids[j];

            d->point_triangles[vid][d->n_point_triangles[vid]] = i;
            d->n_point_triangles[vid]++;
        }
    }

    if (tio_out->edgelist != NULL) 
    {
        d->nedges = tio_out->numberofedges;  
        d->edges = (int *)malloc( d->nedges * 2 * sizeof( int ) );
        memcpy(d->edges, tio_out->edgelist, d->nedges * 2 * sizeof(int));
    }
}
#else
static void tio2delaunayPoly2Tri( p2t::CDT & CDT, delaunay* d )
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

   // get all triangles from CDT 
   std::vector<p2t::Triangle*> triangles = CDT.GetTriangles( );
   d->ntriangles = triangles.size( );

   if ( d->ntriangles > 0 )
   {
      d->triangles = (triangle *)malloc( d->ntriangles * sizeof( triangle ) );
      d->neighbours = (triangle_neighbours *)malloc( d->ntriangles * sizeof( triangle_neighbours ) );
      d->circles = (circle *)malloc( d->ntriangles * sizeof( circle ) );
      d->n_point_triangles = (int *)calloc( d->npoints, sizeof( int ) );
      d->point_triangles = (int **)malloc( d->npoints * sizeof( int* ) );
      d->flags = (int *)calloc( d->ntriangles, sizeof( int ) );
   }

   // local variables to save delaunay interpolation 
   std::vector<p2t::Point*>        trianglePoints( 3 );
   std::vector<p2t::Triangle*>     triangleNeighbours( 3 );
   std::vector<int>                neighboursInt( 3 );
   std::vector<p2t::GeneralEdge>   globalEdges;
   std::vector<p2t::GeneralEdge>   triangleEdges( 3 );
   int pos;

   if ( nn_verbose )
      fprintf( stderr, "triangles:\n" );
   for ( i = 0; i < d->ntriangles; ++i ) {
      int offset = i * 3;
      triangle* t = &d->triangles[i];
      triangle_neighbours* n = &d->neighbours[i];
      circle* c = &d->circles[i];
      int status;

      for ( int tp = 0; tp != 3; ++tp )
      {
         trianglePoints[tp] = triangles[i]->GetPoint( tp );
         triangleNeighbours[tp] = triangles[i]->NeighborCCW( *trianglePoints[tp] );
      }

      triangleEdges[0] = p2t::GeneralEdge( *trianglePoints[0], *trianglePoints[1] );
      triangleEdges[1] = p2t::GeneralEdge( *trianglePoints[1], *trianglePoints[2] );
      triangleEdges[2] = p2t::GeneralEdge( *trianglePoints[2], *trianglePoints[0] );

      // save
      t->vids[0] = trianglePoints[0]->GetInd( );
      t->vids[1] = trianglePoints[1]->GetInd( );
      t->vids[2] = trianglePoints[2]->GetInd( );

      for ( int tp = 0; tp != 3; ++tp )
      {
         pos = std::find( triangles.begin( ), triangles.end( ), triangleNeighbours[tp] ) - triangles.begin( );
         if ( pos >= triangles.size( ) ) pos = -1;
         neighboursInt[tp] = pos;
         std::vector<p2t::GeneralEdge>::iterator it;
         it = std::find( globalEdges.begin( ), globalEdges.end( ), triangleEdges[tp] );
         if ( it == globalEdges.end( ) ) globalEdges.push_back( triangleEdges[tp] );
      }

      // save
      n->tids[0] = neighboursInt[0];
      n->tids[1] = neighboursInt[1];
      n->tids[2] = neighboursInt[2];

      if ( nn_verbose )
         fprintf( stderr, "  %d: (%d,%d,%d)\n", i, t->vids[0], t->vids[1], t->vids[2] );

      status = circle_build1( c, &d->points[t->vids[0]], &d->points[t->vids[1]], &d->points[t->vids[2]] );
      assert( status );

   }

   //something is different here between the triangulation calculated by poly2tri and triangle (some entries of the triangles vector are different!)
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
         edgelist[tp * 2] = globalEdges[tp].p->GetInd( );
         edgelist[tp * 2 + 1] = globalEdges[tp].q->GetInd( );
      }
      d->nedges = globalEdges.size( );
      d->edges = (int *)malloc( d->nedges * 2 * sizeof( int ) );
      memcpy( d->edges, edgelist, d->nedges * 2 * sizeof( int ) );
      free( edgelist );
   }
}
#endif

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
#ifdef USETRIANGLE
    struct triangulateio tio_in; 
    struct triangulateio tio_out;
#endif
    char cmd[64] = "eznC";
    int i, j;
    
    assert(sizeof(REAL) == sizeof(double));
#ifdef USETRIANGLE
    tio_init(&tio_in);
#endif
    if (np == 0) {
        free(d);
        return NULL;
    }
#ifdef USETRIANGLE
    tio_in.pointlist = (double *) malloc(np * 2 * sizeof(double));
    tio_in.numberofpoints = np;
    for (i = 0, j = 0; i < np; ++i) {
        tio_in.pointlist[j++] = points[i].x;
        tio_in.pointlist[j++] = points[i].y;
    }

    if (ns > 0) {
       tio_in.segmentlist = (int *)malloc( ns * 2 * sizeof( int ) );
        tio_in.numberofsegments = ns;
        memcpy(tio_in.segmentlist, segments, ns * 2 * sizeof(int));
    }

    if (nh > 0) {
        tio_in.holelist = (double *) malloc(nh * 2 * sizeof(double));
        tio_in.numberofholes = nh;
        memcpy(tio_in.holelist, holes, nh * 2 * sizeof(double));
    }

    tio_init(&tio_out);
#endif
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
#ifdef USETRIANGLE
    triangulate( cmd, &tio_in, &tio_out, NULL );
#else
    // First define the bounding polygon
    std::vector<p2t::Point*> polyline;
    std::vector<p2t::Point>  uniquePoints;
    int pointInd = 0;
    int convexHullPoints = 100; // nvexHullPoints is an additional parameter that should be provided by the function

    for ( i = 0; i < convexHullPoints; ++i )
    {
       polyline.push_back( new p2t::Point( points[i].x, points[i].y, pointInd++ ) );
       uniquePoints.push_back( *polyline.back( ) );
    }

    // create CTD 
    p2t::CDT CDT( polyline );

    // add only new  points, use polyline to store pointers for later memory deletion
    for ( i = convexHullPoints; i != np; ++i )
    {
       // create a temporary point
       p2t::Point tempPoint = p2t::Point( points[i].x, points[i].y );
       //push back the edges if they are not found
       std::vector<p2t::Point>::iterator it;
       it = std::find( uniquePoints.begin(), uniquePoints.end(), tempPoint );
       if ( it == uniquePoints.end() )
       {
       polyline.push_back( new p2t::Point( points[i].x, points[i].y, pointInd++ ) );
       uniquePoints.push_back( *polyline.back( ) );
       CDT.AddPoint( polyline.back( ) );
       }
    }

    // Triangulate
    CDT.Triangulate( );
#endif

    if (nn_verbose)
        fflush(stderr);

    d->npoints = np;
    d->points = points;

#ifdef USETRIANGLE
    tio2delaunay(&tio_out, d);

    tio_destroy(&tio_in);
    tio_destroy(&tio_out);
#else
    // delete allocated p2t::Point
    for ( i = 0; i != polyline.size( ); ++i )
    {
       delete polyline[i];
    }
#endif
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
