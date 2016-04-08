#include "pitt_msgs/ClustersOutput.h" // for cluster input (msg(
#include "pitt_msgs/InliersCluster.h" // for cluster input (sub msg)
#include "pitt_msgs/PrimitiveSegmentation.h" // for ransac service output
#include "pitt_msgs/TrackedShapes.h" // for out message (an array of TrackedShape)
#include "pitt_msgs/TrackedShape.h" // for out message
#include "point_cloud_library/pc_manager.h" // for my static library
#include "point_cloud_library/srv_manager.h"

using namespace ros;
using namespace pcm;
using namespace srvm;
//using namespace pitt_object_table_segmentation;
//using namespace geometric_traking;
using namespace pitt_msgs;

typedef vector< InliersCluster> InliersClusters;
typedef boost::shared_ptr< InliersClusters> InliersClustersPtr;
typedef boost::shared_ptr< PrimitiveSegmentation> PrimitiveSegmentationPtr;

ros::NodeHandle* nh_ptr = NULL;

boost::shared_ptr< visualization::PCLVisualizer> vis;	// to visualize cloud
Publisher pub; // to publish out results

static const int SPHERE_MIN_INLIERS = 40;
static const int CYLINDER_MIN_INLIERS = 40;
static const int CONE_MIN_INLIERS = 40;
static const int PLANE_MIN_INLIERS = 40;

static const bool SHOW_PRIMITIVE = true;
const static float CONE_TO_CYLINDER_PRIORITY = 0.9f;// 0.90f;
const static int TXT_UNKNOWN_SHAPE_TAG = 0;
const static int TXT_PLANE_SHAPE_TAG = 1;
const static int TXT_SPHERE_SHAPE_TAG = 2;
const static int TXT_CONE_SHAPE_TAG = 3;
const static int TXT_CYLINDER_SHAPE_TAG = 4;

// use ransac to detach sphere
bool callRansacSphereSegmentation( PCLCloudPtr cloud, PCLNormalPtr norm, PrimitiveSegmentationPtr &out){//, bool parameterTest){
    // call sphere service
    NodeHandle n;
    ServiceClient client = n.serviceClient< PrimitiveSegmentation>( srvm::SRV_NAME_RANSAC_SPHERE_FILTER);
    PrimitiveSegmentation srv;

    // set input parameters
    srv.request.cloud = PCManager::cloudToRosMsg( cloud);
    srv.request.normals = PCManager::normToRosMsg( norm);

    // call the service
    if( client.call( srv)){

        int minInliers = 0;
        nh_ptr->param(srvm::PARAM_NAME_SPHERE_MIN_INLIERS, minInliers, SPHERE_MIN_INLIERS);

        if(  srv.response.inliers.size() > minInliers){
            *out = srv;

            //if( parmeterTest)
            //	toWrite << "\t[sphere segmentation] " << usedParameterString( srv.response) << endl;

            return( true);
        }
    } else  ROS_ERROR( " ERROR on service %s", client.getService().c_str());
    return( false);
}
// print sphere details
void printSphereInfo( PrimitiveSegmentationPtr info, int idx){
    if( info->response.inliers.size() > 0 && info->response.coefficients.size() > 0)
        cout << idx << "- sphere found ... inliers:" << info->response.inliers.size() <<
        " Cx:" << info->response.coefficients[ 0] <<
        " Cy:" << info->response.coefficients[ 1] <<
        " Cz:" << info->response.coefficients[ 2] <<
        " radius:" << info->response.coefficients[ 3] << endl;
}

// use ransac to detach cylinder
bool callRansacCylinderSegmentation( PCLCloudPtr cloud, PCLNormalPtr norm, PrimitiveSegmentationPtr &out){//, bool parmeterTest){
    // call sphere service
    NodeHandle n;
    ServiceClient client = n.serviceClient< PrimitiveSegmentation>( srvm::SRV_NAME_RANSAC_CYLINDER_FILTER);
    PrimitiveSegmentation srv;

    // set input parameters
    srv.request.cloud = PCManager::cloudToRosMsg( cloud);
    srv.request.normals = PCManager::normToRosMsg( norm);

    // call the service
    if( client.call( srv)){
        int minInliers = 0;
        nh_ptr->param(srvm::PARAM_NAME_CYLINDER_MIN_INLIERS, minInliers, CYLINDER_MIN_INLIERS);
        if(  srv.response.inliers.size() > minInliers){
            *out = srv; // get respose

            //if( parmeterTest)
            //	toWrite << "\t[cylinder segmentation] " << usedParameterString( srv.response) << endl;

            return( true);
        }
    } else  ROS_ERROR( " ERROR on service %s", client.getService().c_str());
    return( false);
}
// print sphere details
void printCylinderInfo( PrimitiveSegmentationPtr info, int idx){
    if( info->response.inliers.size() > 0 && info->response.coefficients.size() > 0)
        cout << " cylinder found ... inliers:" << info->response.inliers.size() <<
        " pointX:" << info->response.coefficients[ 0] <<
        " pointY:" << info->response.coefficients[ 1] <<
        " pointZ:" << info->response.coefficients[ 2] <<
        "  axisX:" << info->response.coefficients[ 3] <<
        "  axisY:" << info->response.coefficients[ 4] <<
        "  axisZ:" << info->response.coefficients[ 5] <<
        " radious:" << info->response.coefficients[ 6] << endl;
}

// use ransac to detach cone
bool callRansacConeSegmentation( PCLCloudPtr cloud, PCLNormalPtr norm, PrimitiveSegmentationPtr &out){//,  bool parmeterTest){
    // call sphere service
    NodeHandle n;
    ServiceClient client = n.serviceClient< PrimitiveSegmentation>( srvm::SRV_NAME_RANSAC_CONE_FILTER);
    PrimitiveSegmentation srv;

    // set input parameters
    srv.request.cloud = PCManager::cloudToRosMsg( cloud);
    srv.request.normals = PCManager::normToRosMsg( norm);

    // call the service
    if( client.call( srv)){
        int minInliers = 0;
        nh_ptr->param(srvm::PARAM_NAME_CONE_MIN_INLIERS, minInliers, CONE_MIN_INLIERS);
        if(  srv.response.inliers.size() > minInliers){
            *out = srv; // get the respose

            //if( parmeterTest)
            //	toWrite << "\t[cone segmentation] " << usedParameterString( srv.response) << endl;
            return( true);
        }
    } else  ROS_ERROR( " ERROR on service %s", client.getService().c_str());
    return( false);
}

// print sphere details
void printConeInfo( PrimitiveSegmentationPtr info, int idx){
    if( info->response.inliers.size() > 0 && info->response.coefficients.size() > 0)
        cout << " cone  found ... inliers:" <<  info->response.inliers.size() <<
        " apexX:" << info->response.coefficients[ 0] <<
        " apexY:" << info->response.coefficients[ 1] <<
        " apexZ:" << info->response.coefficients[ 2] <<
        " axixX:" << info->response.coefficients[ 3] <<
        " axixY:" << info->response.coefficients[ 4] <<
        " axixZ:" << info->response.coefficients[ 5] <<
        " angle:" << info->response.coefficients[ 6] << endl;
}


// use ransac to detach plane
bool callRansacPlaneSegmentation( PCLCloudPtr cloud, PCLNormalPtr norm, PrimitiveSegmentationPtr &out){//,  bool parmeterTest){
    // call sphere service
    NodeHandle n;
    ServiceClient client = n.serviceClient< PrimitiveSegmentation>( srvm::SRV_NAME_RANSAC_PLANE_FILTER);
    PrimitiveSegmentation srv;

    // set input parameters
    srv.request.cloud = PCManager::cloudToRosMsg( cloud);
    srv.request.normals = PCManager::normToRosMsg( norm);

    // call the service
    if( client.call( srv)){
        int minInliers = 0;
        nh_ptr->param(srvm::PARAM_NAME_PLANE_MIN_INLIERS, minInliers, PLANE_MIN_INLIERS);
        if(  srv.response.inliers.size() > minInliers){
            *out = srv; // get response

            //if( parmeterTest)
            //	toWrite << "\t[plane segmentation] " << usedParameterString( srv.response) << endl;

            return( true);
        }
    } else ROS_ERROR( " ERROR on service %s", client.getService().c_str());
    return( false);
}
// print sphere details
void printPlaneInfo( PrimitiveSegmentationPtr info, int idx){
    if( info->response.inliers.size() > 0 && info->response.coefficients.size() > 0)
        cout << " cone  found ... inliers:" <<  info->response.inliers.size() <<
        " aX:" << info->response.coefficients[ 0] <<
        " bY:" << info->response.coefficients[ 1] <<
        " cZ:" << info->response.coefficients[ 2] <<
        " d :" << info->response.coefficients[ 3] << endl;
}

string returnPrimitiveNameFromTag( int primitiveTag){
    switch( primitiveTag){
        case TXT_UNKNOWN_SHAPE_TAG:  return( "unknown");
        case TXT_PLANE_SHAPE_TAG:	 return( "plane");
        case TXT_SPHERE_SHAPE_TAG:	 return( "sphere");
        case TXT_CONE_SHAPE_TAG:	 return( "cone");
        case TXT_CYLINDER_SHAPE_TAG: return( "cylinder");
    }
}


// node callback
string centroidFileLog;
void clustersAcquisition(const ClustersOutputConstPtr& clusterObj){
    // iterate over all the input clusters
    InliersClusters clusters = clusterObj->clusterObjs;
    // create an output vector to be published
    TrackedShapes::Ptr outShapes ( new TrackedShapes);
    if( clusters.size() > 0){ // at least one cluster
        // add the response to the shape map of the point cloud manager
        for( int j = 0; j < clusters.size(); j++){ // scan all the clusters
            // add primitive
            PCLCloudPtr cluster = PCManager::cloudForRosMsg( clusters[ j].cloud);
            PCLNormalPtr normalCluster = PCManager::estimateNormal( cluster);

            // initialize inlier count
            size_t planeInl = 0, coneInl = 0, sphereInl = 0, cylinderInl = 0;
            size_t clusterSize = cluster->size();

            // call ransac sphere service
            PrimitiveSegmentationPtr outSphere ( new PrimitiveSegmentation);
            if( callRansacSphereSegmentation( cluster, normalCluster, outSphere))
                sphereInl = outSphere->response.inliers.size();

            // call ransac cylinder service
            PrimitiveSegmentationPtr outCylinder ( new PrimitiveSegmentation);
            if( callRansacCylinderSegmentation( cluster, normalCluster, outCylinder))
                cylinderInl = outCylinder->response.inliers.size();


            // call ransac cone service
            PrimitiveSegmentationPtr outCone ( new PrimitiveSegmentation);
            if( callRansacConeSegmentation( cluster, normalCluster, outCone))
                coneInl = outCone->response.inliers.size();

            // call ransac plane service
            PrimitiveSegmentationPtr outPlane ( new PrimitiveSegmentation);
            if( callRansacPlaneSegmentation( cluster, normalCluster, outPlane))
                planeInl = outPlane->response.inliers.size();

            // show cluster with color w.r.t. primitive
            int R, G, B;
            int detechedPrimitiveTag;
            float xC, yC, zC;
            PrimitiveSegmentationPtr primitiveInfo;
            if( ( ! planeInl) && ( ! sphereInl) && ( ! cylinderInl) && ( ! coneInl)){
                R = 100, G = 100, B = 100;	// nothing : GRAY
                detechedPrimitiveTag = TXT_UNKNOWN_SHAPE_TAG;
            } else if ( ( coneInl >= planeInl) && ( coneInl >= sphereInl) && ( coneInl >= cylinderInl * CONE_TO_CYLINDER_PRIORITY)) {
                R = 0; G = 255; B = 0;		// cone : GREEN	[priority w.r.t. cylinder]
                detechedPrimitiveTag = TXT_CONE_SHAPE_TAG;
                primitiveInfo = outCone;
                xC = outCone->response.x_centroid;
                yC = outCone->response.y_centroid;
                zC = outCone->response.z_centroid;
            } else if ( ( cylinderInl >= planeInl) && ( cylinderInl >= coneInl) && ( cylinderInl >= sphereInl)) {
                R = 255; G = 255; B = 255;  // cylinder : WHITE
                detechedPrimitiveTag = TXT_CYLINDER_SHAPE_TAG;
                primitiveInfo = outCylinder;
                xC = outCylinder->response.x_centroid;
                yC = outCylinder->response.y_centroid;
                zC = outCylinder->response.z_centroid;
                //centroidFileLog += boost::to_string( j) + "," + boost::to_string( outCylinder->response.xCentroid) + "," + boost::to_string( outCylinder->response.yCentroid) + "," + boost::to_string( outCylinder->response.zCentroid) + "\n";
            }else if( ( planeInl >= coneInl) && ( planeInl >= sphereInl) && ( planeInl >= cylinderInl)){
                R = 255; G = 0; B = 0;		// plane : RED
                detechedPrimitiveTag = TXT_PLANE_SHAPE_TAG;
                primitiveInfo = outPlane;
                xC = outPlane->response.x_centroid;
                yC = outPlane->response.y_centroid;
                zC = outPlane->response.z_centroid;
            } else if ( ( sphereInl >= planeInl) && ( sphereInl >= coneInl) && ( sphereInl >= cylinderInl)) {
                R = 0; G = 0; B = 255;		// sphere : BLUE [priority w.r.t. cone]
                detechedPrimitiveTag = TXT_SPHERE_SHAPE_TAG;
                primitiveInfo = outSphere;
                xC = outSphere->response.x_centroid;
                yC = outSphere->response.y_centroid;
                zC = outSphere->response.z_centroid;
                //centroidFileLog += boost::to_string( j) + "," + boost::to_string( outSphere->response.xCentroid) + "," + boost::to_string( outSphere->response.yCentroid) + "," + boost::to_string( outSphere->response.zCentroid) + "\n";
            } else {
                R = 100, G = 100, B = 100;	// nothing : GRAY
                detechedPrimitiveTag = TXT_UNKNOWN_SHAPE_TAG;
            }
            if( SHOW_PRIMITIVE)
                PCManager::updateVisor( vis, cluster, R, G, B, "clusterShape" + boost::to_string( j));

            ROS_INFO( "cluster_%d: %d #INLIER plane: %d sphere: %d cylinder: %d cone: %d selected: %s",
                      clusters[ j].shape_id, (int) cluster->size(), planeInl, sphereInl, cylinderInl, coneInl, returnPrimitiveNameFromTag( detechedPrimitiveTag).c_str());

            // prepared output to be published
            TrackedShape::Ptr outShape ( new TrackedShape);
            outShape->object_id = clusters[ j].shape_id;
            outShape->x_pc_centroid = clusters[ j].x_centroid;
            outShape->y_pc_centroid = clusters[ j].y_centroid;
            outShape->z_pc_centroid = clusters[ j].z_centroid;
            outShape->shape_tag = returnPrimitiveNameFromTag( detechedPrimitiveTag);
            if( detechedPrimitiveTag != TXT_UNKNOWN_SHAPE_TAG){
                outShape->x_est_centroid = xC;
                outShape->y_est_centroid = yC;
                outShape->z_est_centroid = zC;
                outShape->coefficients = primitiveInfo->response.coefficients;
            }
            outShapes->tracked_shapes.push_back( *outShape);
            // propagate usefull data from geometric_tracker (no computation on those vectors)
            // not working !!!!!
            //outShapes->clusterAdded = clusterObj->clusterAdded;
            //outShapes->clusterAdded = clusterObj->clusterRemove;
        }
    } // error in cluster service
    ROS_INFO( " ------------------------------------ " );
    //PCManager::writeToFile( centroidFileLog, "/home/luca-phd/ransac_centroid");

    // publish out results
    pub.publish( outShapes);

}


// main method of the node
int main(int argc, char **argv){

    init(argc, argv, "ransac_segmentation");
    ros::NodeHandle nh;
    nh_ptr = &nh;

    centroidFileLog = "";

    // subscribe to cluster published by obj_segmentation.cpp
    //Subscriber sub = n.subscribe ("obj_segmentation/cluster", 10, clustersAcquisition); // get data from cluster node
    Subscriber sub = nh.subscribe( "geometric_tracker/trackedCluster", 10, clustersAcquisition); // get data from tracker node
    // to publish the data at the end of the process
    pub = nh.advertise< TrackedShapes>( "ransac_segmentation/trackedShape", 10);

    // create window to visualize cloud
    if( SHOW_PRIMITIVE)
        vis = PCManager::createVisor( "Ransac shape segmentation");


    //Rate rate(20); // hz
    while ( nh.ok()){
        spinOnce();
    }

    return 0;
}