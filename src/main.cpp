#include <iostream>
#include <pcl/console/parse.h>
#include <pcl/io/vtk_lib_io.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/surface/poisson.h>
#include <pcl/io/pcd_io.h>
#include <pcl/io/ply_io.h>
#include <pcl/point_types.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/features/normal_3d.h>
#include <thread>
#include <pcl/io/vlp_grabber.h>

using namespace std;
using namespace pcl;
using namespace pcl::console;
using namespace pcl::visualization;

PCLVisualizer::Ptr viewer;
std::string filename;
typedef pcl::PointXYZI PointType;

void pcl_viewer()
{
    // Point Cloud
    pcl::PointCloud<PointType>::ConstPtr cloud;
    // PCL Visualizer
    boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer( new pcl::visualization::PCLVisualizer( "Velodyne Viewer" ) );
    viewer->addCoordinateSystem( 3.0, "coordinate" );
    viewer->setBackgroundColor( 0.0, 0.0, 0.0, 0 );
    viewer->initCameraParameters();
    viewer->setCameraPosition( 0.0, 0.0, 30.0, 0.0, 1.0, 0.0, 0 );

    pcl::visualization::PointCloudColorHandler<PointType>::Ptr handler;

        boost::shared_ptr<pcl::visualization::PointCloudColorHandlerGenericField<PointType>> color_handler( new pcl::visualization::PointCloudColorHandlerGenericField<PointType>( "intensity" ) );
        handler = color_handler;

    // Retrieved Point Cloud Callback Function
    boost::mutex mutex;
    boost::function<void( const pcl::PointCloud<PointType>::ConstPtr& )> function =
        [ &cloud, &mutex ]( const pcl::PointCloud<PointType>::ConstPtr& ptr ){
            boost::mutex::scoped_lock lock( mutex );

            cloud = ptr;
        };// VLP Grabber
    boost::shared_ptr<pcl::VLPGrabber> grabber;
        std::cout << "Capture from PCAP..." << std::endl;
        grabber = boost::shared_ptr<pcl::VLPGrabber>( new pcl::VLPGrabber( filename ) );

    // Register Callback Function
    boost::signals2::connection connection = grabber->registerCallback( function );

    // Start Grabber
    grabber->start();

    while( !viewer->wasStopped() ){
        // Update Viewer
        viewer->spinOnce();

        boost::mutex::scoped_try_lock lock( mutex );
        if( lock.owns_lock() && cloud ){
            // Update Point Cloud
            handler->setInputCloud( cloud );
            if( !viewer->updatePointCloud( cloud, *handler, "cloud" ) ){
                viewer->addPointCloud( cloud, *handler, "cloud" );
            }
        }
    }

    // Stop Grabber
    grabber->stop();

    // Disconnect Callback Function
    if( connection.connected() ){
        connection.disconnect();
    }
}

int main(int argc, char * argv[])
{
  std::vector<int> file_indices = pcl::console::parse_file_extension_argument (argc, argv, ".pcap");
  if (file_indices.empty())
  {
    PCL_ERROR ("Please provide file as argument\n");
    return 1;
  }
  filename = argv[file_indices[0]];

  pcl_viewer();

  return 0;
}
