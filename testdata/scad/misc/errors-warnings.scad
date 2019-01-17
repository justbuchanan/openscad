echo(a);

polygon(points=[[0,0],[100,0],[130,50],[30]]);

polyhedron(
  points=[ [10,10,0],[10,-10,0],[-10,-10,0],[-10,10,0],
           [0]  ],
  faces=[ [0,1,4],[1,2,4],[2,3,4],[3,0,4],
              [1,0,3],[2,1,3] ]
 );
 
//file does not exist - we only care about the file ending
import("doesNotExist.aaa");
//file does not exist an thus creates a warning
import("doesNotExist.off");
//unkown module
hello();
//unkown function
b=test();
//radius is ignored as a diameter is defined
circle(r=1,d=4);
cylinder(r1=1,d1=1);

rotate(1/0)
rotate([1/0,1/0])
rotate((1/0)/(1/0))
cube();

circle(rad=5);
//too many unnamed arguments
cube(1,2,3,4,5,6);
//either r or r1 and r2
cylinder(r1=1,r2=2,r=3);
cylinder(r=0,r1=1,r2=2);
cylinder(d=0,r1=1,r2=2);
cylinder(5,r1=1,r2=2);

$vpr="[1,2,3]";
$vpt=[1,2,3,4];
$vpd=[1];

color([-1,0,1,2])
color("notAName")
box();

cylinder($fs=0);
cylinder($fa=0);
cylinder($fn=0);

cylinder($fs=1/0);
cylinder($fa=1/0);
cylinder($fn=1/0);

cylinder($fs="test");
cylinder($fa="test");
cylinder($fn="test");