// Configuration
pcbheight = 1.6;
pcbheight_populated = pcbheight + 4;
blockheight = 8.5;
blockmargin_x = 15;
blockmargin_y = 25;
pcbwidth_nm = 35;
pcb_squarelength_nm = 90;
pcb_trapezoidlength_nm = 9.5;
pcb_trapezoidwidth_nm = 25;
pcb_trapezoidoffset = 1;

// Some production margins
pcbwidth = pcbwidth_nm + 0.3;
pcb_squarelength = pcb_squarelength_nm + 0.3;
pcb_trapezoidlength = pcb_trapezoidlength_nm + 0.3;
pcb_trapezoidwidth = pcb_trapezoidwidth_nm + 0.3;

// Derived values
pcb_length = pcb_squarelength + pcb_trapezoidlength;
pcb_translation = [blockmargin_x, blockmargin_y, blockheight - pcbheight_populated];
blocksize = [2 * blockmargin_x + pcb_length, pcbwidth + 2 * blockmargin_y, blockheight];
trapezoid = [
    [pcb_squarelength, 0],
    [pcb_squarelength, pcbwidth],
    [pcb_squarelength + pcb_trapezoidlength, pcbwidth + (pcb_trapezoidwidth - pcbwidth) / 2 + pcb_trapezoidoffset],
    [pcb_squarelength + pcb_trapezoidlength, (pcbwidth - pcb_trapezoidwidth) / 2 + pcb_trapezoidoffset]
];

// Basic shape: PCB-shaped hole in block
difference() {
    color([0, 0.4, 1]) cube(blocksize);
    translate(pcb_translation) {
        cube([pcb_squarelength, pcbwidth, pcbheight_populated + 1]);
        linear_extrude(height = pcbheight_populated + 1) {
            polygon(points=trapezoid);
        }
    }
}

// PCB support structure in unpopulated areas
pcbsupport_battery_length = 60;
pcbsupport_height = pcbheight_populated - pcbheight;
pcbsupport_dbghdr_length = 20;
pcbsupport_dbghdr_width = 7;
pcbsupport_mnthole_width = 7;
pcbsupport_mnthole_length = 5;
translate(pcb_translation) {
    translate([pcb_length - pcbsupport_battery_length, 0, 0])
        cube([pcbsupport_battery_length, pcbwidth, pcbsupport_height]);
    cube([pcbsupport_dbghdr_length, pcbsupport_dbghdr_width, pcbsupport_height]);
    translate([0, pcbwidth - pcbsupport_mnthole_width, 0])
        cube([pcbsupport_mnthole_length, pcbsupport_mnthole_width, pcbsupport_height]);
}