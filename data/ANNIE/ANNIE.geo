{
  name: "GEO",
  index: "world",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  invisible: 1, // omitted for visualization
  mother: "",
  type: "box",
  size: [5000.0,5000.0,5000.0], //mm, half-length
  material: "rock",
}

{
  name: "GEO",
  index: "hall",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  mother: "world",
  type: "box",
  size: [3000.0,3000.0,3000.0], //mm, half-length
  position: [0.0,0.0,1018.8],
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
  r_max: 1524.0,
  size_z: 1981.2,
  position: [0.0,0.0,-1018.8],
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
  r_max: 1494.0,
  size_z: 1951.2,
  position: [0.0,0.0,0.0],
  material: "water",
  surface: "tyvek",
  color: [0.0, 0.0, 0.3, 0.3],
}

{
  name: "GEO",
  index: "ncv",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  mother: "content",
  type: "tube",
  r_max: 254.0,
  size_z: 304.8,
  position: [0.0,-800.0,500.0],
  material: "acrylic",
  color: [0.0, 0.0, 0.7, 0.5],
}

{
  name: "GEO",
  index: "ncv",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  mother: "content",
  type: "tube",
  r_max: 254.0,
  size_z: 304.8,
  position: [0.0,-800.0,500.0],
  material: "acrylic_uvt",
  color: [0.0, 0.5, 0.7, 0.5],
}

{
  name: "GEO",
  index: "ncv_content",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  mother: "ncv",
  type: "tube",
  r_max: 244.475,
  size_z: 295.275,
  position: [0.0,0.0,0.0],
  material: "gd_scintillator",
  color: [0.3, 0.2, 0.7, 0.5],
}

{
  name: "GEO",
  index: "pmts",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  mother: "content",
  type: "pmtarray",
  pmt_model: "r5912",
  pmt_detector_type: "idpmt",
  sensitive_detector: "/mydet/pmt/inner",
  efficiency_correction: 1.0,
  pos_table: "PMTINFO",
  orientation: "manual",
}
