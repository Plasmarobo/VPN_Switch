WIDTH=42;
LENGTH=72;
HEIGHT=25;
TOLERENCE=0.3;
PUNCH_FACTOR=0.1;
GENERAL_PADDING=2 + TOLERENCE;
SCREW_CAP_DIAMETER=5.5 + TOLERENCE;
SCREW_HEX_DIAMETER=5.8 + TOLERENCE;
SCREW_HOLE_DIAMETER=3.2 + TOLERENCE;
SCREW_CAP_HEIGHT=3;
SCREW_HEX_HEIGHT=2.5;
SCREW_HEIGHT=20;

LED_DIAMETER=10 + TOLERENCE;
LED_OFFSET_WIDTH=26;
LED_OFFSET_LENGTH=32;

SWITCH_DIAMETER=12 + TOLERENCE;
SWITCH_OFFSET_WIDTH=35;
SWITCH_OFFSET_LENGTH=65;

PLATE_THICKNESS=3;
LID_THICKNESS=4;
LID_CATCH_HEIGHT=1;

SCREW_PADDING = 2 * max(SCREW_CAP_DIAMETER, SCREW_HEX_DIAMETER);
WALL_THICKNESS = (SCREW_PADDING/2) + GENERAL_PADDING;

POWER_HEIGHT=12 + TOLERENCE;
POWER_WIDTH=20 + TOLERENCE;
POWER_LENGTH=WALL_THICKNESS;
POWER_LIP=1.2;

FEATHER_FRONT_POST_DIAMETER=2.6 - TOLERENCE;
FEATHER_BACK_POST_DIAMETER=2.3 - TOLERENCE;
FEATHER_POST_OFFSET=3 + TOLERENCE;
FEATHER_POST_HEIGHT=7;
FEATHER_FRONT_WIDTH=18;
FEATHER_BACK_WIDTH=18.8;
FEATHER_LENGTH=45 + TOLERENCE;

USB_MICRO_HEIGHT=8;
USB_MICRO_WIDTH=13;
USB_MICRO_LENGTH=WALL_THICKNESS;

EXTERIOR_WIDTH = WIDTH + (WALL_THICKNESS*2);
EXTERIOR_LENGTH = LENGTH + (WALL_THICKNESS*2);

$fn = 200;

difference() {
    union() {
        BaseBox();
        // OR LID
        //Lid();
    }
    union() {
        translate([WALL_THICKNESS/2,WALL_THICKNESS/2,0]) {
            ScrewAnchor(HEIGHT + PLATE_THICKNESS,
                LID_THICKNESS,
                SCREW_HOLE_DIAMETER,
                SCREW_HEIGHT,
                SCREW_CAP_DIAMETER,
                SCREW_CAP_HEIGHT,
                SCREW_HEX_DIAMETER,
                SCREW_HEX_HEIGHT);
        }
        translate([EXTERIOR_LENGTH-(WALL_THICKNESS/2),EXTERIOR_WIDTH-(WALL_THICKNESS/2),0]) {
            ScrewAnchor(HEIGHT + PLATE_THICKNESS,
                LID_THICKNESS,
                SCREW_HOLE_DIAMETER,
                SCREW_HEIGHT,
                SCREW_CAP_DIAMETER,
                SCREW_CAP_HEIGHT,
                SCREW_HEX_DIAMETER,
                SCREW_HEX_HEIGHT);
        }
        translate([EXTERIOR_LENGTH-(WALL_THICKNESS/2),WALL_THICKNESS/2,0]) {
            ScrewAnchor(HEIGHT + PLATE_THICKNESS,
                LID_THICKNESS,
                SCREW_HOLE_DIAMETER,
                SCREW_HEIGHT,
                SCREW_CAP_DIAMETER,
                SCREW_CAP_HEIGHT,
                SCREW_HEX_DIAMETER,
                SCREW_HEX_HEIGHT);
        }
        translate([WALL_THICKNESS/2,EXTERIOR_WIDTH-(WALL_THICKNESS/2),0]) {
            ScrewAnchor(HEIGHT + PLATE_THICKNESS,
                LID_THICKNESS,
                SCREW_HOLE_DIAMETER,
                SCREW_HEIGHT,
                SCREW_CAP_DIAMETER,
                SCREW_CAP_HEIGHT,
                SCREW_HEX_DIAMETER,
                SCREW_HEX_HEIGHT);
        }
        translate([-1, (EXTERIOR_WIDTH/2)-(USB_MICRO_WIDTH/2), PLATE_THICKNESS+FEATHER_POST_HEIGHT-(USB_MICRO_HEIGHT/3)]){
            cube([USB_MICRO_LENGTH+2, USB_MICRO_WIDTH, USB_MICRO_HEIGHT]);
        }
        translate([EXTERIOR_LENGTH-POWER_LIP-POWER_LENGTH, (EXTERIOR_WIDTH/2) - (POWER_WIDTH/2) - (POWER_LIP/2), (HEIGHT/2) - (POWER_HEIGHT/2)]) {
            union() {
                cube([POWER_LENGTH + PUNCH_FACTOR, POWER_WIDTH + POWER_LIP, POWER_HEIGHT + POWER_LIP]);
                translate([POWER_LENGTH, POWER_LIP/2, POWER_LIP/2]) {
                    cube([POWER_LIP + PUNCH_FACTOR, POWER_WIDTH, POWER_HEIGHT]);
                }
            }
        }
    }
}

function cot(x)=1/tan(x);
module Hexagon(d,h)
{
	angle = 360/6;		// 6 sides
	cote = d * cot(angle);
	
	union()
	{
		rotate([0,0,0])
			cube([d,cote,h],center=true);
		rotate([0,0,angle])
			cube([d,cote,h],center=true);
		rotate([0,0,2*angle])
			cube([d,cote,h],center=true);
	}

}

module ScrewAnchor(box_h, lid_h, screw_d, screw_h, screw_cap_d, screw_cap_h, hex_d, hex_h) {
    cap_well_height = max(lid_h - LID_CATCH_HEIGHT,screw_cap_h - LID_CATCH_HEIGHT);
    effective_screw_height = screw_h - hex_h - LID_CATCH_HEIGHT; 
    hex_well_height = (box_h - effective_screw_height + LID_CATCH_HEIGHT);
    
    union() {
        translate([0,0,box_h + LID_CATCH_HEIGHT]) {
            cylinder(h=cap_well_height + PUNCH_FACTOR, d=screw_cap_d);
        }
        translate([0,0,box_h + LID_CATCH_HEIGHT - effective_screw_height]) {
            cylinder(h=effective_screw_height + PUNCH_FACTOR, d=screw_d);
        }
        translate([0,0, hex_well_height/2]) {
            Hexagon(hex_d, hex_well_height + PUNCH_FACTOR);
        }
    }
}

module BaseBox() {
    // Baseplate
    cube([EXTERIOR_LENGTH, EXTERIOR_WIDTH, PLATE_THICKNESS]);
    // Perimeter
    translate([0,0,PLATE_THICKNESS]){
        translate([WALL_THICKNESS+FEATHER_POST_OFFSET,(EXTERIOR_WIDTH/2)-(FEATHER_FRONT_WIDTH/2), 0]) {
            cylinder(h=FEATHER_POST_HEIGHT, d=FEATHER_FRONT_POST_DIAMETER);
            translate([0,FEATHER_FRONT_WIDTH,0]) {
                cylinder(h=FEATHER_POST_HEIGHT, d=FEATHER_FRONT_POST_DIAMETER);
            }
        }
        translate([WALL_THICKNESS+FEATHER_POST_OFFSET,(EXTERIOR_WIDTH/2)-(FEATHER_BACK_WIDTH/2), 0]) {
            cylinder(h=FEATHER_POST_HEIGHT, d=FEATHER_FRONT_POST_DIAMETER);
            translate([0,FEATHER_BACK_WIDTH,0]) {
                cylinder(h=FEATHER_POST_HEIGHT, d=FEATHER_FRONT_POST_DIAMETER);
            }
        
            translate([FEATHER_LENGTH,0,0]) {
                cylinder(h=FEATHER_POST_HEIGHT, d=FEATHER_BACK_POST_DIAMETER);
                translate([0, FEATHER_BACK_WIDTH,0]) {
                    cylinder(h=FEATHER_POST_HEIGHT, d=FEATHER_BACK_POST_DIAMETER);
                }
            }
        }
        
        linear_extrude(HEIGHT) {
            difference() {
                square([EXTERIOR_LENGTH, EXTERIOR_WIDTH]); 
                translate([WALL_THICKNESS, WALL_THICKNESS]) {
                    square([LENGTH, WIDTH]);
                }
            }
        }
    }
}

module Lid() {
    translate([0,0,HEIGHT + PLATE_THICKNESS]) {
        difference() {
            cube([EXTERIOR_LENGTH, EXTERIOR_WIDTH, LID_THICKNESS]);
        
            union() {
                translate([LED_OFFSET_LENGTH, LED_OFFSET_WIDTH, -1]) {
                    cylinder(h=LID_THICKNESS+2, d=LED_DIAMETER);
                }
                translate([SWITCH_OFFSET_LENGTH, SWITCH_OFFSET_WIDTH, -1]) {
                    cylinder(h=LID_THICKNESS+2, d=SWITCH_DIAMETER);
                }
            }
        }
    }
}
