/*
 * HwInterface.cpp
 *
 *  Created on: 12.10.2016
 *      Author: matejko
 */

//class library
#include "mrvk_driver/HwInterface.h"
//#include <exception>



namespace mrvk {

HwInterface::HwInterface() : nh("/"){
	inicialized = false;
}

int HwInterface::init(hardware_interface::RobotHW* hardware,transmission_interface::RobotTransmissions* robot_transmissions,std::string robot_description){

	if (!hardware)            				{throw std::invalid_argument("Invalid robot hardware pointer.");}
	if (!robot_transmissions) 				{throw std::invalid_argument("Invalid robot transmissions pointer.");}
	nh.getParam(robot_description,robot_URDF);
	nh.getParam("transmissions",transmission_names);
	if(robot_URDF.empty()) 	  				{throw std::invalid_argument("Empty Urdf.");}
	if(transmission_names.empty())			{throw std::invalid_argument("Empty transmissions.");}

	// Can throw
	transmission_loader_.reset(new transmission_interface::TransmissionInterfaceLoader(hardware, robot_transmissions));
	// Can throw
	infos = loadTransmissionInfos(robot_URDF,transmission_names);

	if(infos.empty())						{throw std::invalid_argument("No transmission found in Urdf .");}

	createActInterface(hardware);
	if(!transmission_loader_->load(infos))	{throw HwInterface::Exception("Load Failed");}
	inicialized = true;
	return infos.size();
}

std::vector<Limits> HwInterface::getLimits(){
	if(!inicialized) {throw HwInterface::Exception("class not inicialized");}
	std::vector<std::string> joint_names;
	BOOST_FOREACH(transmission_interface::TransmissionInfo info, infos){
		BOOST_FOREACH(transmission_interface::JointInfo joint,info.joints_){
			joint_names.push_back(joint.name_);
		}
	}
	urdf::Model urdf;
	if(!urdf.initString(robot_URDF))		{throw HwInterface::Exception("Failed to init Urdf model");}

	std::vector<Limits> joint_limits(joint_names.size());
	std::vector<std::string> not_found;
	for(int i=0; i<joint_names.size();i++){
		boost::shared_ptr<const urdf::Joint> urdf_wheel_joint = urdf.getJoint(joint_names[i]);
		joint_limits[i].joint = joint_names[i];
		if(!urdf_wheel_joint){
			not_found.push_back(joint_names[i]);
			continue;
		}
		if(!getJointLimits(urdf_wheel_joint, joint_limits[i].joint_info)){
			ROS_WARN("Failed to get '%s' limits from URDF",joint_names[i].c_str());
		}
		//get acc limit from parameter server
		if(!joint_limits_interface::getJointLimits(joint_names[i], nh, joint_limits[i].joint_info))
			ROS_WARN("Failed to get '%s' joint limits from parameter server",joint_names[i].c_str());
	}
	if(!not_found.empty()){					throw HwInterface::NotFound("Joint not found in Urdf",not_found);}
	return joint_limits;

}

/*
 * Create all actuators interfaces for each actuator
 * */
void HwInterface::createActInterface(hardware_interface::RobotHW* hardware){
	std::vector<std::string> act_names;
	BOOST_FOREACH(transmission_interface::TransmissionInfo info, infos){
		BOOST_FOREACH(transmission_interface::ActuatorInfo act,info.actuators_){
			act_names.push_back(act.name_);
		}
	}
	pos.resize(act_names.size(),0);
	vel.resize(act_names.size(),0);
	eff.resize(act_names.size(),0);
	pos_cmd.resize(act_names.size(),0);
	vel_cmd.resize(act_names.size(),0);
	eff_cmd.resize(act_names.size(),0);
	for(int i = 0; i<act_names.size();i++){
		 act_state_interface.registerHandle(hardware_interface::ActuatorStateHandle(act_names[i], &pos[i], &vel[i], &eff[i]));
		 act_pos_interface.registerHandle(hardware_interface::ActuatorHandle(act_state_interface.getHandle(act_names[i]), &pos_cmd[i]));
		 act_vel_interface.registerHandle( hardware_interface::ActuatorHandle(act_state_interface.getHandle(act_names[i]), &vel_cmd[i]));
		 act_eff_interface.registerHandle(hardware_interface::ActuatorHandle(act_state_interface.getHandle(act_names[i]), &eff_cmd[i]));
	}
	hardware->registerInterface(&act_state_interface);
	hardware->registerInterface(&act_pos_interface);
	hardware->registerInterface(&act_vel_interface);
	hardware->registerInterface(&act_eff_interface);
	return;
}

bool HwInterface::initUrfd(urdf::Model *urdf,std::string robot_URDF){
	if(!urdf->initString(robot_URDF)){
		ROS_ERROR("Failed to parse robot description");
		return false;
	}
	return true;
}

/*
 * 	Return info about actuators contained in transmission_names
 *  Parameters:
 *  robot_URDF - string with urdf
 *  transmission_names - vector of string with actuator names
 * * */
std::vector<transmission_interface::TransmissionInfo> HwInterface::loadTransmissionInfos(std::string robot_URDF,std::vector<std::string> transmission_names){

	transmission_interface::TransmissionParser parser;
	std::vector<transmission_interface::TransmissionInfo> all_infos;
	if (!parser.parse(robot_URDF, all_infos)) 	{throw HwInterface::Exception("Faile to parse Urdf");} //natiahne mi prevodovky z urdf
	if (all_infos.empty())						{throw HwInterface::Exception("No transmissions were found in the robot description");}

	std::vector<transmission_interface::TransmissionInfo> my_info;
	std::vector<std::string> not_found;
	for(int i = 0; i<transmission_names.size();i++){
		for(int j = 0 ; j < all_infos.size() ; j++){
			if(transmission_names[i] == all_infos[j].name_){ // iba pre jednen motor
				my_info.push_back(all_infos[j]);
				ROS_ERROR("mam '%s'",all_infos[j].name_.c_str());
				break;
			}
			if(j == all_infos.size()-1){
				ROS_ERROR("Failed to find actuator '%s' in Urdf",transmission_names[i].c_str());
				not_found.push_back(transmission_names[i]);
			}
		}
	}
	if(!not_found.empty()) 					{throw HwInterface::NotFound("Faile to parse Urdf",not_found);}
	return my_info;
}

std::vector<double>* HwInterface::getPosVector(){
	return &pos;
}
std::vector<double>* HwInterface::getVelVector(){
	return &vel;
}
std::vector<double>* HwInterface::getEffVector(){
	return &eff;
}
std::vector<double>* HwInterface::getPosCmdVector(){
	return &pos_cmd;
}
std::vector<double>* HwInterface::getVelCmdVector(){
	return &vel_cmd;
}
std::vector<double>* HwInterface::getEffCmdVector(){
	return &eff_cmd;
}

HwInterface::~HwInterface() {
	// TODO Auto-generated destructor stub
}

} /* namespace Mrvk */