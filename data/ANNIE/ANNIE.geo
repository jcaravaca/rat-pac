{
  name: "GEO",
  index: "world",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  invisible: 1, // omitted for visualization
  mother: "",
  type: "box",
  size: [7000.0,7000.0,10000.0], //mm, half-length
  material: "rock",
}

{
  name: "GEO",
  index: "soil",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  mother: "world",
  type: "box",
  size: [7000.0,7000.0,5000.0], //mm, half-length
  position: [0.0,0.0,-3500.0],
  material: "rock",
  color: [0.5, 0.5, 0.5, 0.5],
}

{
  name: "GEO",
  index: "gate",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  mother: "world",
  type: "box",
  size: [3100.0,3100.0,2100.0], //mm, half-length
  position: [0.0,0.0,3600.0],
  material: "aluminum",
  color: [0.0, 1.0, 1.0, 0.5],
}

{
  name: "GEO",
  index: "gfloor",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  mother: "gate",
  type: "box",
  size: [3000.0,3000.0,2000.0], //mm, half-length
  position: [0.0,0.0,0.0],
  material: "air",
  color: [1.0, 1.0, 1.0, 0.5],
}

{
  name: "GEO",
  index: "hall",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  mother: "soil",
  type: "box",
  size: [3000.0,3000.0,4000.0], //mm, half-length
  position: [0.0,0.0,0.0],
  material: "air",
  color: [1.0, 1.0, 1.0, 0.5],
}

{
  name: "GEO",
  index: "tank",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  mother: "hall",
  type: "tube",
  r_max: 1500.0,
  size_z: 2000.0,
  position: [1000.0,0.0,-2000.0],
  material: "steel",
  color: [0.0, 0.5, 0.5, 0.5],
}

{
  name: "GEO",
  index: "content",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  mother: "tank",
  type: "tube",
  r_max: 1400.0,
  size_z: 1900.0,
  position: [0.0,0.0,0.0],
  material: "water",
  color: [0.0, 0.0, 0.7, 0.5],
}

{
  name: "GEO",
  index: "mrd",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  mother: "soil",
  type: "box",
  size: [500.0,1500.0,2000.0], //mm, half-length
  position: [-1500.0,0.0,-2000.0],
  material: "steel",
  color: [1.0, 0.0, 1.0, 0.5],
}

{
  name: "GEO",
  index: "pmts",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  mother: "content",
  type: "pmtarray",
  pmt_model: "r7081_hqe",
  pmt_detector_type: "idpmt",
  sensitive_detector: "/mydet/pmt/inner",
  efficiency_correction: 1.0,
  pos_table: "PMTINFO",
  orientation: "manual",
}
