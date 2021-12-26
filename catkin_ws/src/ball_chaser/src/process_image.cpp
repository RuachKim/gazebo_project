#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include <map>
#include <cstring>
#include <iterator>
#include <iostream>
#include <algorithm>

using namespace std;

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
    // map
    map<string, int> posMap;

    int white_pixel = 255;

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    bool moving_state = false;
    // int moving_cmd = 0;
    int obj_pos = 0;
    int max_val = 0;
    // Loop through each pixel in the image and check if its equal to the first one
    // left = [0, img.step/3];  
    // forward = [img.step/3, img.step/3*2]
    // right = [img.step/3*2, img.step] 
    posMap.insert(pair<string, int>{"left",0});
    posMap.insert(pair<string, int>{"forward",0});
    posMap.insert(pair<string, int>{"right",0});
    
    int idx_l = 0;
    int idx_f = 0;
    int idx_r = 0;

    for (int i = 0; i < img.height * img.step; i+=3) {
        if(img.data[i] == white_pixel && img.data[i+1] == white_pixel &&  img.data[i+2] == white_pixel){
            moving_state = true;    
            obj_pos = i%img.step;
            if(obj_pos < img.step/3){
                idx_l += 1;
                // posMap['left'] = idx_l;
                posMap.find("left")->second = idx_l;
            }
            else if((obj_pos >= img.step/3) && (obj_pos < img.step*2/3)){
                idx_f += 1;
                posMap.find("forward")->second = idx_f;
            }
            else if(obj_pos >= img.step*2/3){
                idx_r += 1;
                posMap.find("right")->second = idx_r;
            }
           
        }
 
    }
    
    if(!moving_state){
        ROS_INFO("Stop - No Object Found");
        drive_robot(0.0,0.0);
    } 
    else{
        
        ROS_INFO("Detected!");
        moving_state = false;

        max_val = max({idx_l, idx_f, idx_r});

        for(auto& it : posMap){
            if(it.second == max_val){
                if(it.first == "left"){
                    ROS_INFO("Drive Left");
                    drive_robot(0,0.5);
                }
                else if(it.first == "forward"){
                    ROS_INFO("Drive Forward");
                    drive_robot(0.5,0);
                }
                else if(it.first == "right"){
                    ROS_INFO("Drive Right");
                    drive_robot(0,-0.5);
                }
            }
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
