W = 85;

T = 5;
screw_r = 1.9/2;

difference(){
  cube([W, W, T]);
  rotate(a=45, v=[0, 0, 1])translate([-W, 0, -T])cube([3*W, 3*W, 3*T]);
  translate([W - 10, 80, T/2])rotate(a=90, v=[1, 0, 0])cylinder(r=screw_r, h=100, $fn=20);
  translate([W - 55, 80, T/2])rotate(a=90, v=[1, 0, 0])cylinder(r=screw_r, h=100, $fn=20);
  translate([0, 10, T/2])rotate(a=90, v=[0, 1, 0])cylinder(r=screw_r, h=100, $fn=20);
  translate([0, 55, T/2])rotate(a=90, v=[0, 1, 0])cylinder(r=screw_r, h=100, $fn=20);
}


