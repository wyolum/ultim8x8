$fn=100;
height=6;
d = 3.5;
D = d + 2;

difference(){
    cylinder(h=height, d=D+1);
    translate([0, 0, -1])cylinder(h=height+2, d=d);
}
