/*
 * callbacks.cpp
 *
 *  Created on: 26.10.2016
 *      Author: michal
 */

#ifndef CALLBACKS_H_
#define CALLBACKS_H_

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ros/ros.h>
#include <std_srvs/Trigger.h>

#include <mrvk_driver/communication_interface.h>


class MrvkCallbacks{
	public:
		MrvkCallbacks(CommunicationInterface *interface);

	void getMbFromParam(SET_MAIN_BOARD *config);
	void getMotorParametersFromParam(REGULATOR_MOTOR *reg, bool *regulation_type);

	private:

	CommunicationInterface *interface;

		//ros::ServiceServer initSS;
		ros::ServiceServer shutdownSS;
		//ros::ServiceServer resetFlagsSS;
		ros::ServiceServer resetBatterySS;
		ros::ServiceServer resetCentralStopSS;
		//ros::ServiceServer stopSS;
		ros::ServiceServer setArmVoltageSS;
		ros::ServiceServer setCameraSourceSS;
		ros::ServiceServer setPowerManagmentSS;
		ros::ServiceServer setMotorParametersSS;

		//	bool initCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res);
		bool shutdownCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res);
		//bool resetFlagsCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res);
		bool resetBatteryCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res);
		bool resetCentralStopCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res);
		//bool stopCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res);
		bool setArmVoltageCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res);
		bool setCameraSourceCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res);
		bool setPowerManagmentCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res);
		//bool setMotorParametersCallback(std_srvs::Trigger::Request  &req, std_srvs::Trigger::Response &res);


};

#endif /* CALLBACKS_CPP_ */