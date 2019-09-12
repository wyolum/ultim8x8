inch=25.4;
//cube/led width
cw = 7;
led_h = 1.4;
//diagonal separation
//ds = sqrt(2*(cw*cw));
ds = .3*inch;
// ds = 7.7;
nx = 24;
ny = 24;

L = nx * ds;
W = ny * ds;

//baffle height
bh = 3;
bt = .82;

w = .18 * inch;
h = led_h;

module interlock(){
  translate([(1.5) * ds + bt, (3.5) * ds - bt/2, 0])rotate(a=45, v=[0, 0, 1])translate([-w/2, -w/2, 0]) cube([w, w, h]);
  translate([(5.5) * ds + bt, (3.5) * ds - bt/2, 0])rotate(a=45, v=[0, 0, 1])translate([-w/2, -w/2, 0]) cube([w, w, h]);
  translate([(3.5) * ds + bt, (1.5) * ds - bt/2, 0])rotate(a=45, v=[0, 0, 1])translate([-w/2, -w/2, 0]) cube([w, w, h]);
  translate([(3.5) * ds + bt, (5.5) * ds - bt/2, 0])rotate(a=45, v=[0, 0, 1])translate([-w/2, -w/2, 0]) cube([w, w, h]);
}
module grid(nx, ny){
  for(j = [1:1:ny-1]){
    yl = j * ds;
    translate([-cw/2, -ds/2 + yl-bt, led_h])cube([nx * ds, bt, bh]);
  } 
  for(i = [0:1:nx]){
    xl = i * ds;
    translate([-cw/2 + xl, -ds/2 - bt, led_h])cube([bt, ny*ds + bt, bh]);
  }

  for(i = [0:8:ny-1]){
    for(j = [0:8:nx-1]){
      translate([j * ds, i * ds, 0])interlock();
    }
  }
}

//rotate(a=180, v=[0, 1, 0])grid(nx, ny);
rotate(a=180, v=[0, 1, 0])
intersection(){
  grid(nx, 8);
  color("red")translate([-ds, -ds/2+bt/2, 0])cube([(nx  + 2)* ds, 8 * ds - 2 * bt, 10]);
}


