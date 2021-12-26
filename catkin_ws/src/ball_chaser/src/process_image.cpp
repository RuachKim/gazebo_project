#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>



// Define a global client that can request services
ros::ServiceClient client; 

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ROS_INFO_STREAM("Try to call service command_robot");

    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the safe_move service and pass the requested joint angles
    if (!client.call(srv))
        ROS_ERROR("Failed to call service command_robot");

}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    bool moving_state = false;
    // int moving_cmd = 0;
    int obj_pos = 0;
    int avg_rgb = 0;
    // Loop through each pixel in the image and check if its equal to the first one
    // left = [0, img.step/3];  
    // forward = [img.step/3, img.step/3*2]
    // right = [img.step/3*2, img.step] 
    for (int i = 0; i < img.height * img.step; i++) {
        avg_rgb = (img.data[i] + img.data[i+1] + img.data[i+2])/3;
        ROS_INFO("RGB = %d", avg_rgb);
        if(img.data[i] == white_pixel && img.data[i+1] == white_pixel &&  img.data[i+2] == white_pixel){
            ROS_INFO("Detected!");
            moving_state = true;    
            obj_pos = i%img.step;
            break;
        }

    }

    if(!moving_state){
        ROS_INFO("Stop - No Object Found");
        drive_robot(0.0,0.0);
    } 
    else{
        moving_state = false;
        if(obj_pos < img.step/3){
            ROS_INFO("Drive Left -- (lin_x: 0.0 , ang_z: 0.1)");
            drive_robot(0,0.1);
        }
        else if((obj_pos >= img.step/3) && (obj_pos < img.step*2/3)){
            ROS_INFO("Drive Forward-- (lin_x: 0.1 , ang_z: 0.0)");
            drive_robot(0.1,0);
        }
        else if(obj_pos >= img.step*2/3){
            ROS_INFO("Drive Right -- (lin_x: 0.0 , ang_z: -0.1)");
            drive_robot(0,-0.1);
        }

    }

}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}