H =6;
inch = 25.4;
pixel = 1/3.5433;
L = 2.4*inch;
Screw_r = 1.3;
screw_r = 1.9/2;

module side(){
  difference(){
    //linear_extrude(height=H)scale(.386)import("side.dxf");
    linear_extrude(height=H)scale(.51)import("side.dxf");

    translate([ .2*inch, 15, H/2])rotate(a=90, v=[1, 0, 0])cylinder(r=screw_r, h=20, $fn=15);
    translate([ .8*inch, 15, H/2])rotate(a=90, v=[1, 0, 0])cylinder(r=Screw_r, h=20, $fn=15);
    translate([1.4*inch, 15, H/2])rotate(a=90, v=[1, 0, 0])cylinder(r=Screw_r, h=20, $fn=15);
    translate([2.0*inch, 15, H/2])rotate(a=90, v=[1, 0, 0])cylinder(r=Screw_r, h=20, $fn=15);
    rotate(a=90, v=[0, 0, 1])
      union(){
      translate([ .2*inch, 20-15, H/2])rotate(a=90, v=[1, 0, 0])cylinder(r=screw_r, h=20, $fn=15);
      translate([ .8*inch, 20-15, H/2])rotate(a=90, v=[1, 0, 0])cylinder(r=Screw_r, h=20, $fn=15);
      translate([1.4*inch, 20-15, H/2])rotate(a=90, v=[1, 0, 0])cylinder(r=Screw_r, h=20, $fn=15);
      translate([2.0*inch, 20-15, H/2])rotate(a=90, v=[1, 0, 0])cylinder(r=Screw_r, h=20, $fn=15);
    }
  }
  //translate([-L, 0, 0])cube([L, L, H]);
}
side();

//translate([0, -80, 0])cube([80, 80, H/2]);
