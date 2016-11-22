/*
 * HwInterface.h
 *
 *  Created on: 12.10.2016
 *      Author: matejko
 */

#ifndef HWINTERFACE_H_
#define HWINTERFACE_H_
//ros libraries
#include <ros/ros.h>

//ros_controll libraries
#include <hardware_interface/actuator_state_interface.h>
#include <hardware_interface/actuator_command_interface.h>
#include <hardware_interface/robot_hw.h>
#include <transmission_interface/transmission_interface_loader.h>

//joint limit interface libraries
#include <joint_limits_interface/joint_limits.h>
#include <joint_limits_interface/joint_limits_urdf.h>
#include <joint_limits_interface/joint_limits_rosparam.h>
#include <joint_limits_interface/joint_limits_interface.h>

//URDF library
#include <urdf/model.h>

//boost libraries
#include <boost/scoped_ptr.hpp>
#include <boost/foreach.hpp>

namespace mrvk {

typedef struct
{
  std::string joint;
  joint_limits_interface::JointLimits joint_info;
}Limits;

class HwInterface {
public:
	HwInterface();
	int init(hardware_interface::RobotHW* hardware,transmission_interface::RobotTransmissions* robot_transmissions,std::string robot_description="/robot_description");
	std::vector<Limits> getLimits();
	std::vector<double>* getPosVector();
	std::vector<double>* getVelVector();
	std::vector<double>* getEffVector();
	std::vector<double>* getPosCmdVector();
	std::vector<double>* getVelCmdVector();
	std::vector<double>* getEffCmdVector();
	virtual ~HwInterface();
private:
	void createActInterface(hardware_interface::RobotHW* hardware);
	bool initUrfd(urdf::Model *urdf,std::string robot_URDF);
	std::vector<transmission_interface::TransmissionInfo> loadTransmissionInfos(std::string robot_URDF,std::vector<std::string> act_names);
	ros::NodeHandle nh;
	hardware_interface::ActuatorStateInterface act_state_interface;
	hardware_interface::PositionActuatorInterface act_pos_interface;
	hardware_interface::VelocityActuatorInterface act_vel_interface;
	hardware_interface::EffortActuatorInterface act_eff_interface;
	boost::scoped_ptr<transmission_interface::TransmissionInterfaceLoader> transmission_loader_;
	std::vector<transmission_interface::TransmissionInfo> infos;
	std::vector<double> pos;
	std::vector<double> vel;
	std::vector<double> eff;
	std::vector<double> pos_cmd;
	std::vector<double> vel_cmd;
	std::vector<double> eff_cmd;
	std::vector<std::string> transmission_names;
	std::string robot_URDF;
	bool inicialized;
	class Exception;
	class NotFound;
};

class HwInterface::Exception : public std::exception{
public:
	Exception(std::string reazon_) : reazon(reazon_){}
	virtual ~Exception() throw() {}
	std::string what(){return reazon;}
private:
	std::string reazon;
};

class HwInterface::NotFound : public std::exception{
public:
	NotFound(std::string reazon_,std::vector<std::string> transmissions_) : reazon(reazon_),transmissions(transmissions_){}
	virtual ~NotFound() throw() {}
	std::string what(){return reazon;}
	std::vector<std::string> transmission(){return transmissions;}
private:
	std::string reazon;
	std::vector<std::string> transmissions;
};



} /* namespace Mrvk */

#endif /* HWINTERFACE_H_ */