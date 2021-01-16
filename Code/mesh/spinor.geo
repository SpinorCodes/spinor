//////////////////////////////////////////////////////////////////
//
//         D---------C
//        /|        /|
//       / |       / |
//      H---------G  |
//      |  A------|--B
//      | /       | /
//      |/        |/
//      E---------F
//
//
//      y
//      |
//      |
//      o -----x
//     /
//    /
//   z
//
//////////////////////////////////////////////////////////////////

ds = 0.5;                                                       // Setting side discretization length...
x_min = -1.0;                                                   // Setting "x_min"...
x_max = +1.0;                                                   // Setting "x_max"...
y_min = -1.0;                                                   // Setting "y_min"...
y_max = +1.0;                                                   // Setting "y_max"...
z_min = -1.0;                                                   // Setting "y_min"...
z_max = +1.0;                                                   // Setting "y_max"...

//////////////////////////////////////////////////////////////////
///////////////////////////// VERTICES ///////////////////////////
//////////////////////////////////////////////////////////////////
Point(1) = {x_min, y_min, z_min, ds};                           // Setting point "A"...
Point(2) = {x_max, y_min, z_min, ds};                           // Setting point "B"...
Point(3) = {x_max, y_max, z_min, ds};                           // Setting point "C"...
Point(4) = {x_min, y_max, z_min, ds};                           // Setting point "D"...
Point(5) = {x_min, y_min, z_max, ds};                           // Setting point "E"...
Point(6) = {x_max, y_min, z_max, ds};                           // Setting point "F"...
Point(7) = {x_max, y_max, z_max, ds};                           // Setting point "G"...
Point(8) = {x_min, y_max, z_max, ds};                           // Setting point "H"...

//////////////////////////////////////////////////////////////////
////////////////////////////// EDGES /////////////////////////////
//////////////////////////////////////////////////////////////////
Line(1)  = {1, 2};                                              // Setting side "AB"...
Line(2)  = {2, 3};                                              // Setting side "BC"...
Line(3)  = {3, 4};                                              // Setting side "CD"...
Line(4)  = {1, 4};                                              // Setting side "AD"...
Line(5)  = {5, 6};                                              // Setting side "EF"...
Line(6)  = {6, 7};                                              // Setting side "FG"...
Line(7)  = {7, 8};                                              // Setting side "GH"...
Line(8)  = {5, 8};                                              // Setting side "EH"...
Line(9)  = {4, 8};                                              // Setting side "DH"...
Line(10) = {3, 7};                                              // Setting side "CG"...
Line(11) = {1, 5};                                              // Setting side "AE"...
Line(12) = {2, 6};                                              // Setting side "BF"...

//////////////////////////////////////////////////////////////////
////////////////////////////// FACES /////////////////////////////
//////////////////////////////////////////////////////////////////
Curve Loop(1) = {1, 2, 3, -4};                                  // Setting perimeter "ABCD"...
Plane Surface(1) = {1};                                         // Setting surface "ABCD"...
Physical Surface(1) = {1};				                        // Setting physical surface "ABCD"

Curve Loop(2) = {5, 6, 7, -8};                                  // Setting perimeter "EFGH"...
Plane Surface(2) = {2};                                         // Setting surface "EFGH"...
Physical Surface(2) = {2};				                        // Setting physical surface "EFGH"

Curve Loop(3) = {4, 9, -8, -11};                                // Setting perimeter "ADHE"...
Plane Surface(3) = {3};                                         // Setting surface "ADHE"...
Physical Surface(3) = {3};				                        // Setting physical surface "ADHE"

Curve Loop(4) = {2, 10, -6, -12};                               // Setting perimeter "BCGF"...
Plane Surface(4) = {4};                                         // Setting surface "BCGF"...
Physical Surface(4) = {4};				                        // Setting physical surface "BCGF"

Curve Loop(5) = {1, 12, -5, -11};                               // Setting perimeter "ABFE"...
Plane Surface(5) = {5};                                         // Setting surface "ABFE"...
Physical Surface(5) = {5};				                        // Setting physical surface "ABFE"

Curve Loop(6) = {-3, 10, 7, -9};                                // Setting perimeter "DCGH"...
Plane Surface(6) = {6};                                         // Setting surface "DCGH"...
Physical Surface(6) = {6};				                        // Setting physical surface "DCGH"

Physical Surface (7) = {1, 2, 3, 4, 5, 6};			            // Setting frame surface...

//////////////////////////////////////////////////////////////////
///////////////////////////// VOLUME /////////////////////////////
//////////////////////////////////////////////////////////////////
out[] = Extrude {0.0 , 0.0, (z_max - z_min)}                    // Creating extrusion along z-axis...
{
  Surface{1};                                                   // Setting surface to be extruded...
};

Transfinite Surface {1};					                    // Setting transfinite surface...
Transfinite Surface {2};					                    // Setting transfinite surface...
Transfinite Surface {3};					                    // Setting transfinite surface...
Transfinite Surface {4};					                    // Setting transfinite surface...
Transfinite Surface {5};					                    // Setting transfinite surface...
Transfinite Surface {6};				  	                    // Setting transfinite surface...

Recombine Surface {1, 2, 3, 4, 5, 6};		  	                // Recombining surfaces...
Transfinite Volume {1} = {1, 2, 3, 4, 5, 6, 7, 8};		        // Setting transfinite volume...
Physical Volume(1) = {1};					                    // Setting physical group...
Mesh 3;							                                // Generating 3D mesh...
Mesh.SaveAll = 1;                                               // Saving all mesh nodes...