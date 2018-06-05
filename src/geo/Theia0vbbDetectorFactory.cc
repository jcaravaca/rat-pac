#include <RAT/Theia0vbbDetectorFactory.hh>
#include <RAT/Log.hh>
#include <RAT/DB.hh>

#include <cmath>
#include <vector>

using namespace std;

namespace RAT {

void Theia0vbbDetectorFactory::DefineDetector(DBLinkPtr /*detector*/) {
    const std::string geo_template = "Theia_0vbb/Theia_0vbb.geo";
    DB *db = DB::Get();
    if (db->Load(geo_template) == 0) {
        Log::Die("Theia0vbbDetectorFactory: could not load template Theia_0vbb/Theia_0vbb.geo");
    }
    DBLinkPtr params = db->GetLink("THEIA_PARAMS");
    
    const double photocathode_coverage = params->GetD("photocathode_coverage");
    const double veto_coverage = params->GetD("veto_coverage");
    
    const double fiducial_radius = params->GetD("fiducial_diameter")/2.0;
    const double fiducial_halfheight = params->GetD("fiducial_height")/2.0;
    const double fiducial_buffer = params->GetD("fiducial_buffer");
    const double veto_buffer = params->GetD("veto_buffer");
    const double veto_pmt_offset = params->GetD("veto_pmt_offset");
    const double tank_thickness = params->GetD("wall_thickness");
    const double balloon_radius = params->GetD("balloon_radius");
    const double balloon_thickness = params->GetD("balloon_thickness");
   
    const double det_radius = fiducial_radius+veto_buffer+fiducial_buffer;
    const double det_halfheight = fiducial_halfheight+veto_buffer+fiducial_buffer;

    // Re-create the layered balloon from KamLAND
    //
    //     Layer 1: EVOH (15 um)
    //     Layer 2: Nylon (25 um)
    //     Layer 3: EVOH (15 um)
    //     Layer 4: Nylon (25 um)
    //     Layer 5: EVOH (15 um)
    //     Layer 6: Nylon (25 um)
    //     Layer 7: EVOH (15 um)
    //
    // If the balloon's thickness parameter is changed from 135 um, the 
    // thickness of all the layers is proportionally changed.
    //
    //  -C. Grant, 7/11/2017
    //

    const double balloon_r_max = balloon_radius;
    const double nylon_thickness = 0.185185*balloon_thickness;
    const double evoh_thickness = 0.111111*balloon_thickness;
    const double nylon1_r_max = balloon_radius-evoh_thickness;
    const double nylon1_r_min = nylon1_r_max-nylon_thickness;
    const double nylon2_r_max = nylon1_r_min-evoh_thickness;
    const double nylon2_r_min = nylon2_r_max-nylon_thickness;
    const double nylon3_r_max = nylon2_r_min-evoh_thickness;
    const double target_r_max = balloon_radius-balloon_thickness;
  
    //calculate the area of the defined inner_pmts
    DBLinkPtr inner_pmts = db->GetLink("GEO","inner_pmts");
    string pmt_model = inner_pmts->GetS("pmt_model");
    DBLinkPtr pmt = db->GetLink("PMT", pmt_model);
    vector<double> rho_edge = pmt->GetDArray("rho_edge");
    double photocathode_radius = rho_edge[0];
    for (size_t i = 1; i < rho_edge.size(); i++) {
        if (photocathode_radius < rho_edge[i]) photocathode_radius = rho_edge[i];
    }
    const double photocathode_area = M_PI*photocathode_radius*photocathode_radius;
    
    const double pmt_radius = det_radius - veto_buffer;
    const double veto_radius = pmt_radius + veto_pmt_offset;
    
    const double topbot_offset = det_halfheight - veto_buffer;
    const double topbot_veto_pmt_offset = topbot_offset + veto_pmt_offset;
    
    const double surface_area = 2.0*M_PI*pmt_radius*pmt_radius + 2.0*topbot_offset*2.0*M_PI*pmt_radius;
    const double required_pmts = ceil(photocathode_coverage * surface_area / photocathode_area);
    const double veto_surface_area = 2.0*M_PI*veto_radius*veto_radius + 2.0*topbot_veto_pmt_offset*2.0*M_PI*veto_radius;
    const double required_vetos = ceil(veto_coverage * veto_surface_area / photocathode_area);
    
    const double pmt_space = sqrt(surface_area/required_pmts);
    const double veto_space = sqrt(veto_surface_area/required_vetos);
    
    const size_t cols = round(2.0*M_PI*pmt_radius/(pmt_space*1.118));
    const size_t rows = round(2.0*topbot_offset/(pmt_space/1.118));
    const size_t veto_cols = round(2.0*M_PI*veto_radius/(veto_space*1.118));
    const size_t veto_rows = round(2.0*topbot_veto_pmt_offset/(veto_space/1.118));
    
    info << "Generating new PMT positions for:\n";
    info << "\tdesired photocathode coverage " << photocathode_coverage << '\n';
    info << "\ttotal area " << surface_area << '\n';
    info << "\tphotocathode radius " << photocathode_radius << '\n';
    info << "\tphotocathode area " << photocathode_area << '\n';
    info << "\tdesired PMTs " << required_pmts << '\n';
    info << "\tPMT spacing " << pmt_space << '\n';
    
    //make the grid for top and bottom PMTs
    vector<pair<double,double> > topbot;
    vector<pair<double,double> > topbot_veto;
    const int rdim = round(1.5*pmt_radius/pmt_space); 
    for (int i = -rdim; i <= rdim; i++) {
        for (int j = -rdim; j <= rdim; j++) {
            double x = i*pmt_space/1.118;
            double y = (j+(i%2 ? 0.5 : 0.0))*pmt_space*1.118;
            if (sqrt(x*x+y*y) <= pmt_radius-pmt_space/2.0) {
                topbot.push_back(make_pair(x,y));
            }
            double vx = i*veto_space/1.118;
            double vy = (j+(i%2 ? 0.5 : 0.0))*veto_space*1.118;
            if (sqrt(vx*vx+vy*vy) <= pmt_radius-pmt_space/2.0) { // pmt_* is not a mistake
                topbot_veto.push_back(make_pair(vx,vy));
            }
        }
    }
    
    size_t num_pmts = cols*rows + 2*topbot.size();
    size_t num_vetos = veto_cols*veto_rows + 2*topbot_veto.size();
    size_t total_pmts = num_pmts + num_vetos;
    
    info << "Actual calculated values:\n"; 
    info << "\tactual photocathode coverage " << photocathode_area*num_pmts/surface_area << '\n';
    info << "\tgenerated PMTs " << num_pmts << '\n';
    info << "\tcols " << cols << '\n';
    info << "\trows " << rows << '\n';
    info << "\tgenerated Vetos " << num_vetos << '\n';
    info << "\tcols " << veto_cols << '\n';
    info << "\trows " << veto_rows << '\n';
    
    vector<double> x(total_pmts), y(total_pmts), z(total_pmts), dir_x(total_pmts), dir_y(total_pmts), dir_z(total_pmts);
    vector<int> type(total_pmts);
    
    //generate cylinder PMT positions
    for (size_t col = 0; col < cols; col++) {
        for (size_t row = 0; row < rows; row++) {
            const size_t idx = row + col*rows;
            const double phi = 2.0*M_PI*(col+(row%2 ? 0.5 : 0.0))/cols;
            
            x[idx] = pmt_radius*cos(phi);
            y[idx] = pmt_radius*sin(phi);
            z[idx] = row*2.0*topbot_offset/rows + pmt_space/2.0 - topbot_offset;
            
            dir_x[idx] = -cos(phi);
            dir_y[idx] = -sin(phi);
            dir_z[idx] = 0.0;
            
            type[idx] = 1;
        }
    }
    
    //generate topbot PMT positions
    for (size_t i = 0; i < topbot.size(); i++) {
        const size_t idx = rows*cols+i*2;
        
        //top = idx
        x[idx] = topbot[i].first;
        y[idx] = topbot[i].second;
        z[idx] = topbot_offset;
        
        dir_x[idx] = dir_y[idx] = 0.0;
        dir_z[idx] = -1.0;
        
        type[idx] = 1;
        
        //bot = idx+1
        x[idx+1] = topbot[i].first;
        y[idx+1] = topbot[i].second;
        z[idx+1] = -topbot_offset;
        
        dir_x[idx+1] = dir_y[idx] = 0.0;
        dir_z[idx+1] = 1.0;
        
        type[idx+1] = 1;
    }
    
    //generate cylinder Veto positions
    for (size_t col = 0; col < veto_cols; col++) {
        for (size_t row = 0; row < veto_rows; row++) {
            const size_t idx = num_pmts + row + col*veto_rows;
            const double phi = 2.0*M_PI*(col+(row%2 ? 0.0 : 0.5))/veto_cols;
            
            x[idx] = veto_radius*cos(phi);
            y[idx] = veto_radius*sin(phi);
            z[idx] = row*2.0*topbot_offset/veto_rows + veto_space/2 - topbot_offset;
            
            dir_x[idx] = cos(phi);
            dir_y[idx] = sin(phi);
            dir_z[idx] = 0.0;
            
            type[idx] = 2;
        }
    }
    
    //generate topbot Veto positions
    for (size_t i = 0; i < topbot_veto.size(); i++) {
        const size_t idx = num_pmts + veto_rows*veto_cols+i*2;
        
        //top = idx
        x[idx] = topbot_veto[i].first;
        y[idx] = topbot_veto[i].second;
        z[idx] = topbot_veto_pmt_offset;
        
        dir_x[idx] = dir_y[idx] = 0.0;
        dir_z[idx] = 1.0;
        
        type[idx] = 2;
        
        //bot = idx+1
        x[idx+1] = topbot_veto[i].first;
        y[idx+1] = topbot_veto[i].second;
        z[idx+1] = -topbot_veto_pmt_offset;
        
        dir_x[idx+1] = dir_y[idx] = 0.0;
        dir_z[idx+1] = -1.0;
        
        type[idx+1] = 2;
    }
    
    info << "Rescale Theia geometry...\n";
    vector<double> world_size(3);
    world_size[0] = world_size[1] = det_radius+10000.0;
    world_size[2] = det_halfheight+10000.0;
    db->Set("GEO","world","size",world_size);
    db->Set("GEO","tank","r_max",det_radius+tank_thickness);
    db->Set("GEO","tank","size_z",det_halfheight+tank_thickness);
    db->Set("GEO","detector","r_max",det_radius);
    db->Set("GEO","detector","size_z",det_halfheight);
    db->Set("GEO","balloon","r_max",balloon_r_max);
    db->Set("GEO","balloon_nylon_layer1","r_max",nylon1_r_max);
    db->Set("GEO","balloon_nylon_layer2","r_max",nylon2_r_max);
    db->Set("GEO","balloon_nylon_layer3","r_max",nylon3_r_max);
    db->Set("GEO","target","r_max",target_r_max);

    info << "Generating PMTINFO...\n";
    db->Set("PMTINFO","x",x);
    db->Set("PMTINFO","y",y);
    db->Set("PMTINFO","z",z);
    db->Set("PMTINFO","dir_x",dir_x);
    db->Set("PMTINFO","dir_y",dir_y);
    db->Set("PMTINFO","dir_z",dir_z);
    db->Set("PMTINFO","type",type);
    
    info << "Update geometry fields related to veto PMTs...\n";
    db->Set("GEO","veto_pmts","start_idx",num_pmts);
    db->Set("GEO","veto_pmts","end_idx",total_pmts-1);
    
    info << "Update geometry fields related to normal PMTs...\n";
    db->Set("GEO","inner_pmts","start_idx",0);
    db->Set("GEO","inner_pmts","end_idx",num_pmts-1);
    
}

}
