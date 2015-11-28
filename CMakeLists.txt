project( saobaker )
cmake_minimum_required( VERSION 2.6.2 )
find_package(CGAL QUIET COMPONENTS Core )
include( ${CGAL_USE_FILE} )
include( CGAL_CreateSingleSourceCGALProgram )

create_single_source_cgal_program( "parameterize.cpp" )
create_single_source_cgal_program( "segmentation.cpp" )
