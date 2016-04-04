CylinderFaces = 360;

/* All Dimensions in Inches */
MalletBaseH = 3/8;
MalletEdgeH = 3/4;
MalletEdgeRingThick = 3/8;
MalletDiam = 3.75;

BoltDiam = 1/4 + .01; // Adding .01" of "wiggle room"
BoltRiserH = 1;
BoltRiserDiam = 3/4;
BoltRiserOffsetX = 1;
BoltRiserOffsetY = 1;

difference(){

union(){

difference(){

union(){
/* Create the mallet */
translate([0,0, MalletEdgeH/2]) cylinder(d = MalletDiam, h = MalletEdgeH, center = true, $fn = CylinderFaces);
}

/* Hollow out the center so we get an edge ring */
translate([0,0, 10/2 + MalletBaseH]) cylinder(d = MalletDiam - 2*MalletEdgeRingThick, h = 10, $fn = CylinderFaces, center = true);
}

/* Add the bolt risers */
translate([BoltRiserOffsetX,BoltRiserOffsetY,BoltRiserH/2]) cylinder(d = BoltRiserDiam, h = BoltRiserH, $fn = CylinderFaces, center = true);
translate([BoltRiserOffsetX,-BoltRiserOffsetY,BoltRiserH/2]) cylinder(d = BoltRiserDiam, h = BoltRiserH, $fn = CylinderFaces, center = true);
translate([-BoltRiserOffsetX,BoltRiserOffsetY,BoltRiserH/2]) cylinder(d = BoltRiserDiam, h = BoltRiserH, $fn = CylinderFaces, center = true);
translate([-BoltRiserOffsetX,-BoltRiserOffsetY,BoltRiserH/2]) cylinder(d = BoltRiserDiam, h = BoltRiserH, $fn = CylinderFaces, center = true);

}

/* Drill bolt hole in risers */
translate([BoltRiserOffsetX,BoltRiserOffsetY,BoltRiserH/2 + MalletBaseH]) cylinder(d = BoltDiam, h = BoltRiserH, $fn = CylinderFaces, center = true);
translate([BoltRiserOffsetX,-BoltRiserOffsetY,BoltRiserH/2 + MalletBaseH]) cylinder(d = BoltDiam, h = BoltRiserH, $fn = CylinderFaces, center = true);
translate([-BoltRiserOffsetX,BoltRiserOffsetY,BoltRiserH/2 + MalletBaseH]) cylinder(d = BoltDiam, h = BoltRiserH, $fn = CylinderFaces, center = true);
translate([-BoltRiserOffsetX,-BoltRiserOffsetY,BoltRiserH/2 + MalletBaseH]) cylinder(d = BoltDiam, h = BoltRiserH, $fn = CylinderFaces, center = true);

}
